/**
 * @file udp_listener.h
 * @brief Local control transport: answers signed SinricPro commands over the LAN
 *
 * Built on lwIP's raw UDP API. This port compiles with LWIP_SOCKET and
 * LWIP_NETCONN off, so there are no BSD sockets and no netconn available.
 */

#ifndef SINRICPRO_UDP_LISTENER_H
#define SINRICPRO_UDP_LISTENER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "message_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Port the SinricPro app talks to. */
#define SINRICPRO_UDP_PORT 3333

/** SinricPro local-control multicast group. */
#define SINRICPRO_UDP_MULTICAST_IP "224.9.9.9"

/**
 * @brief Start listening for local-control requests
 *
 * Binds to any address so the one PCB takes unicast, multicast and subnet
 * broadcast alike; broadcast is how a client finds the device on networks that
 * drop multicast. Received datagrams are queued for the caller's loop -- the
 * lwIP receive callback does no parsing, verification or dispatch.
 *
 * A failed multicast join is logged but not fatal: unicast and broadcast still
 * work without it.
 *
 * @param rx_queue Queue receiving inbound requests, with the sender's address
 * @return true if the listener is bound
 */
bool sinricpro_udp_start(sinricpro_queue_t *rx_queue);

/**
 * @brief Send a reply to the peer a request came from
 *
 * Sent from the listening PCB. A separate send-only PCB is not used: on lwIP
 * that has been observed to report success while transmitting nothing.
 *
 * @param message   Null-terminated message
 * @param length    Message length
 * @param peer_addr Peer IPv4 in network order
 * @param peer_port Peer port
 * @return true if lwIP accepted the datagram
 */
bool sinricpro_udp_send(const char *message, size_t length,
                        uint32_t peer_addr, uint16_t peer_port);

/**
 * @brief Stop listening and release the PCB
 */
void sinricpro_udp_stop(void);

/**
 * @brief Whether the listener is currently bound
 */
bool sinricpro_udp_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* SINRICPRO_UDP_LISTENER_H */
