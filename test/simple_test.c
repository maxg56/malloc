#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    printf("=== Simple Malloc Test ===\n");
    
    // Test simple allocation
    char *ptr = malloc(100);
    if (ptr)
    {
        strcpy(ptr, "Hello World");
        printf("Allocated: %s\n", ptr);
        free(ptr);
        printf("Freed successfully\n");
    }
    
    printf("=== Test completed ===\n");
    return 0;
}
