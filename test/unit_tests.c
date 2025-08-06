#include "test_framework.h"

// Global test counters
int g_tests_run = 0;
int g_tests_passed = 0;
int g_tests_failed = 0;

void test_basic_malloc(void)
{
    TEST_START("Basic malloc and free");
    
    char *ptr = malloc(100);
    TEST_ASSERT(ptr != NULL, "malloc(100) should return non-NULL");
    
    strcpy(ptr, "Hello, World!");
    TEST_ASSERT(strcmp(ptr, "Hello, World!") == 0, "Memory should be writable and readable");
    
    free(ptr);
    TEST_ASSERT(1, "free() should not crash");
    
    TEST_END();
}

void test_free_null(void)
{
    TEST_START("Free NULL pointer");
    
    free(NULL);
    TEST_ASSERT(1, "free(NULL) should not crash");
    
    TEST_END();
}

void test_malloc_zero(void)
{
    TEST_START("Malloc with zero size");
    
    void *ptr = malloc(0);
    if (ptr == NULL) {
        TEST_ASSERT(1, "malloc(0) should return NULL");
    } else {
        printf("  DEBUG: malloc(0) returned %p (expected NULL)\n", ptr);
        TEST_ASSERT(ptr == NULL, "malloc(0) should return NULL");
        free(ptr); // Clean up if it actually allocated something
    }
    
    TEST_END();
}

void test_large_malloc(void)
{
    TEST_START("Large malloc (> 4096 bytes)");
    
    char *ptr = malloc(8192);
    TEST_ASSERT(ptr != NULL, "malloc(8192) should return non-NULL");
    
    // Test writing to the allocated memory
    memset(ptr, 'A', 8191);
    ptr[8191] = '\0';
    TEST_ASSERT(ptr[0] == 'A' && ptr[8190] == 'A', "Large memory should be writable");
    
    free(ptr);
    TEST_ASSERT(1, "free() of large allocation should not crash");
    
    TEST_END();
}

void test_realloc_basic(void)
{
    TEST_START("Basic realloc");
    
    char *ptr = malloc(50);
    TEST_ASSERT(ptr != NULL, "Initial malloc should succeed");
    
    strcpy(ptr, "Hello");
    
    ptr = realloc(ptr, 100);
    TEST_ASSERT(ptr != NULL, "realloc should return non-NULL");
    TEST_ASSERT(strcmp(ptr, "Hello") == 0, "realloc should preserve data");
    
    free(ptr);
    TEST_ASSERT(1, "free() after realloc should not crash");
    
    TEST_END();
}

void test_realloc_null(void)
{
    TEST_START("Realloc with NULL pointer");
    
    char *ptr = realloc(NULL, 100);
    TEST_ASSERT(ptr != NULL, "realloc(NULL, size) should work like malloc");
    
    strcpy(ptr, "Test");
    TEST_ASSERT(strcmp(ptr, "Test") == 0, "Memory should be usable");
    
    free(ptr);
    TEST_END();
}

void test_realloc_zero(void)
{
    TEST_START("Realloc with zero size");
    
    char *ptr = malloc(100);
    TEST_ASSERT(ptr != NULL, "Initial malloc should succeed");
    
    ptr = realloc(ptr, 0);
    TEST_ASSERT(ptr == NULL, "realloc(ptr, 0) should free memory and return NULL");
    
    TEST_END();
}

void test_multiple_allocations(void)
{
    TEST_START("Multiple allocations");
    
    char *ptrs[10];
    
    // Allocate multiple blocks
    for (int i = 0; i < 10; i++)
    {
        ptrs[i] = malloc(100 + i * 10);
        TEST_ASSERT(ptrs[i] != NULL, "Each allocation should succeed");
        sprintf(ptrs[i], "Block %d", i);
    }
    
    // Verify all blocks
    for (int i = 0; i < 10; i++)
    {
        char expected[20];
        sprintf(expected, "Block %d", i);
        TEST_ASSERT(strcmp(ptrs[i], expected) == 0, "Each block should retain its data");
    }
    
    // Free all blocks
    for (int i = 0; i < 10; i++)
    {
        free(ptrs[i]);
    }
    
    TEST_END();
}

void test_fragmentation(void)
{
    TEST_START("Memory fragmentation handling");
    
    char *ptrs[5];
    
    // Allocate 5 blocks
    for (int i = 0; i < 5; i++)
    {
        ptrs[i] = malloc(200);
        TEST_ASSERT(ptrs[i] != NULL, "Initial allocations should succeed");
    }
    
    // Free every other block to create fragmentation
    free(ptrs[1]);
    free(ptrs[3]);
    
    // Try to allocate blocks that should fit in the freed spaces
    char *new_ptr1 = malloc(150);
    char *new_ptr2 = malloc(150);
    
    TEST_ASSERT(new_ptr1 != NULL && new_ptr2 != NULL, "Should reuse fragmented memory");
    
    // Clean up
    free(ptrs[0]);
    free(ptrs[2]);
    free(ptrs[4]);
    free(new_ptr1);
    free(new_ptr2);
    
    TEST_END();
}

void test_stress_test(void)
{
    TEST_START("Stress test - many allocations");
    
    #define STRESS_COUNT 100
    char *ptrs[STRESS_COUNT];
    
    // Allocate many blocks of varying sizes
    for (int i = 0; i < STRESS_COUNT; i++)
    {
        size_t size = (i % 10 + 1) * 100;  // Sizes from 100 to 1000
        ptrs[i] = malloc(size);
        TEST_ASSERT(ptrs[i] != NULL, "Stress allocation should succeed");
        
        // Write some data
        sprintf(ptrs[i], "Stress %d", i);
    }
    
    // Verify and free in reverse order
    for (int i = STRESS_COUNT - 1; i >= 0; i--)
    {
        char expected[20];
        sprintf(expected, "Stress %d", i);
        TEST_ASSERT(strcmp(ptrs[i], expected) == 0, "Stress data should be intact");
        free(ptrs[i]);
    }
    
    TEST_END();
}
