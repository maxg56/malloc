#include "../include/malloc.h"

t_heap g_heap = {
    .tiny = NULL,
    .small = NULL, 
    .large = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .debug = {false, false, false, false, 0},
    .history_buffer = {{0}},  // Buffer statique initialisé
    .history_count = 0
};

void *malloc(size_t size)
{
    void *ptr;
    static bool debug_initialized = false;

    if (size == 0)
        return NULL;

    size = ALIGN(size);

    pthread_mutex_lock(&g_heap.mutex);
    
    /* Initialize debug flags once */
    if (!debug_initialized)
    {
        init_debug_flags();
        debug_initialized = true;
    }
    
    if (size <= TINY_MAX)
        ptr = allocate_from_zone(&g_heap.tiny, size, TINY_ZONE_SIZE);
    else if (size <= SMALL_MAX)
        ptr = allocate_from_zone(&g_heap.small, size, SMALL_ZONE_SIZE);
    else
        ptr = allocate_large(size);

    if (ptr)
    {
        /* Pre-scribble allocated memory */
        if (g_heap.debug.pre_scribble)
            scribble_memory(ptr, size, MALLOC_SCRIBBLE_ALLOC);
        
        /* Add to allocation history */
        add_to_history(ptr, size, true);
    }

    pthread_mutex_unlock(&g_heap.mutex);
    
    return ptr;
}