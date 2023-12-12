#pragma once

#include "utils.h"

char *itoa(int value, char *str, int base);

char *ulltoa(unsigned long long int value, char *str, int base);

size_t strlen(const char *s);

char *strcpy(char *destination, const char *source);

char *strcat(char *destination, const char *source);

char *strncat(char *destination, const char *source, size_t num);

// Minimalist implementation.
int vsprintf(char *str, const char *format, va_list list);