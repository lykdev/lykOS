#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Invalid arguments. Usage: ksym <input file> <output file>\n");
        return EXIT_FAILURE;
    }

    FILE *in  = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    uint64_t addr;
    char type; // Discarded.
    char name[256];
    while (fscanf(in, "%llx %c %s", &addr, &type, &name) == 3)
    {
        fwrite(&addr, sizeof(addr), 1, out);
        fwrite(&name, 1, strlen(name) + 1, out);
    }

    fclose(in);
    fclose(out);

    return 0;
}
