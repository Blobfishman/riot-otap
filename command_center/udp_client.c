// Client side implementation of UDP client-server model
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 6666
#define SIZE 1024
int main(int argc, char* argv[])
{

    int sockfd;
    char buffer[SIZE];
    struct sockaddr_in6 servaddr;
    if (argc != 3) {
        printf("please enter with ipv6 adress of the remote");
        return -1;
    }

    // socket creation
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }
    //bind socket to interface
    const char* opt = "tapbr0";
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, opt, strlen(opt)) < 0) {
        perror("Binding to interface failed,try to run as root");
    }
    //allocate space for addr
    memset(&servaddr, 0, sizeof(servaddr));

    char* ip = argv[1];
    if (inet_pton(AF_INET6, ip, &(servaddr.sin6_addr)) != 1) {
        printf("adress converting failed\n, Please enter a valid ipv6 adress");
        return -1;
    }

    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PORT);
    int n, len;

    sendto(sockfd, argv[2], strlen(argv[2]),
        MSG_DONTWAIT, (const struct sockaddr*)&servaddr,
        sizeof(servaddr));

    close(sockfd);
    return 0;
}
