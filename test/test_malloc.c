#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/malloc.h"

void	test_basic_malloc(void)
{
	printf("=== Test Basic Malloc ===\n");
	
	char *ptr1 = malloc(50);
	char *ptr2 = malloc(100);
	char *ptr3 = malloc(200);
	
	if (ptr1 && ptr2 && ptr3)
	{
		strcpy(ptr1, "Hello");
		strcpy(ptr2, "World");
		strcpy(ptr3, "Malloc Test");
		
		printf("ptr1: %s\n", ptr1);
		printf("ptr2: %s\n", ptr2);
		printf("ptr3: %s\n", ptr3);
		
		free(ptr1);
		free(ptr2);
		free(ptr3);
		printf("✓ Basic malloc test passed\n");
	}
	else
	{
		printf("✗ Basic malloc test failed\n");
	}
	printf("\n");
}

void	test_large_malloc(void)
{
	printf("=== Test Large Malloc ===\n");
	
	char *large_ptr = malloc(5000);
	if (large_ptr)
	{
		memset(large_ptr, 'A', 4999);
		large_ptr[4999] = '\0';
		printf("Large allocation successful (size: 5000)\n");
		free(large_ptr);
		printf("✓ Large malloc test passed\n");
	}
	else
	{
		printf("✗ Large malloc test failed\n");
	}
	printf("\n");
}

void	test_realloc(void)
{
	printf("=== Test Realloc ===\n");
	
	char *ptr = malloc(50);
	if (ptr)
	{
		strcpy(ptr, "Initial data");
		printf("Initial: %s\n", ptr);
		
		ptr = realloc(ptr, 100);
		if (ptr)
		{
			strcat(ptr, " - Extended");
			printf("Extended: %s\n", ptr);
			
			ptr = realloc(ptr, 25);
			if (ptr)
			{
				printf("Shrunk: %s\n", ptr);
				free(ptr);
				printf("✓ Realloc test passed\n");
			}
			else
			{
				printf("✗ Realloc shrink failed\n");
			}
		}
		else
		{
			printf("✗ Realloc extend failed\n");
		}
	}
	else
	{
		printf("✗ Initial malloc failed\n");
	}
	printf("\n");
}

void	test_edge_cases(void)
{
	printf("=== Test Edge Cases ===\n");
	
	// Test malloc(0)
	size_t zero = 0;
	char *ptr1 = malloc(zero);
	printf("malloc(0): %s\n", ptr1 ? "returned pointer" : "returned NULL");
	if (ptr1) free(ptr1);
	
	// Test free(NULL)
	free(NULL);
	printf("free(NULL): no crash\n");
	
	// Test realloc(NULL, size)
	char *ptr2 = realloc(NULL, 50);
	if (ptr2)
	{
		strcpy(ptr2, "realloc from NULL");
		printf("realloc(NULL, 50): %s\n", ptr2);
		free(ptr2);
	}
	
	// Test realloc(ptr, 0)
	char *ptr3 = malloc(50);
	size_t zero_size = 0;
	ptr3 = realloc(ptr3, zero_size);
	printf("realloc(ptr, 0): %s\n", ptr3 ? "returned pointer" : "returned NULL");
	
	printf("✓ Edge cases test completed\n");
	printf("\n");
}

void	test_multiple_allocations(void)
{
	printf("=== Test Multiple Allocations ===\n");
	
	char *ptrs[10];
	int i;
	
	// Initialize array
	for (i = 0; i < 10; i++)
		ptrs[i] = NULL;
	
	// Allocate multiple blocks
	for (i = 0; i < 10; i++)
	{
		ptrs[i] = malloc((i + 1) * 10);
		if (ptrs[i])
		{
			sprintf(ptrs[i], "Block %d", i);
		}
	}
	
	// Verify data
	for (i = 0; i < 10; i++)
	{
		if (ptrs[i])
		{
			printf("ptrs[%d]: %s\n", i, ptrs[i]);
		}
	}
	
	// Free odd indices first
	for (i = 1; i < 10; i += 2)
	{
		if (ptrs[i])
		{
			free(ptrs[i]);
			ptrs[i] = NULL;
		}
	}
	
	// Allocate new blocks for odd indices
	for (i = 1; i < 10; i += 2)
	{
		ptrs[i] = malloc(15);
		if (ptrs[i])
		{
			sprintf(ptrs[i], "New %d", i);
			printf("ptrs[%d]: %s (reallocated)\n", i, ptrs[i]);
		}
	}
	
	// Free all remaining
	for (i = 0; i < 10; i++)
	{
		if (ptrs[i])
		{
			free(ptrs[i]);
			ptrs[i] = NULL;
		}
	}
	
	printf("✓ Multiple allocations test completed\n");
	printf("\n");
}

int	main(void)
{
	printf("=== Custom Malloc Library Test ===\n\n");
	
	test_basic_malloc();
	test_large_malloc();
	test_realloc();
	test_edge_cases();
	test_multiple_allocations();
	
	printf("=== All tests completed ===\n");
	return (0);
}
