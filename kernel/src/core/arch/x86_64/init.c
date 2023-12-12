#include "init.h"

#include <lib/utils.h>

#include "idt.h"

void x86_64_init()
{
    idt_init();
}