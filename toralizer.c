/* toralizer.c */

#include "toralizer.h"

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

    // server = gethostbyname(hostname);
    server = gethostbyname(PROXY);
    if (server == NULL) {
        fprintf(stderr,"Invalid hostname\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr,server->h_addr_list[0],server->h_length);
    addr.sin_port = htons(PROXYPORT);

    if (connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)) == -1) {
        fprintf(stderr,"Failed to connect to server\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to-> %s:%d\n",hostname,port);

    char request[1024];

    char greet[] = {0x05,0x01,0x00};

    char RFC[10];
    RFC[0] = 0x05;
    RFC[1] = 0x01;
    RFC[2] = 0x00;
    RFC[3] = 0x01;
    memcpy(&RFC[4], &addr.sin_addr.s_addr, sizeof(addr.sin_addr.s_addr));

    // Copy 2 bytes of network byte order port
    uint16_t net_port = htons(PROXYPORT);
    memcpy(&RFC[8], &net_port, sizeof(net_port));

    ssize_t sent = send(sockfd, RFC, sizeof(RFC), 0);
    if (sent < (ssize_t)sizeof(RFC)) {
        perror("SOCKS5 connect request failed");
        close(sockfd);
        return 1;
    }
    snprintf(request,sizeof(request),"GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",PROXY);

    if (send(sockfd,request,sizeof(request),0) < 0) {
        perror("send failed\n");
        close(sockfd);
        return 1;

    }

    char buffer[4096];

    int bytes_received = 0;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0) perror("recv");

    close(sockfd);

    return 0;
}
