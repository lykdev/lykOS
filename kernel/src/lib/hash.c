#include "hash.h"

uint hash_str(const char *str)
{
    uint hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
