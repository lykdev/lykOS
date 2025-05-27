 #pragma once

#include <lib/def.h>

typedef struct
{
    u64  timestamp;
    u16  keycode;
    bool pressed;
}
__attribute__((packed))
kb_event_t;

void ps2kb_setup();
