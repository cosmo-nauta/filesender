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
//server - reciever
#define PORT 8080

int receive_file(int sock)
{
    uint64_t size; uint16_t nlen;
    if (recv_all(sock, &size, sizeof size) < 0) return -1;
    if (recv_all(sock, &nlen, sizeof nlen) < 0) return -1;
    size = be64toh(size);
    nlen = ntohs(nlen);

    char name[256];
    if (nlen == 0 || nlen >= sizeof name) return -1;
    if (recv_all(sock, name, nlen) < 0) return -1;
    name[nlen] = '\0';

    /* IMPORTANT: don't trust the name (see below) */
    if (strchr(name, '/') || strcmp(name, "..") == 0) return -1;

    int out = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) return -1;

    char buf[64 * 1024];
    uint64_t remaining = size;
    while (remaining > 0) {
        size_t want = remaining < sizeof buf ? remaining : sizeof buf;
        ssize_t n = recv(sock, buf, want, 0);
        if (n < 0) { if (errno == EINTR) continue; close(out); return -1; }
        if (n == 0) { close(out); return -1; }   /* connection dropped */
        if (write(out, buf, n) != n) { close(out); return -1; }
        remaining -= n;
    }
    close(out);
    return 0;
}

int main(void)
{
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    listen(server_sock, 1);
    printf("Waiting for connection...\n");

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int client_sock = accept(server_sock, (struct sockaddr *)&client, &len);
    printf("Client connected!\n\n");
    
    while(1){
    	
    	int result =  receive_file(server_sock);
    	
    	 if (result < 0) {
        	printf("Error receiving file.\n");
        	return -1;
   	 	}
    
	}
	
	close(sock);
	return 0;
}