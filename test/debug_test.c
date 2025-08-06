#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/malloc.h"

int main(void)
{
    printf("=== Debug Malloc Test ===\n");
    
    char *ptr1 = malloc(50);
    char *ptr2 = malloc(100);
    char *ptr3 = malloc(200);
    
    if (ptr1 && ptr2 && ptr3)
    {
        strcpy(ptr1, "Test1");
        strcpy(ptr2, "Test2");
        strcpy(ptr3, "Test3");
        
        printf("Allocated 3 blocks\n");
        show_alloc_mem();
        
        free(ptr2);
        printf("After freeing ptr2:\n");
        show_alloc_mem();
        
        free(ptr1);
        free(ptr3);
        printf("After freeing all:\n");
        show_alloc_mem();
    }
    
    return 0;
}
