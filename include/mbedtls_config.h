/**
 * @file mbedtls_config.h
 * @brief mbedTLS configuration for SinricPro Pico SDK
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

// Workaround for mbedTLS 3.x with old pico-sdk code
#include <limits.h>
#define MBEDTLS_ALLOW_PRIVATE_ACCESS

// Platform
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

// Crypto primitives needed by SinricPro
#define MBEDTLS_MD_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_BASE64_C

// Required for TLS/SSL support (needed by pico-sdk's altcp_tls)
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2

// X.509 certificate support (required by altcp_tls_mbedtls.c)
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_OID_C

// Public key cryptography (required for certificates)
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_RSA_C

// Cipher suites
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_GCM_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CTR

// Elliptic curve crypto
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED

// Key exchange
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED

// Random number generation
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT

// Memory and debugging
#define MBEDTLS_SSL_MAX_CONTENT_LEN 16384

// Additional SSL settings
#define MBEDTLS_SSL_IN_CONTENT_LEN 16384
#define MBEDTLS_SSL_OUT_CONTENT_LEN 4096

// Timing functions
#define MBEDTLS_HAVE_TIME
#define MBEDTLS_PLATFORM_MS_TIME_ALT

#include <stdio.h>
#include <stdlib.h>

#endif /* MBEDTLS_CONFIG_H */
