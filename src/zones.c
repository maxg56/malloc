#include "../include/malloc.h"

void *allocate_from_zone(t_zone **zone, size_t size, size_t zone_size)
{
    t_block *block;
    t_zone *current_zone;

    // If no zone exists, create the first one
    if (!*zone)
    {
        *zone = create_zone(zone_size);
        if (!*zone)
            return NULL;
        
        // Initialize the first block in the new zone
        char *zone_start = (char *)*zone + sizeof(t_zone);
        char *aligned_start = (char *)((((uintptr_t)zone_start) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1));
        (*zone)->blocks = (t_block *)aligned_start;
        size_t used_space = aligned_start - (char *)*zone;
        (*zone)->blocks->size = zone_size - used_space - sizeof(t_block);
        (*zone)->blocks->is_free = true;
        (*zone)->blocks->next = NULL;
        (*zone)->blocks->alloc_time = 0;
    }

    // Try to find a free block in existing zones
    current_zone = *zone;
    while (current_zone)
    {
        block = find_free_block(current_zone, size);
        if (block)
        {
            // Split the block if it's too big
            if (block->size > size + sizeof(t_block) + ALIGNMENT)
                split_block(block, size);
            
            block->is_free = false;
            block->alloc_time = time(NULL);
            return (void *)((char *)block + sizeof(t_block));
        }
        current_zone = current_zone->next;
    }

    // If no suitable block found, create a new zone
    t_zone *new_zone = create_zone(zone_size);
    if (!new_zone)
        return NULL;
    
    // Initialize the first block in the new zone
    char *zone_start = (char *)new_zone + sizeof(t_zone);
    char *aligned_start = (char *)((((uintptr_t)zone_start) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1));
    new_zone->blocks = (t_block *)aligned_start;
    size_t used_space = aligned_start - (char *)new_zone;
    new_zone->blocks->size = zone_size - used_space - sizeof(t_block);
    new_zone->blocks->is_free = true;
    new_zone->blocks->next = NULL;
    new_zone->blocks->alloc_time = 0;
    
    // Link the new zone to the existing chain
    new_zone->next = *zone;
    *zone = new_zone;
    
    // Allocate from the new zone
    block = new_zone->blocks;
    if (block->size > size + sizeof(t_block) + ALIGNMENT)
        split_block(block, size);
    
    block->is_free = false;
    block->alloc_time = time(NULL);
    return (void *)((char *)block + sizeof(t_block));
}



t_zone *create_zone(size_t zone_size)
{
    t_zone *new_zone;

    new_zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_zone == MAP_FAILED)
        return NULL;

    new_zone->size = zone_size;
    new_zone->next = NULL;
    new_zone->blocks = NULL;

    return new_zone;
}

void *allocate_large(size_t size)
{
    t_block *new_block;
    size_t total_size;

    // Calculate total size needed (block header + aligned size)
    total_size = sizeof(t_block) + size;
    
    // Allocate memory using mmap
    new_block = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_block == MAP_FAILED)
        return NULL;

    // Initialize the block
    new_block->size = size;
    new_block->is_free = false;
    new_block->next = g_heap.large;
    new_block->alloc_time = time(NULL);
    
    // Add to the large blocks list
    g_heap.large = new_block;

    // Return pointer to usable memory (after the block header)
    return (void *)((char *)new_block + sizeof(t_block));
}
