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
#define SIZE 500


int main(int argc, char* argv[])
{     
    if (argc != 3) {
    	printf("Usage: ./client <ipv6-adress-node> <interface-name> <abs-path-to-update>");
        return -1;
    }
     //open file
     FILE * fptr = fopen(argv[3], "rb");
     
     if(fptr == NULL) {
     	printf("Error opening file");
	return -1;
     }
     //calculate how many packets to be send
     fseek(fptr,0, SEEK_END);
     size_t file_size = ftell(fptr);
     fseek(fptr,0, SEEK_SET);
     int packets = (file_size/500) +1;

    
    int sockfd;
    char buffer[SIZE];
    struct sockaddr_in6 servaddr;

    // socket creation
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }
    //bind socket to interface
    const char* interface_name = argv[3];
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, interface_name, strlen(interface_name)) < 0) {
        perror("Binding to interface failed,try to run as root");
    }
    //errase memory
    memset(&servaddr, 0, sizeof(servaddr));

    char* ip = argv[1];
    if (inet_pton(AF_INET6, ip, &(servaddr.sin6_addr)) != 1) {
        printf("adress converting failed\n, Please enter a valid ipv6 adress");
        return -1;
    }

    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PORT);
    int n, len;
    
    //send amount of packets to client
    sprintf(buffer, "%d",packets);
    sendto(sockfd,buffer, SIZE,
        0, (const struct sockaddr*)&servaddr,
        sizeof(servaddr));
    //clear buffer
    memset(buffer, 0, SIZE);
    
   //read and send file 
   while(!feof(fptr)) {
    	int byte_read =	fread(buffer,1,SIZE,fptr);
    	if(byte_read != SIZE) {
   		if(!feof(fptr)) {
		printf("Reading failed!!");
		return -1;
		}	 
    	}
	//send chuncked packets
	sendto(sockfd,buffer, byte_read,
        0, (const struct sockaddr*)&servaddr,
        sizeof(servaddr));
    	
    }

    close(sockfd);
    fclose(fptr);
    return 0;
}
