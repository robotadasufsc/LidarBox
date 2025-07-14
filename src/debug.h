#pragma once

#ifdef DEBUG_TO_SERIAL

#define DEBUG_STREAM Serial

#define DEBUG(message) { if(DEBUG_STREAM) DEBUG_STREAM.print(message); }
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
#define DEBUGLN(message) { if(DEBUG_STREAM) DEBUG_STREAM.println(message); }

#else

#define DEBUG(message) {}
#define DEBUGHEX(code, size) {}
#define DEBUGLN(message) {}

#endif
