#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* These functions are from our malloc implementation */
extern void show_alloc_mem_ex(void);
extern void malloc_defragment(void);

int main(void)
{
    void *ptr1, *ptr2, *ptr3;
    
    printf("=== Testing malloc bonus features ===\n");
    
    /* Test basic allocations */
    printf("\n1. Making some allocations...\n");
    ptr1 = malloc(42);
    strcpy((char *)ptr1, "Hello world from malloc!");
    
    ptr2 = malloc(1024);
    memset(ptr2, 0x42, 1024);
    
    ptr3 = malloc(8192); /* LARGE allocation */
    strcpy((char *)ptr3, "This is a large allocation for testing purposes.");
    
    printf("Allocations done!\n");
    
    /* Show detailed memory information */
    printf("\n2. Showing detailed memory info with show_alloc_mem_ex():\n");
    show_alloc_mem_ex();
    
    /* Free some memory */
    printf("\n3. Freeing some memory...\n");
    free(ptr2);
    
    /* Test defragmentation */
    printf("\n4. Running defragmentation...\n");
    malloc_defragment();
    
    printf("\n5. Memory after defragmentation:\n");
    show_alloc_mem_ex();
    
    /* Clean up */
    free(ptr1);
    free(ptr3);
    
    printf("\n=== Test completed ===\n");
    return 0;
}