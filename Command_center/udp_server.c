// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT	 6666
#define SIZE 1024 

// Driver code 
int main() { 
	int socketfd; 
	char buffer[SIZE]; 
	struct sockaddr_in6 local_addr;
	struct sockaddr_in6 remote_addr; 
	 
	//socket creation
	if ( (socketfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) { 
		perror("erstellen des sockets fehlgeschlagen"); 
		return -1; 
	} 

	memset(&local_addr, 0, sizeof(local_addr));	
	//set up local address	
	local_addr.sin6_family = AF_INET6;  
	local_addr.sin6_port = htons(PORT); 
	local_addr.sin6_addr = in6addr_any;
 
	//bind socket to adress
	if ( bind(socketfd, (struct sockaddr *)&local_addr, 
			sizeof(local_addr)) < 0 ) 
	{ 
		perror("bind fehlgeschlagen"); 
		return -1; 
	} 
	

	int len = sizeof(remote_addr); 

	while(1) {
		//wait for message
		int size  = recvfrom(socketfd, (char *)buffer, SIZE, 
				MSG_WAITALL, ( struct sockaddr *) &remote_addr, 
				&len);
		if (size < 0 ) {
			perror("packet war zu klein");
			return -1;		
		} 
		buffer[size] = '\0'; 
		//print data
		printf("Client : %s\n", buffer);
		//clear buffer		
		memset(buffer,0, SIZE);
	} 
	
	
	return 0; 
} 




