#include "init.h"

#include <lib/utils.h>

#include "exc.h"

void aarch64_init()
{
    exc_enable();
}