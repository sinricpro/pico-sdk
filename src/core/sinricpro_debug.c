/**
 * @file sinricpro_debug.c
 * @brief Debug logging implementation
 */

#include "sinricpro_debug.h"

static bool debug_enabled = false;

void sinricpro_debug_set_enabled(bool enabled) {
    debug_enabled = enabled;
}

bool sinricpro_debug_is_enabled(void) {
    return debug_enabled;
}
