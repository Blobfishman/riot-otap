#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"

#include "packet_logger.h"

static char server_stack[THREAD_STACKSIZE_DEFAULT];

extern void* udp_server(void* arg);

int main(void)
{

    /* Uncomment the following lines to enable packet logging */
    //gnrc_netreg_entry_t dump;
    //enable_packet_logging(dump);

    thread_create(server_stack, sizeof(server_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        udp_server,
        NULL, "udp_server");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
