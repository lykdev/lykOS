#pragma once

#include <utils/def.h>

// Copying

void* memcpy(void *dest, const void *src, size_t n);

void* memmove(void *dest, const void *src, size_t n);

void strcpy(char *dest, const char *src);

// Concatenation

char* strcat(char* dest, const char* src);

// Searching

char* strchr(const char *s, char c);

// Comparison

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, size_t n);

// Other

void *memset(void *s, int c, size_t n);

size_t strlen(const char *s);
