#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void show_alloc_mem_ex(void);

int main(void)
{
    printf("=== malloc Debug Environment Variables Demo ===\n");
    printf("Available environment variables:\n");
    printf("- MALLOC_SCRIBBLE=1      : Fill freed memory with 0xDE\n");
    printf("- MALLOC_PRE_SCRIBBLE=1  : Fill allocated memory with 0xAA\n");
    printf("- MALLOC_GUARD=1         : Enable guard checking (placeholder)\n");
    printf("- MALLOC_STACK_LOGGING=1 : Enable allocation history tracking\n");
    printf("- MALLOC_CHECK_=0-3      : Set malloc checking level\n");
    printf("\nCurrent configuration and allocations:\n\n");
    
    void *p1 = malloc(64);
    void *p2 = malloc(512);
    void *p3 = malloc(5000); // Large allocation
    
    strcpy((char *)p1, "Small allocation");
    strcpy((char *)p2, "Medium allocation");  
    strcpy((char *)p3, "Large allocation");
    
    show_alloc_mem_ex();
    
    free(p1);
    free(p2);
    free(p3);
    
    return 0;
}