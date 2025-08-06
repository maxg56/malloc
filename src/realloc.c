#include "../include/malloc.h"

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

    block = (t_block *)((char *)ptr - sizeof(t_block));
    if (block->size >= size)
        return ptr;

    new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    ft_memcpy(new_ptr, ptr, block->size);
    free(ptr);
    return new_ptr;
}
