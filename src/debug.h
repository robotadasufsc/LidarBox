#pragma once

#ifdef DEBUG_TO_SERIAL

#define DEBUG_STREAM Serial

void disable_debug();
bool is_debug_enabled();

#define DEBUG(message) {\
    if(is_debug_enabled()) Serial.print(message);\
}

#define DEBUGHEX(code, size) {\
    const unsigned int s = size;\
    if(s > 0 && DEBUG_STREAM) {\
        DEBUG_STREAM.print(code[0], HEX);\
        for(unsigned int i = 1; i < s; i++ ) {\
            DEBUG(' ');\
            if(code[i] < 16) DEBUG('0'); \
            DEBUG_STREAM.print(code[i], HEX);\
        }\
    }\
}

#define DEBUGLN(message) {\
    if(is_debug_enabled()) Serial.println(message);\
}

#else

#define DEBUG(message) {}
#define DEBUGHEX(code, size) {}
#define DEBUGLN(message) {}

#endif
