/* toralizer.c */

#include "toralizer.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
    char *hostname;
    int port;

    if (argc < 3) {
        fprintf(stderr,"Usage: %s <host> <port>\n",argv[0]);
    }

    hostname = argv[1];
    port = atoi(argv[2]);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0) {
        fprintf(stderr,"socket() failed\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    struct hostent *server;

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"Invalid hostname\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr,
            server->h_addr_list[0],
            server->h_length);
    addr.sin_port = htons(80);

    if (connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)) == -1) {
        fprintf(stderr,"Failed to connect to server\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to-> %s:%d\n",hostname,port);

    char request[1024];

    snprintf(request, sizeof(request),
            "GET / HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            hostname);

    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        return 1;
    }

    char buffer[4096];

    int bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0) perror("recv");

    close(sockfd);

    return 0;
}
