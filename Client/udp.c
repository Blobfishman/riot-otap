#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "xtimer.h"
// #include "fmt.h" 

static char client_stack[THREAD_STACKSIZE_DEFAULT];
// buffer fuer die empfangenen daten
uint8_t server_buffer[128];
uint8_t client_buffer[128];
uint16_t server_port = 8888;
uint16_t client_port = 7777;

sock_udp_ep_t router;

extern char app1(void);
extern char app2(void);

char (*app)(void);

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
    
    char data = 0;
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
        if (sock_udp_recv(&sock, server_buffer, sizeof(server_buffer), 
                                                SOCK_NO_TIMEOUT,
                                                &remote) >= 0) {
            //Nachricht printen
            printf("Message von erhalten: %s\n", server_buffer);

            // gleiche nachricht zurueck schicken (nachher aendern)
            if (sock_udp_send(&sock, "server acc", sizeof("server acc"),
                                 &remote) < 0) {
                puts("Error sending reply");
            }

            // client starten oder update simulieren
            if(atoi((char*)server_buffer) == 10) {
                puts("starte Client");
                app = &app1;
                // router ip und port setzen
                router = remote;
                router.port = server_port;

                thread_create(client_stack, sizeof(client_stack),
                                THREAD_PRIORITY_MAIN - 1,
                                THREAD_CREATE_STACKTEST,
                                udp_client,
                                NULL, "udp_client");

            }
            if(atoi((char*)server_buffer) == 20) {
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

