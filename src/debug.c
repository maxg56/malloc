#include "../include/malloc.h"
#include <string.h>

void init_debug_flags(void)
{
    char *env;

    /* Initialize debug flags from environment variables */
    g_heap.debug.scribble = getenv("MALLOC_SCRIBBLE") != NULL;
    g_heap.debug.pre_scribble = getenv("MALLOC_PRE_SCRIBBLE") != NULL;
    g_heap.debug.guard = getenv("MALLOC_GUARD") != NULL;
    g_heap.debug.stack_logging = getenv("MALLOC_STACK_LOGGING") != NULL;

    env = getenv("MALLOC_CHECK_");
    if (env)
        g_heap.debug.check_level = atoi(env);
    else
        g_heap.debug.check_level = 0;

    /* Initialize history */
    g_heap.history = NULL;
    g_heap.history_count = 0;
}

void add_to_history(void *ptr, size_t size, bool is_alloc)
{
    t_alloc_history *new_entry;

    if (!g_heap.debug.stack_logging || g_heap.history_count >= MAX_ALLOC_HISTORY)
        return;

    new_entry = mmap(NULL, sizeof(t_alloc_history), PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_entry == MAP_FAILED)
        return;

    new_entry->ptr = ptr;
    new_entry->size = size;
    new_entry->timestamp = time(NULL);
    new_entry->is_freed = !is_alloc;
    new_entry->next = g_heap.history;

    g_heap.history = new_entry;
    g_heap.history_count++;
}

void scribble_memory(void *ptr, size_t size, unsigned char pattern)
{
    if (!ptr || size == 0)
        return;
    
    memset(ptr, pattern, size);
}

void check_guards(void *ptr)
{
    /* Basic guard checking - would need more sophisticated implementation */
    if (!ptr || !g_heap.debug.guard)
        return;
    
    /* This is a simplified version - real implementation would check 
       guard bytes around allocations */
    (void)ptr; /* Suppress unused parameter warning */
}

void defragment_zones(void)
{
    t_zone *zone;
    t_block *current, *next;

    /* Defragment TINY zones */
    for (zone = g_heap.tiny; zone; zone = zone->next)
    {
        current = zone->blocks;
        while (current && current->next)
        {
            if (current->is_free && current->next->is_free)
            {
                /* Merge adjacent free blocks */
                current->size += current->next->size + sizeof(t_block);
                next = current->next->next;
                current->next = next;
                /* Continue checking from current block */
                continue;
            }
            current = current->next;
        }
    }

    /* Defragment SMALL zones */
    for (zone = g_heap.small; zone; zone = zone->next)
    {
        current = zone->blocks;
        while (current && current->next)
        {
            if (current->is_free && current->next->is_free)
            {
                /* Merge adjacent free blocks */
                current->size += current->next->size + sizeof(t_block);
                next = current->next->next;
                current->next = next;
                continue;
            }
            current = current->next;
        }
    }
}

void malloc_defragment(void)
{
    pthread_mutex_lock(&g_heap.mutex);
    defragment_zones();
    pthread_mutex_unlock(&g_heap.mutex);
}