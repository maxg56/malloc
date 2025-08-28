#include "../include/malloc.h"

void free(void *ptr)
{
    t_block *block;

    if (!ptr)
        return;

    pthread_mutex_lock(&g_heap.mutex);

    block = (t_block *)((char *)ptr - sizeof(t_block));

    // If it's a LARGE allocation (blocks stored in g_heap.large), unmap it
    t_block *prev = NULL;
    t_block *cur = g_heap.large;
    while (cur)
    {
        if (cur == block)
        {
            // unlink from large list
            if (prev)
                prev->next = cur->next;
            else
                g_heap.large = cur->next;
            size_t total = sizeof(t_block) + cur->size;
            munmap((void *)cur, total);
            pthread_mutex_unlock(&g_heap.mutex);
            return;
        }
        prev = cur;
        cur = cur->next;
    }

    // Otherwise it's a TINY/SMALL block: mark free and try to merge
    
    /* Scribble freed memory if debug flag is set */
    if (g_heap.debug.scribble)
    {
        void *user_ptr = (void *)((char *)block + sizeof(t_block));
        scribble_memory(user_ptr, block->size, MALLOC_SCRIBBLE_FREE);
    }
    
    /* Add to history */
    add_to_history(ptr, block->size, false);
    
    block->is_free = true;
    merge_blocks(block);

    pthread_mutex_unlock(&g_heap.mutex);
}
