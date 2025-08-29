#include "test_framework.h"

int main(void)
{
    printf(MAGENTA "==================================" RESET "\n");
    printf(MAGENTA "    MALLOC LIBRARY UNIT TESTS    " RESET "\n");
    printf(MAGENTA "==================================" RESET "\n\n");
    
    // Initialize test counters
    g_tests_run = 0;
    g_tests_passed = 0;
    g_tests_failed = 0;
    
    // Run all tests
    test_malloc_zero();
    test_free_null();
    test_basic_malloc();
    test_large_malloc();
    test_realloc_null();
    test_realloc_zero();
    test_realloc_basic();
    test_multiple_allocations();
    test_fragmentation();
    test_stress_test();
    test_page_overhead();
    
    // Print summary
    TEST_SUMMARY();
    
    // Return appropriate exit code
    return (g_tests_failed == 0) ? 0 : 1;
}
