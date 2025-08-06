#include "../include/malloc.h"

t_heap g_heap = {NULL, NULL, NULL, PTHREAD_MUTEX_INITIALIZER};

void *malloc(size_t size)
{
    void *ptr;

    if (size == 0)
        return NULL;

    size = ALIGN(size);

    pthread_mutex_lock(&g_heap.mutex);
    
    if (size <= TINY_MAX)
        ptr = allocate_from_zone(&g_heap.tiny, size, TINY_ZONE_SIZE);
    else if (size <= SMALL_MAX)
        ptr = allocate_from_zone(&g_heap.small, size, SMALL_ZONE_SIZE);
    else
        ptr = allocate_large(size);

    pthread_mutex_unlock(&g_heap.mutex);
    
    return ptr;
}