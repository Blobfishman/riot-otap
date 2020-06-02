#include <stdio.h>

#include "fmt.h"
#include "thread.h"
#include "xtimer.h"
#include "shell.h"
#include "shell_commands.h"
#include "net/gnrc.h"

/**
 * @brief   Priority of the RAW dump thread
 */
#define RAWDUMP_PRIO            (THREAD_PRIORITY_MAIN - 1)

/**
 * @brief   Message queue size of the RAW dump thread
 */
#define RAWDUMP_MSG_Q_SIZE      (32U)

void dump_pkt(gnrc_pktsnip_t *pkt);
void *rawdump(void *arg);
void enable_packet_logging(gnrc_netreg_entry_t gnrc_entry);
