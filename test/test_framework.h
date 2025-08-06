#ifndef TEST_FRAMEWORK_H
# define TEST_FRAMEWORK_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <assert.h>
# include "../include/malloc.h"

// Colors for output
# define RED     "\033[0;31m"
# define GREEN   "\033[0;32m"
# define YELLOW  "\033[0;33m"
# define BLUE    "\033[0;34m"
# define MAGENTA "\033[0;35m"
# define CYAN    "\033[0;36m"
# define WHITE   "\033[0;37m"
# define RESET   "\033[0m"

// Test counters
extern int g_tests_run;
extern int g_tests_passed;
extern int g_tests_failed;

// Test macros
# define TEST_START(name) \
    do { \
        printf(CYAN "Running: %s" RESET "\n", name); \
        g_tests_run++; \
    } while(0)

# define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf(GREEN "  ✓ %s" RESET "\n", message); \
        } else { \
            printf(RED "  ✗ %s" RESET "\n", message); \
            g_tests_failed++; \
            return; \
        } \
    } while(0)

# define TEST_END() \
    do { \
        printf(GREEN "  ✓ Test completed successfully" RESET "\n\n"); \
        g_tests_passed++; \
    } while(0)

# define TEST_SUMMARY() \
    do { \
        printf("\n" YELLOW "=== TEST SUMMARY ===" RESET "\n"); \
        printf("Tests run: %d\n", g_tests_run); \
        printf(GREEN "Tests passed: %d" RESET "\n", g_tests_passed); \
        if (g_tests_failed > 0) { \
            printf(RED "Tests failed: %d" RESET "\n", g_tests_failed); \
        } \
        printf(YELLOW "===================" RESET "\n"); \
        if (g_tests_failed == 0) { \
            printf(GREEN "All tests passed!" RESET "\n"); \
        } else { \
            printf(RED "Some tests failed!" RESET "\n"); \
        } \
    } while(0)

// Test functions declarations
void test_basic_malloc(void);
void test_free_null(void);
void test_malloc_zero(void);
void test_large_malloc(void);
void test_realloc_basic(void);
void test_realloc_null(void);
void test_realloc_zero(void);
void test_multiple_allocations(void);
void test_fragmentation(void);
void test_stress_test(void);

#endif
