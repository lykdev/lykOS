#include "str.h"

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
        s1++, s2++;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
        s1++, s2++, n--;

    if (n == 0)
        return 0;
    else
        return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strchr(const char *s, char c)
{
    while (*s)
    {
        if (*s == c)
            return (char*)s;
        s++;
    }

    return NULL;
}

void strcpy(char *dest, const char *src)
{
    while ((*dest++ = *src++));
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++)
        len++;
    return len;
}

int strmatch(const char *a, const char *b)
{
    int count = 0;
    while (*a && *b && *a == *b)
        count++, a++, b++;
    return count;
}
