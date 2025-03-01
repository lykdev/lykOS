#include "path.h"

char *path_consume_comp(const char *str, char *out_comp)
{
  if (*str == '/')
    str++;

  while (*str != '\0' && *str != '/')
    *out_comp++ = *str++;
  *out_comp = '\0';

  if (*str == '/')
    str++;
  return (char *)str;
}

void path_normalize(const char *path, char *normalized)
{
  const char *p = path;
  char *n = normalized;

  while (*p)
  {
    if (*p == '/')
    {
      // Skip redundant '/'.
      *n++ = '/';
      while (*p == '/')
        p++;
    } else if (*p == '.' && (*(p + 1) == '/' || *(p + 1) == '\0'))
    {
      // Skip './'.
      p++;
      if (*p == '/')
        p++;
    } else
      // Copy valid character.
      *n++ = *p++;
  }

  // Remove trailing '/' unless it's the root.
  if (n > normalized + 1 && *(n - 1) == '/')
    n--;

  *n = '\0';
}
