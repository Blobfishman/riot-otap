#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "xtimer.h"

static char updater_stack[THREAD_STACKSIZE_DEFAULT];

void start_application()
{
    thread_create(updater_stack, sizeof(updater_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        &application_thread,
        NULL, "application_thread");
}

void* application_thread(void* arg)
{
    printf("start application thread");
    (void)arg;

    // setup socket
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = node_data_port;
    sock_udp_t sock;
    int socket_status = sock_udp_create(&sock, &local, NULL, 0);
    if (socket_status < 0) {
        printf("Error: creating socket failed: %d", socket_status);
        return NULL;
    }

    //set remote connection info
    sock_udp_ep_t network_devices = { .family = AF_INET6 };
    network_devices.port = gateway_data_port;
    // a multicast is used as workaround due to the missing gateway ip
    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t*)&network_devices.addr.ipv6,
        IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    // sending routine
    while (1) {
        char* data = "node msg";
        printf("sending message");
        int send_status = sock_udp_send(&sock, &data, sizeof(data), &network_devices);
        if (send_status < 0) {
            printf("Error: sending message failed: %d", send_status);
            // never stop sending data even if it fails
        }
        xtimer_sleep(1);
    }
}
