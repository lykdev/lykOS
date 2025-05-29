#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Hello world!");

    uint32_t *p = malloc(1600);

    for (size_t i = 0; i < 25; i++)
        p[i] = 0xFF0000;
    for (size_t i = 25; i < 50; i++)
        p[i] = 0x00FF00;
    for (size_t i = 50; i < 75; i++)
        p[i] = 0x0000FF;
    for (size_t i = 75; i < 100; i++)
        p[i] = 0xFFFFFF;

    FILE *file = fopen("/dev/fb", "r+");
    fwrite(p, 4, 400, file);

    return 43;
}
