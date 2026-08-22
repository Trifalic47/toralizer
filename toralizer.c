#include "toralizer.h"
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <hostname> <port>",argv[0]);
        return EXIT_FAILURE;
    }
    const char *hostname  = argv[1];
    const int port = atoi(argv[2]);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0) {
        printf("sockt() failed\n");
        return EXIT_FAILURE;
    }

    /* Connecting to the server */

    // Resolving hostname
    struct hostent *server = gethostbyname(PROXY);
    if (server == NULL) {
        perror("Invalid hostname, failed to resolve host\n");
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr,server->h_addr_list[0],(ssize_t)sizeof(server->h_addr_list[0]));
    // memcpy(&addr.sin_addr.s_addr,PROXY,sizeof(PROXY));
    addr.sin_port = htons(PROXY_PORT);

    /* Connecting to the sever */
    if (connect(sockfd,(struct sockaddr*)&addr,(ssize_t)sizeof(addr)) == -1) {
        perror("connect() failed\n");
    }

    /* Sending 3bytes socks5 greeting */
    char greet[] = {0x05,0x01,0x00};
    ssize_t greet_sent = send(sockfd,greet,sizeof(greet),0);
    if (greet_sent < sizeof(greet)) {
        perror("greeting sending failed\n");
    }

    char RFC[10] = {
        0x05,0x01,0x00,
        0x03,
    };
    int len = strlen(hostname);
    RFC[4] = len;
    memcpy(&RFC[5],hostname,len);
    RFC[6+len] = (port >> 8) & 0xFF;
    RFC[7+len] = port & 0xFF;

    ssize_t RFC_Sent = send(sockfd,RFC,sizeof(RFC),0);
    if (RFC_Sent < sizeof(RFC)) {
        perror("Failed sending RFC\n");
    }

    return EXIT_SUCCESS;
}
