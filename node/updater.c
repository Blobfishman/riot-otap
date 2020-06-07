#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "xtimer.h"

static char updater_stack[THREAD_STACKSIZE_DEFAULT];
char update_package[UPDATE_PACKAGE_SIZE];

sock_udp_ep_t router;

void start_updater()
{
    thread_create(updater_stack, sizeof(updater_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        &updater_thread,
        NULL, "udp_server");
}

void* updater_thread(void* arg)
{
    printf("start updater thread");
    bool running = false;
    (void)arg;

    // need to receive update
    int packets = 0;
    int received = 0;
    FILE* newfp;

    // setup socket
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = node_update_port;
    sock_udp_t sock;
    int socket_status = sock_udp_create(&sock, &local, NULL, 0);
    if (socket_status < 0) {
        printf("Error: creating socket failed: %d", socket_status);
        return NULL;
    }

    while (true) {
        // sender address
        sock_udp_ep_t remote;

        // receive package
        int receive_status = sock_udp_recv(&sock, update_package, sizeof(update_package), SOCK_NO_TIMEOUT, &remote));
        if (receive_status > 0) {
            printf("update");
            newfp = fopen("update1.elf", "wb");

            if (newfp == NULL) {
                printf("error opening the file\n");
                return 0;
            }

            packets = atoi(update_package);
            printf("Num packets expected: %d\n", packets);

            while (received < packets) {
                size_t res2 = sock_udp_recv(&sock, update_package, sizeof(update_package), SOCK_NO_TIMEOUT, &remote);

                if (res2 <= 0) {
                    printf("Failed to get Data from file");
                    return 0;
                }

                if ((fwrite(update_package, 1, res2, newfp)) < res2) {
                    printf("error in writing to the file\n");
                    return 0;
                }
                printf("%d\n", received);
                received++;
            }

            printf("Got the data from the server\n");

            fclose(newfp);
        } else {
            printf("Error: receiving data failed: %d", receive_status);
        }
    }
    return NULL;
}
