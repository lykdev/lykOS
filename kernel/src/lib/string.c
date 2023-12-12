#include "string.h"

char *itoa(int value, char *str, int base)
{
    char *rc;
    char *ptr;
    char *low;
    // Check for supported base.
    if (base < 2 || base > 36)
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if (value < 0 && base == 10)
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while (low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

char *ulltoa(unsigned long long int value, char *str, int base)
{
    char *rc;
    char *ptr;
    char *low;
    // Check for supported base.
    if (base < 2 || base > 36)
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while (low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

size_t strlen(const char *s)
{
    size_t count = 0;
    while(*s!='\0')
    {
        count++;
        s++;
    }
    return count;
}

char* strcpy(char* destination, const char* source)
{ 
    char *ptr = destination;
 
    while ((*destination++ = *source++) != '\0');

    return ptr;
}

char* strcat(char* destination, const char* source)
{
    char* ptr = destination + strlen(destination);
 
    while (*source != '\0') {
        *ptr++ = *source++;
    }
 
    *ptr = '\0';
 
    return destination;
}

char* strncat(char* destination, const char* source, size_t num)
{
    // make `ptr` point to the end of the destination string
    char* ptr = destination + strlen(destination);
 
    // Appends characters of the source to the destination string
    while (*source != '\0' && num--) {
        *ptr++ = *source++;
    }
 
    // null terminate destination string
    *ptr = '\0';
 
    // destination string is returned by standard `strncat()`
    return destination;
}

int vsprintf(char *str, const char *format, va_list list)
{
    int i;
    for (i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            char buf[128];
            i++;
            if (format[i] == 's')
                strcat(&str[0], va_arg(list, const char *));
            else if (format[i] == 'd')
                strcat(&str[0], itoa(va_arg(list, int), buf, 10));
            else if (format[i] == 'x')
                strcat(&str[0], itoa(va_arg(list, uint64_t), buf, 16));
            else if (format[i] == 'b')
                strcat(&str[0], itoa(va_arg(list, int), buf, 2));
            else if (format[i] == 'c')
            {
                char c = va_arg(list, int);
                strncat(&str[0], &c, 1);
            }                
            else if (format[i] == 'l')
            {
                i++;
                if (format[i] == 'l')
                {
                    i++;
                    if (format[i] == 'u')
                        strcat(&str[0], ulltoa(va_arg(list, uint64_t), buf, 10));
                    else if (format[i] == 'x')
                        strcat(&str[0], ulltoa(va_arg(list, uint64_t), buf, 16));
                }
            }
        }
        else
            strncat(&str[0], &format[i], 1);
    }

    return strlen(str);
}