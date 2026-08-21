/* toralizer.c */

#include "toralizer.h"
#include <stdio.h>
#include <string.h>
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

    char greet[] = {0x05,0x01,0x00}; // 3bytes greeting
    char hex[strlen(argv[2])];

    // for (int i = 0; i<strlen(argv[2]);i++) {
    //     int num = argv[2][i] - '0';
    //     unsigned char hex_decimal[3];
    //     snprintf((char *)hex_decimal, sizeof(hex_decimal), "%02X", num);
    //     hex[i] = hex_decimal[0];
    // }

    ssize_t sent = send(sockfd, greet,sizeof(greet), 0);
    if (sent < (ssize_t)sizeof(greet)) {
        perror("SOCKS5 connect request failed");
        close(sockfd);
        return 1;
    }

    size_t hostname_len = strlen(hostname);

    unsigned char request[4 + 1 + hostname_len + 2];

    request[0] = 0x05;
    request[1] = 0x01;
    request[2] = 0x00;
    request[3] = 0x03;                  // ATYP = DOMAIN
    request[4] = hostname_len;          // hostname length

    memcpy(&request[5], hostname, hostname_len);

    request[5 + hostname_len] = (port >> 8) & 0xFF;
    request[6 + hostname_len] = port & 0xFF;

    char buffer[4096];
    int bytes_received = 0;

    ssize_t request_send = send(sockfd,request,sizeof(request),0);
    if (request_send < (ssize_t)sizeof(greet)) {
        perror("Sending request failed\n");
        close(sockfd);
        return 1;
    }

    char http_request[4096];

    int http_len = snprintf(
            http_request,
            sizeof(http_request),
            "GET / HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            hostname
            );

    ssize_t sent1 = send(sockfd, http_request, http_len, 0);

    if (sent1 < 0) {
        perror("send HTTP request");
        close(sockfd);
        return 1;
    }

    /* Receiving the response from the server */
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    close(sockfd);
    return 0;
}
