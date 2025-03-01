#include "string.h"

// Copying

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
        pdest[i] = psrc[i];

    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
        for (size_t i = 0; i < n; i++)
            pdest[i] = psrc[i];
    else if (src < dest)
        for (size_t i = n; i > 0; i--)
            pdest[i - 1] = psrc[i - 1];

    return dest;
}

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;

    while ((*dest++ = *src++))
        ;

    return ret;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;

    while (n && (*dest++ = *src++) != '\0')
        n--;
    while (n--)
        *dest++ = '\0';

    return ret;
}

// Concatenation

char *strcat(char *dest, const char *src)
{
    char *ret = dest;

    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;

    return ret;
}

// Comparison

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
        if (p1[i] != p2[i])
            return p1[i] < p2[i] ? -1 : 1;

    return 0;
}

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

// Searching

char *strchr(const char *s, char c)
{
    while (*s)
    {
        if (*s == c)
            return (char *)s;
        s++;
    }

    return NULL;
}

char *strrchr(const char *s, int c)
{
    char *ret = NULL;

    while (*s)
    {
        if (*s == c)
            ret = (char *)s;
        s++;
    }

    return ret;
}

// Other

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
        p[i] = (uint8_t)c;

    return s;
}

size_t strlen(const char *s)
{
    size_t len = 0;

    while (*s++)
        len++;

    return len;
}
