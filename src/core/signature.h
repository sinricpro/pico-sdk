/**
 * @file signature.h
 * @brief HMAC-SHA256 signature generation and validation for SinricPro messages
 *
 * This module provides cryptographic signing functionality compatible with
 * the SinricPro protocol, using mbedTLS for HMAC-SHA256 computation.
 */

#ifndef SINRICPRO_SIGNATURE_H
#define SINRICPRO_SIGNATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Maximum size for Base64-encoded HMAC signature
 *
 * SHA256 produces 32 bytes, Base64 encoding produces ceil(32/3)*4 = 44 chars + null
 */
#define SINRICPRO_SIGNATURE_MAX_LEN 48

/**
 * @brief Compute HMAC-SHA256 and return Base64-encoded result
 *
 * @param message   The message to sign (null-terminated string)
 * @param key       The secret key (null-terminated string)
 * @param output    Buffer to store Base64-encoded signature
 * @param output_len Size of output buffer (must be >= SINRICPRO_SIGNATURE_MAX_LEN)
 * @return true on success, false on failure
 */
bool sinricpro_hmac_base64(const char *message, const char *key,
                           char *output, size_t output_len);

/**
 * @brief Extract payload JSON from a complete SinricPro message
 *
 * Extracts the content between "payload": and ,"signature" from the message.
 * The caller is responsible for freeing the returned string.
 *
 * @param message   The complete JSON message string
 * @param output    Buffer to store extracted payload
 * @param output_len Size of output buffer
 * @return Length of extracted payload, or 0 on failure
 */
size_t sinricpro_extract_payload(const char *message, char *output, size_t output_len);

/**
 * @brief Calculate signature for a payload string
 *
 * @param key       The secret key (app_secret)
 * @param payload   The serialized payload JSON string
 * @param output    Buffer to store Base64-encoded signature
 * @param output_len Size of output buffer
 * @return true on success, false on failure
 */
bool sinricpro_calculate_signature(const char *key, const char *payload,
                                   char *output, size_t output_len);

/**
 * @brief Verify signature of an incoming message
 *
 * @param key       The secret key (app_secret)
 * @param message   The complete JSON message with signature
 * @param signature The expected signature to verify against
 * @return true if signature matches, false otherwise
 */
bool sinricpro_verify_signature(const char *key, const char *message,
                                const char *signature);

/**
 * @brief Base64 encode a byte array
 *
 * @param input     Input byte array
 * @param input_len Length of input array
 * @param output    Buffer for Base64 output (null-terminated)
 * @param output_len Size of output buffer
 * @return Length of encoded string, or 0 on failure
 */
size_t sinricpro_base64_encode(const uint8_t *input, size_t input_len,
                               char *output, size_t output_len);

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_SIGNATURE_H
