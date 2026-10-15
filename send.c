#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <arpa/inet.h>
//client - sender 
#define PORT 8080

int send_file(int sock, const char *path, const char *name)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); return -1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return -1; }

    /* header: [8 bytes size][2 bytes name length][name] */
    uint64_t size = htobe64(st.st_size);
    uint16_t nlen = htons(strlen(name));

    if (send_all(sock, &size, sizeof size, MSG_MORE) < 0 ||
        send_all(sock, &nlen, sizeof nlen, MSG_MORE) < 0 ||
        send_all(sock, name, strlen(name), MSG_MORE) < 0) {
        perror("send header");
        close(fd);
        return -1;
    }

    /* body */
    off_t offset = 0;
    while (offset < st.st_size) {
        ssize_t n = sendfile(sock, fd, &offset, st.st_size - offset);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("sendfile");
            close(fd);
            return -1;
        }
        if (n == 0) {
            fprintf(stderr, "sendfile: unexpected EOF\n");
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in server = {
		.sin_family = AF_INET,
        .sin_port = htons(8080),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
	}
	
	
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Couldn't connect");
        return 1;
    }
    
    printf("Connected to server!\n\n");
    
	if (argc < 3 || argv[1] == NULL) { 
    	printf("main argument empty\n"); 
    	return -1; 
	}
	
	if (argv[2] == NULL) { 
    	printf("main argument empty\n"); 
    	return -1; 
	}
	 
	int result = send_file(sock, argv[1], argv[2]);
	     
    if (result < 0) {
        printf("Error sending file.\n");
        return -1;
    }
	 
	close(sock);
	printf("File Sent!\n");
	return 0;
}