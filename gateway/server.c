#include <stdio.h>
#include <string.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "xtimer.h"

extern int app1(void);
extern int app2(void);

int (*app)(void);

//buffer for the messages
uint8_t buf[512];
char pc_buf[512];

int packets = 0;
int received = 0;

FILE *newfp;

//ports for the nodes and the pc
static uint16_t server_port = 8888;
static uint16_t client_multicast_port = 8888;

static uint16_t pc_server_port = 5555;
static uint16_t pc_client_port = 6666;

bool connection_to_pc = false;

sock_udp_ep_t server;



//Method to send Data to the pc
int send_data_to_pc(int res)
{
    
    server.port = pc_server_port;


    sock_udp_t sock;
    if (sock_udp_create(&sock, &server, NULL, 0) < 0) {
        puts("Error creating UDP sock to send data to pc");
        return 1;
    }

    if (sock_udp_send(&sock, buf, res, &server) < 0) {
        puts("Error sending message to pc");
        //sock_udp_close(&sock);
        return 1;
    }

    sock_udp_close(&sock);
    puts("Send message\n");
    return 0;
}



//send update via Multicast to all nodes
int send_update(void* arg)
{
    (void) arg;

    sock_udp_ep_t remote = { .family = AF_INET6 };
    sock_udp_t sock;
    remote.port = client_multicast_port;

    if (sock_udp_create(&sock, &remote, NULL, 0) < 0) {
        puts("Error creating UDP sock");
        return 1;
    }

    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t*)&remote.addr.ipv6, IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    char buffer[512];

    FILE * fptr = fopen("Update.elf", "rb");
     
     if(fptr == NULL) {
     	printf("Error opening file");
	return -1;
    }

    fseek(fptr,0, SEEK_END);
    size_t file_size = ftell(fptr);
    fseek(fptr,0, SEEK_SET);

    int packets = 0;

    if ((file_size/512) % 2 == 0) 
    {
        packets = (file_size/512);
    }
    else {
        packets = (file_size/512) +1;
    }

    if (sock_udp_send(&sock, &packets, sizeof(&packets), &remote) < 0) 
    {
        puts("Error sending message");
        sock_udp_close(&sock);
        return 1;
    }

    memset(buffer, 0, 512);

     //read and send file 
   while(!feof(fptr)) {
    	int byte_read =	fread(buffer,1,512,fptr);
    	if(byte_read != 512) {
   		if(!feof(fptr)) {
		printf("Reading failed!!");
		return -1;
		}	 
    	}
        
       
        usleep(4200);

    if(byte_read > 0)
    {

	//send chuncked packets

     if (sock_udp_send(&sock, buffer, sizeof(buffer) -1, &remote) < 0) 
    {
        puts("Error sending chunked packages");
        sock_udp_close(&sock);
        return 1;
    }


    	
    }
   }

    fclose(fptr);
    
    sock_udp_close(&sock);
    return 0;
}




//Method to receive Data from Nodes
void* receive_data(void* arg)
{
    (void) arg;
    puts("Udp_Server start for Nodes");

    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = server_port;
    sock_udp_t sock;

    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP sock for nodes");
        return NULL;
    }

    sock_udp_ep_t nodes = { .family = AF_INET6 };
    nodes.port = client_multicast_port;
    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t*)&nodes.addr.ipv6, IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

        if (sock_udp_send(&sock, "10", sizeof("10"), &nodes) < 0) {
            puts("Error sending message");
            sock_udp_close(&sock);
            return 0;
        }


    while (1) {
        sock_udp_ep_t remote;
        ssize_t res;
        if ((res = sock_udp_recv(&sock, buf, sizeof(buf) -1, SOCK_NO_TIMEOUT, &remote)) > 0) {
            puts("Received a message from a node");

            if (connection_to_pc)
            {
                send_data_to_pc(res);
            }
        }
    }
}




//Method to receive Data from the PC
int* receive_update(void* arg)
{

    (void)arg;

    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = pc_client_port;
    sock_udp_t sock;

    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP sock");
        return NULL;
    }

    while (1) {
        sock_udp_ep_t remote;
        size_t res;

        res = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf) -1, SOCK_NO_TIMEOUT, &remote);

        if (res > 0) 
        {
            puts("Udp_Server start for PC");

            server = remote;

            connection_to_pc = true;
    

        while (1)
        {   
            if ((res = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf), SOCK_NO_TIMEOUT, &remote)) > 0) 
            {
                
                newfp = fopen("update.elf","wb");

                if(newfp==NULL)
                {
                    printf("error opening the file\n");
                    return 0;
                }

                packets = 0;
                received = 0;


                packets = atoi(pc_buf);
                printf("Num packets expected: %d\n", packets);

                while(received<=packets)
                {
                    size_t res2 = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf)  , SOCK_NO_TIMEOUT, &remote);

                    if (res2 <= 0)
                    {
                        printf("Failed to get Data from file");
                        return 0;
                    }

                    if((fwrite(pc_buf,1,res2,newfp)) < res2)
                    {
                        printf("error in writing to the file\n");
                        return 0;
                    }
                    printf("%d\n",received);
                    received++;
                }

                printf("Got the data from the pc\n");

                fclose(newfp);

                //send_update(arg);

            }   
        }   
    }
}
}   
