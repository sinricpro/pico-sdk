/**
 * @file signature.c
 * @brief HMAC-SHA256 signature implementation for SinricPro
 *
 * Uses mbedTLS (built into pico-sdk) for cryptographic operations.
 */

#include "signature.h"
#include <string.h>
#include <stdio.h>

#include "mbedtls/md.h"
#include "mbedtls/base64.h"

// SHA256 digest size in bytes
#define SHA256_DIGEST_SIZE 32

bool sinricpro_hmac_base64(const char *message, const char *key,
                           char *output, size_t output_len) {
    if (!message || !key || !output || output_len < SINRICPRO_SIGNATURE_MAX_LEN) {
        return false;
    }

    uint8_t hmac_result[SHA256_DIGEST_SIZE];

    // Initialize mbedTLS HMAC context
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        mbedtls_md_free(&ctx);
        return false;
    }

    int ret = mbedtls_md_setup(&ctx, md_info, 1); // 1 = use HMAC
    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }

    // Compute HMAC-SHA256
    ret = mbedtls_md_hmac_starts(&ctx, (const unsigned char *)key, strlen(key));
    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }

    ret = mbedtls_md_hmac_update(&ctx, (const unsigned char *)message, strlen(message));
    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }

    ret = mbedtls_md_hmac_finish(&ctx, hmac_result);
    mbedtls_md_free(&ctx);

    if (ret != 0) {
        return false;
    }

    // Base64 encode the result
    size_t encoded_len = sinricpro_base64_encode(hmac_result, SHA256_DIGEST_SIZE,
                                                  output, output_len);

    return encoded_len > 0;
}

size_t sinricpro_base64_encode(const uint8_t *input, size_t input_len,
                               char *output, size_t output_len) {
    if (!input || !output || output_len == 0) {
        return 0;
    }

    size_t written = 0;
    int ret = mbedtls_base64_encode((unsigned char *)output, output_len,
                                    &written, input, input_len);

    if (ret != 0) {
        return 0;
    }

    // Ensure null-termination
    if (written < output_len) {
        output[written] = '\0';
    }

    return written;
}

size_t sinricpro_extract_payload(const char *message, char *output, size_t output_len) {
    if (!message || !output || output_len == 0) {
        return 0;
    }

    // Find "payload":
    const char *payload_key = "\"payload\":";
    const char *begin = strstr(message, payload_key);
    if (!begin) {
        return 0;
    }

    // Move past "payload":
    begin += strlen(payload_key);

    // Find ,"signature" which marks the end of payload
    const char *sig_key = ",\"signature\"";
    const char *end = strstr(begin, sig_key);
    if (!end) {
        return 0;
    }

    // Calculate payload length
    size_t payload_len = end - begin;

    // Check if output buffer is large enough
    if (payload_len >= output_len) {
        return 0;
    }

    // Copy payload
    memcpy(output, begin, payload_len);
    output[payload_len] = '\0';

    return payload_len;
}

bool sinricpro_calculate_signature(const char *key, const char *payload,
                                   char *output, size_t output_len) {
    if (!key || !payload || !output) {
        return false;
    }

    if (strlen(payload) == 0) {
        return false;
    }

    return sinricpro_hmac_base64(payload, key, output, output_len);
}

bool sinricpro_verify_signature(const char *key, const char *message,
                                const char *signature) {
    if (!key || !message || !signature) {
        return false;
    }

    // Extract payload from message
    char payload[2048];
    size_t payload_len = sinricpro_extract_payload(message, payload, sizeof(payload));

    if (payload_len == 0) {
        return false;
    }

    // Calculate expected signature
    char calculated_sig[SINRICPRO_SIGNATURE_MAX_LEN];
    if (!sinricpro_calculate_signature(key, payload, calculated_sig, sizeof(calculated_sig))) {
        return false;
    }

    // Compare signatures (constant-time comparison for security)
    size_t sig_len = strlen(signature);
    size_t calc_len = strlen(calculated_sig);

    if (sig_len != calc_len) {
        return false;
    }

    // Constant-time comparison to prevent timing attacks
    uint8_t result = 0;
    for (size_t i = 0; i < sig_len; i++) {
        result |= signature[i] ^ calculated_sig[i];
    }

    return result == 0;
}
