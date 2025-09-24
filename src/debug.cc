#include "debug.h"
#include "Arduino.h"

#ifdef DEBUG_TO_SERIAL

bool _is_debug_enabled = true;

void disable_debug() {
    _is_debug_enabled = false;
}

bool is_debug_enabled() {
    return _is_debug_enabled;
}

#endif
