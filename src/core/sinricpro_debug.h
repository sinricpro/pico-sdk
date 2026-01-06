/**
 * @file sinricpro_debug.h
 * @brief Debug logging utilities for SinricPro SDK
 *
 * Provides debug logging macros that respect the enable_debug configuration.
 */

#ifndef SINRICPRO_DEBUG_H
#define SINRICPRO_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Set debug mode
 *
 * @param enabled true to enable debug logging, false to disable
 */
void sinricpro_debug_set_enabled(bool enabled);

/**
 * @brief Check if debug mode is enabled
 *
 * @return true if debug enabled, false otherwise
 */
bool sinricpro_debug_is_enabled(void);

/**
 * @brief Debug printf - only prints if debug is enabled
 */
#define SINRICPRO_DEBUG_PRINTF(...) \
    do { \
        if (sinricpro_debug_is_enabled()) { \
            printf(__VA_ARGS__); \
        } \
    } while (0)

/**
 * @brief Error printf - always prints regardless of debug setting
 */
#define SINRICPRO_ERROR_PRINTF(...) \
    do { \
        printf(__VA_ARGS__); \
    } while (0)

/**
 * @brief Warning printf - always prints regardless of debug setting
 */
#define SINRICPRO_WARN_PRINTF(...) \
    do { \
        printf(__VA_ARGS__); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // SINRICPRO_DEBUG_H
