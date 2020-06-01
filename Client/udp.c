#include <stdio.h>
// #include <string.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "xtimer.h"
#include "fmt.h" 

extern void print_ipv6(void);


static char client_stack[THREAD_STACKSIZE_DEFAULT];
// buffer fuer die empfangenen daten
static char data_buffer[128];
static char buf[128];
static char client_buffer[128];
static uint16_t server_port = 8888;
static uint16_t client_port = 7777;

sock_udp_ep_t router;

extern int app1(void);
extern int app2(void);

int (*app)(void);

// sendet die message die der server eigenlich sendet
// nur zum debuggen
int udp_send(int argc, char **argv) {
    
    // bei falscher eingabe
    if (argc != 4) {
        puts("Usage: udp <ipv6-addr> <port> <payload>");
        return -1;
    
    }
    // lokale addresse IPV6 und server port
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = 6666;

    //sock structur deklarieren
    sock_udp_t sock;
    // erstelle den socket mit IPV6 addr und port
    // die client addresse kann auf NULL gesetzt werden
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error: sock konnte nicht erstellt werden");
        return -1;
    }

    //
    // kopierter code
    //
    int res;
    sock_udp_ep_t remote = { .family = AF_INET6 };

    // addresse formatieren
    if (ipv6_addr_from_str((ipv6_addr_t *)&remote.addr, argv[1]) == NULL) {
        puts("Error: unable to parse destination address");
        return 1;
    }

    
    if (ipv6_addr_is_link_local((ipv6_addr_t *)&remote.addr)) {
        /* choose first interface when address is link local */
        puts("address ist link local ?!");
        gnrc_netif_t *netif = gnrc_netif_iter(NULL);
        remote.netif = (uint16_t)netif->pid;
    }
    // port formatieren
    remote.port = atoi(argv[2]);

    if (sock_udp_send(&sock, argv[3], sizeof(argv[3]), &remote) < 0) {
        puts("Error sending message");
        //sock_udp_close(&sock);
        return 1;
    }
    // warte auf antwort 
    // timeout,  wenn antwort in 1s nicht kommt
    if ((res = sock_udp_recv(&sock, buf, sizeof(buf), 1 * US_PER_SEC,
                            NULL)) < 0) {
        if (res == -ETIMEDOUT) {
            puts("Timed out");
        }
        else {
            puts("Error receiving message");
        }
    }
    else {
        printf("Received message: \"");
        for (int i = 0; i < res; i++) {
            printf("%c", buf[i]);
        }
        printf("\"\n");
    }
    return 1;
}

void *udp_client(void *arg) {
    
    (void)arg;

    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = client_port;

    //sock structur deklarieren
    sock_udp_t sock;
    // erstelle den socket mit IPV6 addr und port
    // die client addresse kann auf NULL gesetzt werden
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error: sock konnte nicht erstellt werden");
        return NULL;
    }

    int data = 0;
    while (1) {
        ssize_t res;
        data = app();
        if (sock_udp_send(&sock, &data, sizeof(data), &router) < 0) {
            puts("Error sending message");
            return NULL;
        }
        if ((res = sock_udp_recv(&sock, client_buffer, sizeof(client_buffer), 1 * US_PER_SEC,
                                NULL)) < 0) {
            if (res == -ETIMEDOUT) {
                puts("Timed out client");
            }
            else {
                puts("Error receiving message");
            }
        }
        else {
            printf("Received message: \"");
            for (int i = 0; i < res; i++) {
                printf("%c", client_buffer[i]);
            }
            printf("\"\n");
        }
        xtimer_sleep(1);
    }
}

void *udp_server(void *arg) {
    
    puts("start udp_server");

    (void)arg;
    // lokale addresse IPV6 und server port
    sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
    local.port = server_port;

    //sock structur deklarieren
    sock_udp_t sock;
    // erstelle den socket mit IPV6 addr und port
    // die client addresse kann auf NULL gesetzt werden
    if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
        puts("Error: sock konnte nicht erstellt werden");
        return NULL;
    }

    while (true) {
        // addresse vom sender
        sock_udp_ep_t remote;

        // empfange pakete 
        if (sock_udp_recv(&sock, data_buffer, sizeof(data_buffer)-1, 
                                                SOCK_NO_TIMEOUT,
                                                &remote) >= 0) {
            //Nachricht printen
            printf("Message von erhalten: %s\n", data_buffer);

            // gleiche nachricht zurueck schicken (nachher aendern)
            if (sock_udp_send(&sock, "server acc", sizeof("server acc"),
                                 &remote) < 0) {
                puts("Error sending reply");
            }

            // client starten oder update simulieren
            if(atoi(data_buffer) == 10) {
                puts("starte Client");
                app = &app1;
                // router ip und port setzen
                router = remote;
                char ipv6_addr[IPV6_ADDR_MAX_STR_LEN];
                ipv6_addr_to_str(ipv6_addr,(ipv6_addr_t *)&local.addr.ipv6, IPV6_ADDR_MAX_STR_LEN);
                printf("my addr: %s\n", ipv6_addr);

                thread_create(client_stack, sizeof(client_stack),
                                THREAD_PRIORITY_MAIN - 1,
                                THREAD_CREATE_STACKTEST,
                                udp_client,
                                NULL, "udp_client");

            }
            if(atoi(data_buffer) == 20) {
                puts("update");
                if(app == &app1) {
                    app = &app2;
                }
                else {
                    app = &app1;
                }
            }
        }
        else {
            puts("Error beim empfangen");
        }
    }


    return NULL;
}

void print_ipv6(void) {
    print_byte_hex(*router.addr.ipv6);
    puts("\n");
}

