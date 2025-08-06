#include "../include/malloc.h"

void *ft_memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;

    while (n--)
        *d++ = *s++;

    return dest;
}