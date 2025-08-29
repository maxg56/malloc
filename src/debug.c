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

    /* History buffer is statically allocated, no need to initialize */
}

void add_to_history(void *ptr, size_t size, bool is_alloc)
{
    t_alloc_history *new_entry;

    // Ne pas allouer d'historique si pas nécessaire pour économiser des pages
    if (!g_heap.debug.stack_logging || g_heap.history_count >= MAX_ALLOC_HISTORY)
        return;

    // Utiliser le buffer statique au lieu de mmap pour économiser des pages
    size_t index = g_heap.history_count % MAX_ALLOC_HISTORY;
    new_entry = &g_heap.history_buffer[index];

    new_entry->ptr = ptr;
    new_entry->size = size;
    new_entry->timestamp = time(NULL);
    new_entry->is_freed = !is_alloc;
    new_entry->next = NULL;  // Plus besoin de linked list

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