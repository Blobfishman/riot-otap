#include <stdio.h>

#include "fmt.h"
#include "thread.h"
#include "xtimer.h"
#include "shell.h"
#include "shell_commands.h"
#include "net/gnrc.h"

/**
 * @brief   Buffer size used by the shell
 */
#define SHELL_BUFSIZE           (64U)

/**
 * @brief   Priority of the RAW dump thread
 */
#define RAWDUMP_PRIO            (THREAD_PRIORITY_MAIN - 1)

/**
 * @brief   Message queue size of the RAW dump thread
 */
#define RAWDUMP_MSG_Q_SIZE      (32U)

/**
 * @brief   Stack for the raw dump thread
 */
static char rawdmp_stack[THREAD_STACKSIZE_SMALL];

/**
 * @brief   Make a raw dump of the given packet contents
 */
void dump_pkt(gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *snip = pkt;
    uint8_t lqi = 0;
    if (pkt->next) {
        if (pkt->next->type == GNRC_NETTYPE_NETIF) {
            gnrc_netif_hdr_t *netif_hdr = pkt->next->data;
            lqi = netif_hdr->lqi;
            pkt = gnrc_pktbuf_remove_snip(pkt, pkt->next);
        }
    }
    uint64_t now_us = xtimer_usec_from_ticks64(xtimer_now64());

    print_str("rftest-rx --- len ");
    print_u32_hex((uint32_t)gnrc_pkt_len(pkt));
    print_str(" lqi ");
    print_byte_hex(lqi);
    print_str(" rx_time ");
    print_u64_hex(now_us);
    print_str("\n");
    while (snip) {
        for (size_t i = 0; i < snip->size; i++) {
            print_byte_hex(((uint8_t *)(snip->data))[i]);
            print_str(" ");
        }
        snip = snip->next;
    }
    print_str("\n\n");

    gnrc_pktbuf_release(pkt);
}

/**
 * @brief   Event loop of the RAW dump thread
 *
 * @param[in] arg   unused parameter
 */
void *rawdump(void *arg)
{
    msg_t msg_q[RAWDUMP_MSG_Q_SIZE];

    (void)arg;
    msg_init_queue(msg_q, RAWDUMP_MSG_Q_SIZE);
    while (1) {
        msg_t msg;

        msg_receive(&msg);
        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                dump_pkt((gnrc_pktsnip_t *)msg.content.ptr);
                break;
            default:
                /* do nothing */
                break;
        }
    }

    /* never reached */
    return NULL;
}

/**
 * @brief
 */
int main(void)
{

    /* v This part should be copied into every client and server main v */

    gnrc_netreg_entry_t dump;

    dump.target.pid = thread_create(rawdmp_stack, sizeof(rawdmp_stack), RAWDUMP_PRIO,
                                    THREAD_CREATE_STACKTEST, rawdump, NULL, "rawdump");
    dump.demux_ctx = GNRC_NETREG_DEMUX_CTX_ALL;
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &dump);

    /* ^ This part should be copied into every client and server main ^ */

    puts("All your packets are belong to us");

    /* start the shell */
    puts("All ok, starting the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
