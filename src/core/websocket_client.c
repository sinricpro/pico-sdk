/**
 * @file websocket_client.c
 * @brief WebSocket client implementation for SinricPro
 *
 * Uses lwIP (altcp) with mbedTLS for secure WebSocket connections.
 */

#include "websocket_client.h"
#include "sinricpro/sinricpro_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/rand.h"

#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"

#include "mbedtls/base64.h"
#include "mbedtls/sha1.h"

// Buffer sizes
#define WS_TX_BUFFER_SIZE   SINRICPRO_WEBSOCKET_BUFFER_SIZE
#define WS_RX_BUFFER_SIZE   SINRICPRO_WEBSOCKET_BUFFER_SIZE
#define WS_KEY_LENGTH       24  // Base64 encoded 16 bytes

// WebSocket magic GUID for handshake
static const char *WS_MAGIC_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Connection state
typedef struct {
    sinricpro_ws_state_t state;
    sinricpro_ws_config_t config;

    // lwIP connection
    struct altcp_pcb *pcb;
    ip_addr_t server_ip;

    // Buffers
    uint8_t tx_buffer[WS_TX_BUFFER_SIZE];
    uint8_t rx_buffer[WS_RX_BUFFER_SIZE];
    size_t rx_len;

    // WebSocket handshake
    char ws_key[WS_KEY_LENGTH + 1];
    bool handshake_complete;

    // Ping/Pong timing
    uint32_t last_ping_sent;
    uint32_t last_pong_received;
    bool ping_pending;

    // Reconnection
    bool auto_reconnect;
    uint32_t reconnect_delay_ms;
    uint32_t last_disconnect_time;

    // Frame parsing state
    bool frame_in_progress;
    uint8_t frame_opcode;
    size_t frame_length;
    size_t frame_received;
    uint8_t frame_mask[4];
    bool frame_masked;

} ws_context_t;

static ws_context_t ws_ctx;
static bool ws_initialized = false;

// Forward declarations
static err_t ws_tcp_connected(void *arg, struct altcp_pcb *pcb, err_t err);
static err_t ws_tcp_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err);
static void ws_tcp_err(void *arg, err_t err);
static err_t ws_tcp_sent(void *arg, struct altcp_pcb *pcb, u16_t len);
static void ws_dns_callback(const char *name, const ip_addr_t *addr, void *arg);
static void ws_send_handshake(void);
static bool ws_parse_handshake_response(const char *response, size_t len);
static void ws_process_frame(const uint8_t *data, size_t len);
static void ws_set_state(sinricpro_ws_state_t new_state);
static void ws_generate_key(char *key_out);
static size_t ws_encode_frame(uint8_t opcode, const uint8_t *data, size_t len,
                              uint8_t *output, size_t output_len);

// Get current time in milliseconds
static uint32_t get_millis(void) {
    return to_ms_since_boot(get_absolute_time());
}

bool sinricpro_ws_init(void) {
    if (ws_initialized) return true;

    memset(&ws_ctx, 0, sizeof(ws_ctx));
    ws_ctx.state = WS_STATE_DISCONNECTED;
    ws_ctx.auto_reconnect = true;
    ws_ctx.reconnect_delay_ms = SINRICPRO_WEBSOCKET_RECONNECT_DELAY_MS;

    ws_initialized = true;
    return true;
}

bool sinricpro_ws_connect(const sinricpro_ws_config_t *config) {
    if (!ws_initialized || !config || !config->host) {
        return false;
    }

    // Already connecting or connected
    if (ws_ctx.state != WS_STATE_DISCONNECTED &&
        ws_ctx.state != WS_STATE_ERROR) {
        return false;
    }

    // Store config
    memcpy(&ws_ctx.config, config, sizeof(sinricpro_ws_config_t));

    // Reset state
    ws_ctx.rx_len = 0;
    ws_ctx.handshake_complete = false;
    ws_ctx.ping_pending = false;
    ws_ctx.frame_in_progress = false;
    ws_ctx.last_pong_received = get_millis();

    // Generate WebSocket key
    ws_generate_key(ws_ctx.ws_key);

    // Start DNS lookup
    ws_set_state(WS_STATE_DNS_LOOKUP);

    err_t err = dns_gethostbyname(config->host, &ws_ctx.server_ip,
                                  ws_dns_callback, NULL);

    if (err == ERR_OK) {
        // Already cached - proceed to connect
        ws_dns_callback(config->host, &ws_ctx.server_ip, NULL);
    } else if (err != ERR_INPROGRESS) {
        printf("[WS] DNS lookup failed: %d\n", err);
        ws_set_state(WS_STATE_ERROR);
        return false;
    }

    return true;
}

void sinricpro_ws_disconnect(void) {
    if (ws_ctx.pcb) {
        // Send close frame if connected
        if (ws_ctx.state == WS_STATE_CONNECTED) {
            uint8_t close_frame[6];
            size_t len = ws_encode_frame(WS_OPCODE_CLOSE, NULL, 0,
                                         close_frame, sizeof(close_frame));
            altcp_write(ws_ctx.pcb, close_frame, len, TCP_WRITE_FLAG_COPY);
            altcp_output(ws_ctx.pcb);
        }

        altcp_close(ws_ctx.pcb);
        ws_ctx.pcb = NULL;
    }

    ws_ctx.last_disconnect_time = get_millis();
    ws_set_state(WS_STATE_DISCONNECTED);
}

void sinricpro_ws_handle(void) {
    if (!ws_initialized) return;

    // Poll cyw43 and lwIP
    cyw43_arch_poll();

    uint32_t now = get_millis();

    switch (ws_ctx.state) {
        case WS_STATE_CONNECTED:
            // Check ping interval
            if (ws_ctx.config.ping_interval_ms > 0 &&
                (now - ws_ctx.last_ping_sent) >= ws_ctx.config.ping_interval_ms) {

                if (ws_ctx.ping_pending) {
                    // Ping timeout - disconnect
                    uint32_t pong_age = now - ws_ctx.last_pong_received;
                    if (pong_age > ws_ctx.config.ping_timeout_ms) {
                        printf("[WS] Ping timeout (%lu ms)\n", (unsigned long)pong_age);
                        sinricpro_ws_disconnect();
                    }
                } else {
                    sinricpro_ws_send_ping();
                }
            }
            break;

        case WS_STATE_DISCONNECTED:
        case WS_STATE_ERROR:
            // Auto-reconnect
            if (ws_ctx.auto_reconnect && ws_ctx.config.host) {
                if ((now - ws_ctx.last_disconnect_time) >= ws_ctx.reconnect_delay_ms) {
                    printf("[WS] Attempting reconnect...\n");
                    sinricpro_ws_connect(&ws_ctx.config);
                }
            }
            break;

        default:
            break;
    }
}

bool sinricpro_ws_send(const char *message, size_t length) {
    if (ws_ctx.state != WS_STATE_CONNECTED || !ws_ctx.pcb || !message) {
        return false;
    }

    if (length == 0) {
        length = strlen(message);
    }

    // Encode as text frame
    size_t frame_len = ws_encode_frame(WS_OPCODE_TEXT,
                                       (const uint8_t *)message, length,
                                       ws_ctx.tx_buffer, WS_TX_BUFFER_SIZE);

    if (frame_len == 0) {
        printf("[WS] Failed to encode frame\n");
        return false;
    }

    err_t err = altcp_write(ws_ctx.pcb, ws_ctx.tx_buffer, frame_len,
                            TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("[WS] Send failed: %d\n", err);
        return false;
    }

    altcp_output(ws_ctx.pcb);
    return true;
}

bool sinricpro_ws_send_ping(void) {
    if (ws_ctx.state != WS_STATE_CONNECTED || !ws_ctx.pcb) {
        return false;
    }

    uint8_t ping_frame[6];
    size_t len = ws_encode_frame(WS_OPCODE_PING, NULL, 0,
                                 ping_frame, sizeof(ping_frame));

    err_t err = altcp_write(ws_ctx.pcb, ping_frame, len, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        altcp_output(ws_ctx.pcb);
        ws_ctx.last_ping_sent = get_millis();
        ws_ctx.ping_pending = true;
        return true;
    }

    return false;
}

sinricpro_ws_state_t sinricpro_ws_get_state(void) {
    return ws_ctx.state;
}

bool sinricpro_ws_is_connected(void) {
    return ws_ctx.state == WS_STATE_CONNECTED;
}

uint32_t sinricpro_ws_get_last_pong_age(void) {
    return get_millis() - ws_ctx.last_pong_received;
}

void sinricpro_ws_set_reconnect(bool enabled, uint32_t delay_ms) {
    ws_ctx.auto_reconnect = enabled;
    if (delay_ms > 0) {
        ws_ctx.reconnect_delay_ms = delay_ms;
    }
}

// ============================================================================
// Internal Functions
// ============================================================================

static void ws_set_state(sinricpro_ws_state_t new_state) {
    if (ws_ctx.state != new_state) {
        ws_ctx.state = new_state;

        if (ws_ctx.config.on_state_change) {
            ws_ctx.config.on_state_change(new_state, ws_ctx.config.user_data);
        }
    }
}

static void ws_generate_key(char *key_out) {
    // Generate 16 random bytes
    uint8_t random_bytes[16];
    for (int i = 0; i < 16; i++) {
        random_bytes[i] = (uint8_t)(get_rand_32() & 0xFF);
    }

    // Base64 encode
    size_t olen;
    mbedtls_base64_encode((unsigned char *)key_out, WS_KEY_LENGTH + 1,
                          &olen, random_bytes, 16);
    key_out[olen] = '\0';
}

static void ws_dns_callback(const char *name, const ip_addr_t *addr, void *arg) {
    if (!addr) {
        printf("[WS] DNS lookup failed for %s\n", name);
        ws_set_state(WS_STATE_ERROR);
        return;
    }

    printf("[WS] Resolved %s to %s\n", name, ipaddr_ntoa(addr));
    ip_addr_copy(ws_ctx.server_ip, *addr);

    // Create TCP connection
    ws_set_state(WS_STATE_TCP_CONNECTING);

    struct altcp_pcb *pcb;

    if (ws_ctx.config.use_ssl) {
        // Create TLS PCB
        struct altcp_tls_config *tls_config = altcp_tls_create_config_client(
            NULL, 0);  // No client cert

        if (!tls_config) {
            printf("[WS] Failed to create TLS config\n");
            ws_set_state(WS_STATE_ERROR);
            return;
        }

        pcb = altcp_tls_new(tls_config, IPADDR_TYPE_V4);
    } else {
        // Plain TCP
        pcb = altcp_new(NULL);
    }

    if (!pcb) {
        printf("[WS] Failed to create PCB\n");
        ws_set_state(WS_STATE_ERROR);
        return;
    }

    ws_ctx.pcb = pcb;

    // Set callbacks
    altcp_arg(pcb, NULL);
    altcp_recv(pcb, ws_tcp_recv);
    altcp_err(pcb, ws_tcp_err);
    altcp_sent(pcb, ws_tcp_sent);

    // Connect
    err_t err = altcp_connect(pcb, &ws_ctx.server_ip, ws_ctx.config.port,
                              ws_tcp_connected);

    if (err != ERR_OK) {
        printf("[WS] Connect failed: %d\n", err);
        altcp_close(pcb);
        ws_ctx.pcb = NULL;
        ws_set_state(WS_STATE_ERROR);
    }
}

static err_t ws_tcp_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    if (err != ERR_OK) {
        printf("[WS] TCP connect error: %d\n", err);
        ws_set_state(WS_STATE_ERROR);
        return err;
    }

    printf("[WS] TCP connected\n");

    if (ws_ctx.config.use_ssl) {
        ws_set_state(WS_STATE_TLS_HANDSHAKE);
        // TLS handshake happens automatically with altcp_tls
    }

    ws_set_state(WS_STATE_WS_HANDSHAKE);
    ws_send_handshake();

    return ERR_OK;
}

static void ws_send_handshake(void) {
    char request[512];
    const char *path = ws_ctx.config.path ? ws_ctx.config.path : "/";

    // Build HTTP upgrade request with SinricPro headers
    int len = snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n",
        path,
        ws_ctx.config.host,
        ws_ctx.ws_key);

    // Add SinricPro specific headers
    if (ws_ctx.config.app_key) {
        len += snprintf(request + len, sizeof(request) - len,
            "appkey: %s\r\n", ws_ctx.config.app_key);
    }

    if (ws_ctx.config.device_ids) {
        len += snprintf(request + len, sizeof(request) - len,
            "deviceids: %s\r\n", ws_ctx.config.device_ids);
    }

    len += snprintf(request + len, sizeof(request) - len,
        "restoredevicestates: false\r\n");

    if (ws_ctx.config.platform) {
        len += snprintf(request + len, sizeof(request) - len,
            "platform: %s\r\n", ws_ctx.config.platform);
    }

    if (ws_ctx.config.sdk_version) {
        len += snprintf(request + len, sizeof(request) - len,
            "SDKVersion: %s\r\n", ws_ctx.config.sdk_version);
    }

    len += snprintf(request + len, sizeof(request) - len, "\r\n");

    err_t err = altcp_write(ws_ctx.pcb, request, len, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        altcp_output(ws_ctx.pcb);
        printf("[WS] Handshake sent\n");
    } else {
        printf("[WS] Failed to send handshake: %d\n", err);
        ws_set_state(WS_STATE_ERROR);
    }
}

static err_t ws_tcp_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (!p) {
        // Connection closed
        printf("[WS] Connection closed by server\n");
        sinricpro_ws_disconnect();
        return ERR_OK;
    }

    if (err != ERR_OK) {
        pbuf_free(p);
        return err;
    }

    // Copy data to receive buffer
    struct pbuf *q = p;
    while (q != NULL) {
        size_t copy_len = q->len;
        if (ws_ctx.rx_len + copy_len > WS_RX_BUFFER_SIZE) {
            copy_len = WS_RX_BUFFER_SIZE - ws_ctx.rx_len;
        }

        memcpy(ws_ctx.rx_buffer + ws_ctx.rx_len, q->payload, copy_len);
        ws_ctx.rx_len += copy_len;
        q = q->next;
    }

    altcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    // Process received data
    if (!ws_ctx.handshake_complete) {
        // Look for end of HTTP response
        ws_ctx.rx_buffer[ws_ctx.rx_len] = '\0';
        char *header_end = strstr((char *)ws_ctx.rx_buffer, "\r\n\r\n");

        if (header_end) {
            size_t header_len = (header_end + 4) - (char *)ws_ctx.rx_buffer;

            if (ws_parse_handshake_response((char *)ws_ctx.rx_buffer, header_len)) {
                ws_ctx.handshake_complete = true;
                ws_set_state(WS_STATE_CONNECTED);
                ws_ctx.last_pong_received = get_millis();
                printf("[WS] Connected!\n");

                // Remove header from buffer
                if (ws_ctx.rx_len > header_len) {
                    memmove(ws_ctx.rx_buffer, ws_ctx.rx_buffer + header_len,
                            ws_ctx.rx_len - header_len);
                    ws_ctx.rx_len -= header_len;
                } else {
                    ws_ctx.rx_len = 0;
                }
            } else {
                printf("[WS] Handshake failed\n");
                ws_set_state(WS_STATE_ERROR);
                sinricpro_ws_disconnect();
            }
        }
    }

    if (ws_ctx.handshake_complete && ws_ctx.rx_len > 0) {
        ws_process_frame(ws_ctx.rx_buffer, ws_ctx.rx_len);
    }

    return ERR_OK;
}

static void ws_tcp_err(void *arg, err_t err) {
    printf("[WS] TCP error: %d\n", err);
    ws_ctx.pcb = NULL;
    ws_ctx.last_disconnect_time = get_millis();
    ws_set_state(WS_STATE_ERROR);
}

static err_t ws_tcp_sent(void *arg, struct altcp_pcb *pcb, u16_t len) {
    // Data sent successfully
    return ERR_OK;
}

static bool ws_parse_handshake_response(const char *response, size_t len) {
    // Check for "101 Switching Protocols"
    if (!strstr(response, "101")) {
        printf("[WS] Server rejected upgrade: %.100s\n", response);
        return false;
    }

    // Verify Sec-WebSocket-Accept
    char *accept_header = strstr(response, "Sec-WebSocket-Accept:");
    if (!accept_header) {
        printf("[WS] Missing Sec-WebSocket-Accept header\n");
        return false;
    }

    // Calculate expected accept value
    char key_concat[64];
    snprintf(key_concat, sizeof(key_concat), "%s%s", ws_ctx.ws_key, WS_MAGIC_GUID);

    uint8_t sha1_hash[20];
    mbedtls_sha1((unsigned char *)key_concat, strlen(key_concat), sha1_hash);

    char expected_accept[32];
    size_t olen;
    mbedtls_base64_encode((unsigned char *)expected_accept, sizeof(expected_accept),
                          &olen, sha1_hash, 20);

    // Check if server accept matches
    if (!strstr(accept_header, expected_accept)) {
        printf("[WS] Invalid Sec-WebSocket-Accept\n");
        return false;
    }

    return true;
}

static void ws_process_frame(const uint8_t *data, size_t len) {
    size_t offset = 0;

    while (offset < len) {
        if (len - offset < 2) {
            // Need more data
            break;
        }

        // Parse frame header
        uint8_t byte1 = data[offset];
        uint8_t byte2 = data[offset + 1];

        bool fin = (byte1 & 0x80) != 0;
        uint8_t opcode = byte1 & 0x0F;
        bool masked = (byte2 & 0x80) != 0;
        uint64_t payload_len = byte2 & 0x7F;

        size_t header_len = 2;

        // Extended payload length
        if (payload_len == 126) {
            if (len - offset < 4) break;
            payload_len = (data[offset + 2] << 8) | data[offset + 3];
            header_len = 4;
        } else if (payload_len == 127) {
            if (len - offset < 10) break;
            // 64-bit length (not typically needed)
            payload_len = 0;
            for (int i = 0; i < 8; i++) {
                payload_len = (payload_len << 8) | data[offset + 2 + i];
            }
            header_len = 10;
        }

        // Mask key (server frames should not be masked, but handle it)
        uint8_t mask_key[4] = {0};
        if (masked) {
            if (len - offset < header_len + 4) break;
            memcpy(mask_key, &data[offset + header_len], 4);
            header_len += 4;
        }

        // Check if we have full payload
        if (len - offset < header_len + payload_len) {
            break;
        }

        // Get payload
        const uint8_t *payload = &data[offset + header_len];

        // Unmask if needed
        static uint8_t unmasked[WS_RX_BUFFER_SIZE];
        if (masked && payload_len < sizeof(unmasked)) {
            for (size_t i = 0; i < payload_len; i++) {
                unmasked[i] = payload[i] ^ mask_key[i % 4];
            }
            payload = unmasked;
        }

        // Handle frame by opcode
        switch (opcode) {
            case WS_OPCODE_TEXT:
                if (fin && ws_ctx.config.on_message) {
                    // Null-terminate for text
                    if (payload_len < sizeof(unmasked)) {
                        memcpy(unmasked, payload, payload_len);
                        unmasked[payload_len] = '\0';
                        ws_ctx.config.on_message((const char *)unmasked,
                                                 payload_len,
                                                 ws_ctx.config.user_data);
                    }
                }
                break;

            case WS_OPCODE_PING:
                // Send pong
                {
                    uint8_t pong_frame[128];
                    size_t pong_len = ws_encode_frame(WS_OPCODE_PONG,
                                                     payload, payload_len,
                                                     pong_frame, sizeof(pong_frame));
                    if (pong_len > 0 && ws_ctx.pcb) {
                        altcp_write(ws_ctx.pcb, pong_frame, pong_len,
                                   TCP_WRITE_FLAG_COPY);
                        altcp_output(ws_ctx.pcb);
                    }
                }
                break;

            case WS_OPCODE_PONG:
                ws_ctx.ping_pending = false;
                ws_ctx.last_pong_received = get_millis();
                break;

            case WS_OPCODE_CLOSE:
                printf("[WS] Server sent close frame\n");
                sinricpro_ws_disconnect();
                return;

            default:
                break;
        }

        offset += header_len + payload_len;
    }

    // Keep remaining data
    if (offset > 0 && offset < len) {
        memmove(ws_ctx.rx_buffer, ws_ctx.rx_buffer + offset, len - offset);
        ws_ctx.rx_len = len - offset;
    } else if (offset >= len) {
        ws_ctx.rx_len = 0;
    }
}

static size_t ws_encode_frame(uint8_t opcode, const uint8_t *data, size_t len,
                              uint8_t *output, size_t output_len) {
    // Calculate required size
    size_t header_len = 2;
    if (len >= 126 && len <= 65535) {
        header_len = 4;
    } else if (len > 65535) {
        header_len = 10;
    }

    // Client frames must be masked
    size_t total_len = header_len + 4 + len;  // +4 for mask key

    if (total_len > output_len) {
        return 0;
    }

    size_t offset = 0;

    // FIN bit + opcode
    output[offset++] = 0x80 | (opcode & 0x0F);

    // Mask bit + payload length
    if (len < 126) {
        output[offset++] = 0x80 | len;
    } else if (len <= 65535) {
        output[offset++] = 0x80 | 126;
        output[offset++] = (len >> 8) & 0xFF;
        output[offset++] = len & 0xFF;
    } else {
        output[offset++] = 0x80 | 127;
        for (int i = 7; i >= 0; i--) {
            output[offset++] = (len >> (i * 8)) & 0xFF;
        }
    }

    // Generate mask key
    uint8_t mask_key[4];
    uint32_t rand_val = get_rand_32();
    mask_key[0] = (rand_val >> 24) & 0xFF;
    mask_key[1] = (rand_val >> 16) & 0xFF;
    mask_key[2] = (rand_val >> 8) & 0xFF;
    mask_key[3] = rand_val & 0xFF;

    memcpy(&output[offset], mask_key, 4);
    offset += 4;

    // Masked payload
    if (data && len > 0) {
        for (size_t i = 0; i < len; i++) {
            output[offset + i] = data[i] ^ mask_key[i % 4];
        }
    }

    return offset + len;
}
