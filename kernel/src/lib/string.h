#pragma once

#include <lib/def.h>

// Copying

void *memcpy(void *dest, const void *src, size_t n);

void *memmove(void *dest, const void *src, size_t n);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, size_t n);

// Concatenation

char *strcat(char *dest, const char *src);

// Searching

char *strchr(const char *s, char c);

char *strrchr(const char *s, int c);

char *strstr(const char *s1, const char *s2);

// Comparison

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, size_t n);

// Other

void *memset(void *s, int c, size_t n);

size_t strlen(const char *s);
