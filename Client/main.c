#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"

static char server_stack[THREAD_STACKSIZE_DEFAULT];

extern int udp_send(int argc, char **argv);
extern void *udp_server(void *arg);

static const shell_command_t commands[] = {
    {"udp_send", "send a message", udp_send},
    { NULL, NULL, NULL }
};

int main(void) {

    thread_create(server_stack, sizeof(server_stack),
                    THREAD_PRIORITY_MAIN - 1,
                    THREAD_CREATE_STACKTEST,
                    udp_server,
                    NULL, "udp_server");
    
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
