/**
 * @file udp_listener.c
 * @brief Local control transport on lwIP's raw UDP API
 */

#include "udp_listener.h"
#include "sinricpro_debug.h"

#include <string.h>

#include "lwip/igmp.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

static struct udp_pcb *udp_pcb = NULL;
static sinricpro_queue_t *udp_rx_queue = NULL;

/**
 * lwIP receive callback.
 *
 * Runs in lwIP's context, so it copies the datagram and the peer and returns.
 * Verifying a signature or dispatching a device callback here would block the
 * stack, and the failures that causes are intermittent rather than obvious.
 */
static void udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                        const ip_addr_t *addr, u16_t port) {
    (void)arg;
    (void)pcb;

    if (p == NULL) {
        return;
    }

    if (udp_rx_queue != NULL && p->tot_len > 0 &&
        p->tot_len < SINRICPRO_MAX_MESSAGE_SIZE) {
        char buffer[SINRICPRO_MAX_MESSAGE_SIZE];
        const u16_t copied = pbuf_copy_partial(p, buffer, p->tot_len, 0);

        if (copied > 0) {
            buffer[copied] = '\0';
            sinricpro_queue_push_peer(udp_rx_queue, SINRICPRO_IF_UDP, buffer,
                                      copied, ip_addr_get_ip4_u32(addr), port);
        }
    }

    pbuf_free(p);
}

bool sinricpro_udp_start(sinricpro_queue_t *rx_queue) {
    if (udp_pcb != NULL) {
        return true;
    }

    if (rx_queue == NULL) {
        return false;
    }

    udp_pcb = udp_new();
    if (udp_pcb == NULL) {
        SINRICPRO_ERROR_PRINTF("[SinricPro] UDP: out of PCBs\n");
        return false;
    }

    /* Bound to any address rather than the group: the same PCB then takes
     * unicast and subnet broadcast too. */
    if (udp_bind(udp_pcb, IP_ANY_TYPE, SINRICPRO_UDP_PORT) != ERR_OK) {
        SINRICPRO_ERROR_PRINTF("[SinricPro] UDP: cannot bind port %d\n",
                               SINRICPRO_UDP_PORT);
        udp_remove(udp_pcb);
        udp_pcb = NULL;
        return false;
    }

    udp_rx_queue = rx_queue;
    udp_recv(udp_pcb, udp_recv_cb, NULL);

    /* Report the join either way. A silent failure here is how local control
     * ends up looking broken for no visible reason. */
    ip4_addr_t group;
    bool joined = false;

    if (ip4addr_aton(SINRICPRO_UDP_MULTICAST_IP, &group)) {
        joined = (igmp_joingroup(IP4_ADDR_ANY4, &group) == ERR_OK);
    }

    if (!joined) {
        SINRICPRO_WARN_PRINTF("[SinricPro] UDP: could not join %s; "
                              "unicast and broadcast still work\n",
                              SINRICPRO_UDP_MULTICAST_IP);
    }

    SINRICPRO_DEBUG_PRINTF("[SinricPro] UDP: listening on %d, multicast joined=%d\n",
                           SINRICPRO_UDP_PORT, (int)joined);
    return true;
}

bool sinricpro_udp_send(const char *message, size_t length,
                        uint32_t peer_addr, uint16_t peer_port) {
    if (udp_pcb == NULL || message == NULL || length == 0 || peer_port == 0) {
        return false;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)length, PBUF_RAM);
    if (p == NULL) {
        SINRICPRO_ERROR_PRINTF("[SinricPro] UDP: no pbuf for reply\n");
        return false;
    }

    memcpy(p->payload, message, length);

    ip_addr_t dest;
    ip_addr_set_ip4_u32(&dest, peer_addr);

    const err_t err = udp_sendto(udp_pcb, p, &dest, peer_port);
    pbuf_free(p);

    if (err != ERR_OK) {
        SINRICPRO_ERROR_PRINTF("[SinricPro] UDP: reply failed (%d)\n", (int)err);
        return false;
    }

    return true;
}

void sinricpro_udp_stop(void) {
    if (udp_pcb == NULL) {
        return;
    }

    udp_remove(udp_pcb);
    udp_pcb = NULL;
    udp_rx_queue = NULL;
}

bool sinricpro_udp_is_running(void) {
    return udp_pcb != NULL;
}
