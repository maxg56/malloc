#include "../include/malloc.h"

void free(void *ptr)
{
    t_block *block;

    if (!ptr)
        return;

    block = (t_block *)((char *)ptr - sizeof(t_block));
    block->is_free = true;

    // Merge adjacent free blocks
    merge_blocks(block);
}
