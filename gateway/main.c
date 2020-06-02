#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "thread.h"

static char server_stack_note[THREAD_STACKSIZE_DEFAULT];
static char server_stack_pc[THREAD_STACKSIZE_DEFAULT];

extern void* receive_data(void* arg);
extern void* receive_update(void* arg);

static const shell_command_t commands[] = {
    { NULL, NULL, NULL }
};

int main(void)
{

    thread_create(server_stack_note, sizeof(server_stack_note),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        receive_data,
        NULL, "receive_data");

    thread_create(server_stack_pc, sizeof(server_stack_pc),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        receive_update,
        NULL, "receive_update");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
