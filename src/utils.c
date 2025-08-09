#include "../include/malloc.h"
#include <string.h>
#include <stdint.h>

void *ft_memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;

    while (n--)
        *d++ = *s++;

    return dest;
}

static void putstr(const char *s)
{
    write(1, s, strlen(s));
}

static void putnbr_size(size_t n)
{
    char buf[32];
    int i = 31; buf[i--] = '\0';
    if (n == 0) { buf[i] = '0'; write(1, &buf[i], 1); return; }
    while (n && i >= 0)
    {
        buf[i--] = (char)('0' + (n % 10));
        n /= 10;
    }
    write(1, &buf[i + 1], 31 - i);
}

void show_alloc_mem(void)
{
    size_t total = 0;
    pthread_mutex_lock(&g_heap.mutex);

    // helper for hex addresses
    auto void write_hex_addr(void *p) {
        const char *hex = "0123456789ABCDEF";
        uintptr_t v = (uintptr_t)p;
        size_t n = sizeof(uintptr_t) * 2;
        char buf[2 + sizeof(uintptr_t) * 2];
        buf[0] = '0'; buf[1] = 'x';
        for (size_t i = 0; i < n; ++i) {
            unsigned shift = (unsigned)((n - 1 - i) * 4);
            buf[2 + i] = hex[(v >> shift) & 0xF];
        }
        write(1, buf, 2 + n);
    }

    // TINY zones
    putstr("TINY : ");
    if (g_heap.tiny) write_hex_addr(g_heap.tiny);
    write(1, "\n", 1);
    for (t_zone *z = g_heap.tiny; z; z = z->next)
    {
        for (t_block *b = z->blocks; b; b = b->next)
        {
            if (!b->is_free)
            {
                void *start = (void *)((char *)b + sizeof(t_block));
                void *end = (void *)((char *)start + b->size);
                write_hex_addr(start);
                putstr(" - ");
                write_hex_addr(end);
                putstr(" : ");
                putnbr_size(b->size); putstr(" bytes\n");
                total += b->size;
            }
        }
    }

    // SMALL zones
    putstr("SMALL : ");
    if (g_heap.small) write_hex_addr(g_heap.small);
    write(1, "\n", 1);
    for (t_zone *z = g_heap.small; z; z = z->next)
    {
        for (t_block *b = z->blocks; b; b = b->next)
        {
            if (!b->is_free)
            {
                void *start = (void *)((char *)b + sizeof(t_block));
                void *end = (void *)((char *)start + b->size);
                write_hex_addr(start);
                putstr(" - ");
                write_hex_addr(end);
                putstr(" : ");
                putnbr_size(b->size); putstr(" bytes\n");
                total += b->size;
            }
        }
    }

    // LARGE list
    putstr("LARGE : ");
    if (g_heap.large) write_hex_addr(g_heap.large);
    write(1, "\n", 1);
    for (t_block *b = g_heap.large; b; b = b->next)
    {
        void *start = (void *)((char *)b + sizeof(t_block));
        void *end = (void *)((char *)start + b->size);
        write_hex_addr(start);
        putstr(" - ");
        write_hex_addr(end);
        putstr(" : ");
        putnbr_size(b->size); putstr(" bytes\n");
        total += b->size;
    }

    putstr("Total : ");
    putnbr_size(total);
    putstr(" bytes\n");

    pthread_mutex_unlock(&g_heap.mutex);
}

void show_alloc_mem_ex(void)
{
    // Basic stub that calls the base version for now
    show_alloc_mem();
}