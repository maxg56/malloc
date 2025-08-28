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

static void write_hex_addr(void *p)
{
    const char *hex = "0123456789ABCDEF";
    uintptr_t v = (uintptr_t)p;
    size_t n = sizeof(uintptr_t) * 2;
    char buf[2 + sizeof(uintptr_t) * 2];
    
    buf[0] = '0'; 
    buf[1] = 'x';
    for (size_t i = 0; i < n; ++i) {
        unsigned shift = (unsigned)((n - 1 - i) * 4);
        buf[2 + i] = hex[(v >> shift) & 0xF];
    }
    write(1, buf, 2 + n);
}

void show_alloc_mem(void)
{
    size_t total = 0;
    pthread_mutex_lock(&g_heap.mutex);

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

static void print_hex_dump(void *ptr, size_t size)
{
    unsigned char *data = (unsigned char *)ptr;
    size_t i, j;
    char ascii[17];
    
    if (!ptr || size == 0)
        return;
    
    /* Limit dump to first 64 bytes for readability */
    if (size > 64)
        size = 64;
    
    for (i = 0; i < size; i += 16)
    {
        write_hex_addr((void *)((uintptr_t)data + i));
        putstr("  ");
        
        /* Print hex bytes */
        for (j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                unsigned char byte = data[i + j];
                const char *hex = "0123456789ABCDEF";
                char hex_buf[3];
                hex_buf[0] = hex[byte >> 4];
                hex_buf[1] = hex[byte & 0xF];
                hex_buf[2] = ' ';
                write(1, hex_buf, 3);
                ascii[j] = (byte >= 32 && byte <= 126) ? byte : '.';
            }
            else
            {
                putstr("   ");
                ascii[j] = ' ';
            }
        }
        
        /* Print ASCII representation */
        ascii[16] = '\0';
        putstr(" |");
        putstr(ascii);
        putstr("|\n");
    }
    
    if (size == 64)
        putstr("...\n");
}

static void show_allocation_history(void)
{
    t_alloc_history *entry;
    int count = 0;
    
    if (!g_heap.debug.stack_logging || !g_heap.history)
    {
        putstr("Allocation history: disabled (set MALLOC_STACK_LOGGING=1)\n");
        return;
    }
    
    putstr("=== Allocation History ===\n");
    for (entry = g_heap.history; entry && count < 20; entry = entry->next, count++)
    {
        if (entry->is_freed)
            putstr("FREE  ");
        else
            putstr("ALLOC ");
        
        write_hex_addr(entry->ptr);
        putstr(" size: ");
        putnbr_size(entry->size);
        putstr(" time: ");
        putnbr_size((size_t)entry->timestamp);
        putstr("\n");
    }
    
    if (g_heap.history_count > 20)
    {
        putstr("... (showing latest 20 of ");
        putnbr_size(g_heap.history_count);
        putstr(" total entries)\n");
    }
    putstr("\n");
}

void show_alloc_mem_ex(void)
{
    size_t total = 0;
    pthread_mutex_lock(&g_heap.mutex);

    /* Show debug settings */
    putstr("=== Debug Configuration ===\n");
    putstr("MALLOC_SCRIBBLE: ");
    putstr(g_heap.debug.scribble ? "ON" : "OFF");
    putstr("\nMALLOC_PRE_SCRIBBLE: ");
    putstr(g_heap.debug.pre_scribble ? "ON" : "OFF");
    putstr("\nMALLOC_GUARD: ");
    putstr(g_heap.debug.guard ? "ON" : "OFF");
    putstr("\nMALLOC_STACK_LOGGING: ");
    putstr(g_heap.debug.stack_logging ? "ON" : "OFF");
    putstr("\nMALLOC_CHECK_: ");
    putnbr_size((size_t)g_heap.debug.check_level);
    putstr("\n\n");

    /* Show allocation history */
    show_allocation_history();

    /* Show memory zones with hex dumps */
    putstr("=== Memory Zones ===\n");
    
    // TINY zones
    putstr("TINY : ");
    if (g_heap.tiny) write_hex_addr(g_heap.tiny);
    write(1, "\n", 1);
    for (t_zone *z = g_heap.tiny; z; z = z->next)
    {
        putstr("Zone ");
        write_hex_addr(z);
        putstr(" (");
        putnbr_size(z->size);
        putstr(" bytes)\n");
        
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
                putnbr_size(b->size);
                putstr(" bytes (allocated at ");
                putnbr_size((size_t)b->alloc_time);
                putstr(")\n");
                
                /* Show hex dump */
                print_hex_dump(start, b->size);
                putstr("\n");
                
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
        putstr("Zone ");
        write_hex_addr(z);
        putstr(" (");
        putnbr_size(z->size);
        putstr(" bytes)\n");
        
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
                putnbr_size(b->size);
                putstr(" bytes (allocated at ");
                putnbr_size((size_t)b->alloc_time);
                putstr(")\n");
                
                /* Show hex dump */
                print_hex_dump(start, b->size);
                putstr("\n");
                
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
        putnbr_size(b->size);
        putstr(" bytes (allocated at ");
        putnbr_size((size_t)b->alloc_time);
        putstr(")\n");
        
        /* Show hex dump */
        print_hex_dump(start, b->size);
        putstr("\n");
        
        total += b->size;
    }

    putstr("Total : ");
    putnbr_size(total);
    putstr(" bytes\n");

    pthread_mutex_unlock(&g_heap.mutex);
}