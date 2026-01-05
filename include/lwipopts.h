/**
 * @file lwipopts.h
 * @brief lwIP configuration for SinricPro Pico SDK
 *
 * This file configures lwIP for use with the Raspberry Pi Pico W
 * and the SinricPro SDK.
 */

#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally use defaults provided by pico-sdk
// but can override here

// Allow old style includes
#ifndef LWIP_NO_STDDEF_H
#define LWIP_NO_STDDEF_H 0
#endif

// Common settings used in Pico W projects
#define NO_SYS                          1
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0

#define MEM_LIBC_MALLOC                 0
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        16384

#define MEMP_NUM_TCP_SEG                32
#define MEMP_NUM_ARP_QUEUE              10

#define PBUF_POOL_SIZE                  24

#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_ICMP                       1
#define LWIP_RAW                        1
#define LWIP_TCP                        1
#define LWIP_UDP                        1
#define LWIP_DHCP                       1
#define LWIP_IPV4                       1
#define LWIP_DNS                        1

#define TCP_MSS                         1460
#define TCP_WND                         (8 * TCP_MSS)
#define TCP_SND_BUF                     (8 * TCP_MSS)
#define TCP_SND_QUEUELEN                ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_HOSTNAME             1

#define LWIP_CHKSUM_ALGORITHM           3

// TLS/SSL support via altcp
#define LWIP_ALTCP                      1
#define LWIP_ALTCP_TLS                  1
#define LWIP_ALTCP_TLS_MBEDTLS          1

// Debug options (disable for release)
#define LWIP_DEBUG                      0
#define TCP_DEBUG                       LWIP_DBG_OFF
#define ALTCP_MBEDTLS_DEBUG             LWIP_DBG_OFF

// Performance tuning
#define LWIP_TCP_KEEPALIVE              1
#define LWIP_RAND()                     ((u32_t)rand())

#endif /* _LWIPOPTS_H */
