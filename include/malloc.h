#ifndef MALLOC_H
# define MALLOC_H

# define _GNU_SOURCE
# include <sys/mman.h>
# include <unistd.h>
# include <pthread.h>
# include <stddef.h>
# include <stdbool.h>
# include <stdlib.h>
# include <stdint.h>
# include <time.h>



# define TINY_MAX       512
# define SMALL_MAX      4096

# define PAGE_SIZE      getpagesize()
# define TINY_ZONE_SIZE  (PAGE_SIZE * 4)    // 16 Ko - Réduit de 64Ko
# define SMALL_ZONE_SIZE (PAGE_SIZE * 32)   // 128 Ko - Réduit de 512Ko

# define ALIGNMENT 16
# define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* Debug environment variables */
# define MALLOC_SCRIBBLE_FREE 0xDE
# define MALLOC_SCRIBBLE_ALLOC 0xAA
# define MAX_ALLOC_HISTORY 1000

typedef struct s_alloc_history {
    void            *ptr;
    size_t          size;
    time_t          timestamp;
    bool            is_freed;
    struct s_alloc_history *next;
} t_alloc_history;

typedef struct s_debug_flags {
    bool            scribble;        // MALLOC_SCRIBBLE
    bool            pre_scribble;    // MALLOC_PRE_SCRIBBLE  
    bool            guard;           // MALLOC_GUARD
    bool            stack_logging;   // MALLOC_STACK_LOGGING
    int             check_level;     // MALLOC_CHECK_ (0-3)
} t_debug_flags;

typedef struct s_block {
    size_t          size;
    bool             is_free;
    struct s_block  *next;
    time_t          alloc_time;      // For history tracking
} t_block;

typedef struct s_zone {
    size_t          size;
    struct s_zone   *next;
    t_block         *blocks;
} t_zone;

typedef struct s_heap {
    t_zone              *tiny;
    t_zone              *small;
    t_block             *large;
    pthread_mutex_t     mutex;
    t_debug_flags       debug;
    t_alloc_history     history_buffer[MAX_ALLOC_HISTORY];  // Buffer statique
    size_t              history_count;
} t_heap;

extern t_heap g_heap;

/*
    * Memory allocation functions
    * These functions are responsible for allocating, freeing, and reallocating memory
*/
void *malloc(size_t size);

/*
    * Freeing memory
    * This function is responsible for freeing allocated memory blocks
*/
void free(void *ptr);

/*
    * Reallocating memory
    * This function is responsible for resizing allocated memory blocks
*/
void *realloc(void *ptr, size_t size);

/* Helper functions */
t_block *allocate_block(size_t size);
t_zone  *create_zone(size_t zone_size);
void    *allocate_large(size_t size);

/*
    * Memory management functions
    * These functions are responsible for finding, allocating, and managing memory blocks
*/
t_block *find_free_block(t_zone *zone, size_t size);
/*
    * Block splitting
    * This function is responsible for splitting a block into two smaller blocks
*/
t_block *split_block(t_block *block, size_t size);
/*
    * Block merging
    * This function is responsible for merging adjacent free blocks into a larger block
*/
void    merge_blocks(t_block *block);

void *allocate_from_zone(t_zone **zone, size_t size, size_t zone_size);

/* Introspection/visualization */
void show_alloc_mem(void);
void show_alloc_mem_ex(void);

/* Debug functions */
void init_debug_flags(void);
void add_to_history(void *ptr, size_t size, bool is_alloc);
void scribble_memory(void *ptr, size_t size, unsigned char pattern);
void check_guards(void *ptr);
void defragment_zones(void);

/* Public API for defragmentation */
void malloc_defragment(void);

void *ft_memcpy(void *dest, const void *src, size_t n);
#endif