#!/bin/bash

echo "=== malloc BONUS Features Demonstration ==="
echo 

# Compile the demo
gcc -o test_env_demo test/test_env_demo.c -L. -l:libft_malloc.so

echo "1. Standard malloc (no debug flags):"
echo "   ./run_linux.sh ./test_env_demo"
echo
./run_linux.sh ./test_env_demo | head -15
echo "..."
echo

echo "2. With MALLOC_SCRIBBLE (freed memory filled with 0xDE):"
echo "   MALLOC_SCRIBBLE=1 ./run_linux.sh ./test_env_demo"
echo
MALLOC_SCRIBBLE=1 ./run_linux.sh ./test_env_demo | head -10
echo "..."
echo

echo "3. With MALLOC_PRE_SCRIBBLE (allocated memory filled with 0xAA):"
echo "   MALLOC_PRE_SCRIBBLE=1 ./run_linux.sh ./test_env_demo"
echo
MALLOC_PRE_SCRIBBLE=1 ./run_linux.sh ./test_env_demo | head -10
echo "..."
echo

echo "4. With MALLOC_STACK_LOGGING (allocation history tracking):"
echo "   MALLOC_STACK_LOGGING=1 ./run_linux.sh ./test_env_demo"
echo
MALLOC_STACK_LOGGING=1 ./run_linux.sh ./test_env_demo | head -15
echo "..."
echo

echo "5. All debug flags enabled:"
echo "   MALLOC_SCRIBBLE=1 MALLOC_PRE_SCRIBBLE=1 MALLOC_STACK_LOGGING=1 MALLOC_CHECK_=2"
echo
MALLOC_SCRIBBLE=1 MALLOC_PRE_SCRIBBLE=1 MALLOC_STACK_LOGGING=1 MALLOC_CHECK_=2 ./run_linux.sh ./test_env_demo | head -20
echo "..."
echo

echo "=== BONUS Features Summary ==="
echo "✅ Thread Safety: Already implemented with pthread_mutex"
echo "✅ Debug Environment Variables:"
echo "   - MALLOC_SCRIBBLE, MALLOC_PRE_SCRIBBLE, MALLOC_GUARD, MALLOC_CHECK_"
echo "   - MALLOC_STACK_LOGGING for allocation history"
echo "✅ show_alloc_mem_ex(): Enhanced with hex dumps and allocation history"
echo "✅ Memory Defragmentation: malloc_defragment() function available"
echo
echo "All bonus requirements have been implemented! 🎉"

# Cleanup
rm -f test_env_demo