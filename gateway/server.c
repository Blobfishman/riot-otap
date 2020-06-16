#include <stdio.h>
#include <string.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "vfs.h"

#define SIZE 256
extern int app1(void);
extern int app2(void);

int (*app)(void);

//buffer for the messages
uint8_t buf[SIZE];
char pc_buf[SIZE];

int packets = 0;
int received = 0;

int newfp;

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
    (void)arg;

    sock_udp_ep_t remote = { .family = AF_INET6 };
    sock_udp_t sock;
    remote.port = client_multicast_port;

    if (sock_udp_create(&sock, &remote, NULL, 0) < 0) {
        puts("Error creating UDP sock");
        return 1;
    }

    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t*)&remote.addr.ipv6, IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    char buffer[SIZE];

    mode_t mode = 0;

    int fptr = vfs_open("update.elf",0, mode);

    if (fptr < 0) {
        printf("Error opening file");
        return -1;
    }

    vfs_lseek(fptr, 0, SEEK_END);
    
    vfs_lseek(fptr, 0, SEEK_SET);

    int packets = 0;

    
    packets = 1;
   
    char packsize[100];
    sprintf(packsize, "%d",packets);
    if (sock_udp_send(&sock, packsize, sizeof(packsize), &remote) < 0) {
        puts("Error sending message");
        sock_udp_close(&sock);
        return 1;
    }

    memset(buffer, 0, SIZE);
    

    //read and send file
    int byte_read;
    while ((byte_read = vfs_read(fptr,buffer, SIZE)) > 0) {
        if (byte_read != SIZE) {
            if (vfs_read(fptr,buffer, SIZE) > 0 || vfs_read(fptr,buffer, SIZE) < 0 ) {
                printf("Reading failed!!");
                return -1;
            }
        }

        //usleep(4200);

        if (byte_read > 0) {

            //send chuncked packets

            if (sock_udp_send(&sock, buffer, byte_read, &remote) < 0) {
                puts("Error sending chunked packages");
                sock_udp_close(&sock);
                return 1;
            }

       
        }
    }
    vfs_close(fptr);
    sock_udp_close(&sock);
    return 0;
}

//Method to receive Data from Nodes
void* receive_data(void* arg)
{
    (void)arg;
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

    puts("sending message");
    if (sock_udp_send(&sock, "10", sizeof("10"), &nodes) < 0) {
        puts("Error sending message");
        sock_udp_close(&sock);
        return 0;
    }

    while (1) {
        sock_udp_ep_t remote;
        ssize_t res;
        if ((res = sock_udp_recv(&sock, buf, sizeof(buf) - 1, SOCK_NO_TIMEOUT, &remote)) > 0) {
            puts("Received a message from a node");

            if (connection_to_pc) {
                send_data_to_pc(res);
            }
        }
    }
}

//Method to receive Data from the PC
int* receive_update(void* arg)
{
    puts("started pc thread");

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

        res = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf), SOCK_NO_TIMEOUT, &remote);

        if (res > 0) {
            puts("Udp_Server start for PC");

            server = remote;

            connection_to_pc = true;

            while (1) {
                if ((res = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf)-1, SOCK_NO_TIMEOUT, &remote)) > 0) {
                    
                    printf("%s\n",pc_buf); }

                if ((res = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf)-1, SOCK_NO_TIMEOUT, &remote)) > 0) {

                    printf("%s\n",pc_buf); }


        
                    mode_t mode = 0;

                    newfp = vfs_open("update.elf",1, mode );



                    if (newfp < 0) {
                        printf("error opening the file\n");
                        return 0;
                    }

                    packets = 0;
                    received = 0;
                    int package_size;
                    memcpy(buf,pc_buf+1,4);
                    package_size = atoi((char *)(&buf));

                    printf("%d",package_size);


                    packets = atoi(pc_buf);
                    printf("Num packets expected: %d\n", packets);

                    while (received < packets) {
                        size_t res2 = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf), SOCK_NO_TIMEOUT, &remote);

                        if (res2 <= 0) {
                            printf("Failed to get Data from file");
                            return 0;
                        }

                        if (((size_t) vfs_write(newfp, pc_buf, SIZE)) < res2) {
                            printf("error in writing to the file\n");
                            return 0;
                        }
                        //printf("%d\n", received);
                        received++;
                    }

                    printf("Got the data from the pc\n");

                    vfs_close(newfp);

                    send_update(arg);

                }
            }
        }
    }
   

