#pragma once

#include <lib/def.h>

int strcmp(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, size_t n);

char *strchr(const char *s, char c);

void strcpy(char *dest, const char *src);

size_t strlen(const char *s);

int strmatch(const char *a, const char *b);
