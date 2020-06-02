#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"

static char server_stack[THREAD_STACKSIZE_DEFAULT];

extern void* udp_server(void* arg);

int main(void)
{

    thread_create(server_stack, sizeof(server_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        udp_server,
        NULL, "udp_server");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
