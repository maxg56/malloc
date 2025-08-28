#include "../include/malloc.h"

t_block *allocate_block(size_t size)
{
    t_zone *zone;
    t_block *block;

    // Check if the size is within TINY or SMALL limits
    if (size <= TINY_MAX)
        zone = g_heap.tiny;
    else if (size <= SMALL_MAX)
        zone = g_heap.small;
    else
        return NULL; // For simplicity, we do not handle large allocations here

    // Find a free block in the zone
    block = find_free_block(zone, size);
    if (!block)
    {
        // If no free block found, create a new zone or split an existing one
        zone = create_zone(TINY_ZONE_SIZE);
        if (!zone)
            return NULL; // Allocation failed

        g_heap.tiny = zone; // Update the heap with the new zone
        block = allocate_block(size); // Try to allocate again
    }

    return block;
}

t_block *find_free_block(t_zone *zone, size_t size)
{
    t_block *current = zone->blocks;

    while (current)
    {
        if (current->is_free && current->size >= size)
        {
            return current;
        }
        current = current->next;
    }

    return NULL; // No suitable free block found
}

t_block *split_block(t_block *block, size_t size)
{
    if (block->size >= size + sizeof(t_block) + ALIGNMENT)
    {
        t_block *new_block = (t_block *)((char *)block + sizeof(t_block) + size);
        new_block->size = block->size - size - sizeof(t_block);
        new_block->is_free = true;
        new_block->next = block->next;
        new_block->alloc_time = 0;

        block->size = size;
        block->is_free = false;
        block->next = new_block;

        return block; // Return the original block, now resized
    }

    return NULL; // Not enough space to split
}

void merge_blocks(t_block *block)
{
    if (block->next && block->next->is_free)
    {
        block->size += block->next->size + sizeof(t_block);
        block->next = block->next->next;
    }
}
