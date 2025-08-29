#include "../include/malloc.h"

static bool is_large_block(t_block *block)
{
    t_block *cur = g_heap.large;
    while (cur) {
        if (cur == block)
            return true;
        cur = cur->next;
    }
    return false;
}

void *realloc(void *ptr, size_t size)
{
    t_block *block;
    void *new_ptr;

    if (!ptr)
        return malloc(size);

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    size = ALIGN(size);
    block = (t_block *)((char *)ptr - sizeof(t_block));
    
    if (block->size >= size)
        return ptr;

    pthread_mutex_lock(&g_heap.mutex);
    
    // Optimisation pour LARGE blocks : utiliser mremap() si possible
    if (is_large_block(block) && size > SMALL_MAX)
    {
        size_t old_total = sizeof(t_block) + block->size;
        size_t new_total = sizeof(t_block) + size;
        
        void *new_block = mremap(block, old_total, new_total, MREMAP_MAYMOVE);
        if (new_block != MAP_FAILED)
        {
            if (new_block != block)
            {
                t_block *prev = NULL;
                t_block *cur = g_heap.large;
                while (cur && cur != block)
                {
                    prev = cur;
                    cur = cur->next;
                }
                if (cur)
                {
                    if (prev)
                        prev->next = (t_block *)new_block;
                    else
                        g_heap.large = (t_block *)new_block;
                    
                    ((t_block *)new_block)->next = cur->next;
                }
            }
            
            ((t_block *)new_block)->size = size;
            pthread_mutex_unlock(&g_heap.mutex);
            return (void *)((char *)new_block + sizeof(t_block));
        }
    }
    
    pthread_mutex_unlock(&g_heap.mutex);
    
    // Fallback vers l'ancienne méthode
    new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    ft_memcpy(new_ptr, ptr, block->size);
    free(ptr);
    return new_ptr;
}
