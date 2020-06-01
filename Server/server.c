#include <stdio.h>
#include <string.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "xtimer.h"


extern int app1(void);
extern int app2(void);


int (*app)(void);


//buffer for the messages
uint8_t buf[128];
uint8_t pc_buf[128];


//ports for the nodes and the pc
static uint16_t server_port = 8888;
static uint16_t client_multicast_port = 7777;


static uint16_t pc_server_port = 6666;
static uint16_t pc_client_port = 5555;




//Method to send Data to the pc
int send_data_to_pc(int res)
{
    sock_udp_ep_t remote = { .family = AF_INET6 };
   char * adresse = "fe80::70c4:aff:feb0:a637";
   ipv6_addr_from_str((ipv6_addr_t*) &remote.addr, adresse);
    if (ipv6_addr_is_link_local((ipv6_addr_t *)&remote.addr)) {
    /* choose first interface when address is link local */
    	gnrc_netif_t *netif = gnrc_netif_iter(NULL);
    	remote.netif = (uint16_t)netif->pid;
    }
    remote.port = pc_client_port;

    //sock structur deklarieren
    sock_udp_t sock;
    if (sock_udp_create(&sock, &remote, NULL, 0) < 0) {
        puts("Error creating UDP sock to send data to pc");
        return 1;
    }

        if (sock_udp_send(&sock, pc_buf, res, &remote) < 0)
        {
        puts("Error sending message to pc");
        //sock_udp_close(&sock);
        return 1;
        }
    sock_udp_close(&sock);
    puts("Send message\n");
    return 0;

}



//send update via Multicast to all nodes
int send_update(int res)
{
    sock_udp_ep_t remote = { .family = AF_INET6 };
    sock_udp_t sock;
    remote.port = client_multicast_port;

        if (sock_udp_create(&sock, &remote, NULL, 0) < 0) {
        puts("Error creating UDP sock");
        return 1;
    }



    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6,
                                    IPV6_ADDR_MCAST_SCP_LINK_LOCAL);


    //send Data to the end-point


    for(int i = 0; i < res; i++)
    {
        if (sock_udp_send(&sock, &pc_buf[i], sizeof(pc_buf[i]), NULL) < 0)
        {
        puts("Error sending update to Nodes");
        //sock_udp_close(&sock);
        sock_udp_close(&sock);
        return 1;
        }
    }

    return 0;

}






//Method to receive Data from Nodes
void *receive_data(void *arg)
{
    puts("Udp_Server start for Nodes");

    (void) arg;

    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = server_port;
    sock_udp_t sock;


    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP sock for nodes");
        return NULL;
    }

    while (1) {
        sock_udp_ep_t remote;
        ssize_t res;
        if ((res = sock_udp_recv(&sock, buf, sizeof(buf) -1, SOCK_NO_TIMEOUT,
                                 &remote)) > 0) {
            puts("Received a message from a node");

            send_data_to_pc(res);

            if (sock_udp_send(&sock, buf, res, &remote) < 0) {
                puts("Error sending reply to node");
                return NULL;
            }
        }
    }
}






//Method to receive Data from the PC
int *receive_update(void *arg)
{
    puts("Udp_Server start for PC");

    (void) arg;

    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = pc_server_port;
    sock_udp_t sock;


    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error creating UDP sock");
        return NULL;
    }


    while (1) {
        sock_udp_ep_t remote;
        ssize_t res;
        if ((res = sock_udp_recv(&sock, pc_buf, sizeof(pc_buf) -1, SOCK_NO_TIMEOUT,
                                 &remote)) > 0) {
            puts("Received a message");


            send_data_to_pc(res);

        }
    }
}
