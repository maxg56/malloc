# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build the malloc library (creates libft_malloc_<HOSTTYPE>.so)
make

# Build with custom HOSTTYPE suffix (useful for testing)
export HOSTTYPE=Testing
make

# Create symlink (libft_malloc.so -> libft_malloc_<HOSTTYPE>.so)
make symlink

# Clean build artifacts
make clean

# Full rebuild
make re
```

## Testing Commands

```bash
# Run comprehensive unit tests
make test

# Run tests with debugging/sanitizers enabled
make test-debug

# Run tests with valgrind (if available)
make test-valgrind

# Run individual test programs
gcc -o test0 test/test0.c && ./run_linux.sh ./test0
gcc -o test1 test/test1.c && ./run_linux.sh ./test1

# Test with system timing
./run_linux.sh /usr/bin/time -v ./test0
```

## Library Usage

The project builds a shared library that replaces system malloc via LD_PRELOAD:

```bash
# Using the wrapper script (recommended)
./run_linux.sh ./your_program

# Manual LD_PRELOAD setup
export LD_LIBRARY_PATH=.
export LD_PRELOAD=./libft_malloc.so
./your_program
```

## Architecture Overview

### Memory Zone System
The allocator implements a three-tier zone system:
- **TINY zones**: allocations ≤ 512 bytes (64KB zones via mmap)
- **SMALL zones**: allocations ≤ 4096 bytes (512KB zones via mmap) 
- **LARGE blocks**: allocations > 4096 bytes (individual mmap per block)

### Core Data Structures
- `t_heap g_heap`: Global heap with pointers to tiny/small zones and large blocks
- `t_zone`: Memory zone containing linked list of blocks
- `t_block`: Individual allocation block with size, free status, and next pointer
- `pthread_mutex_t`: Global mutex for thread safety

### Key Files and Functions
- `src/malloc.c`: Main allocation entry point, routes to appropriate zone
- `src/zones.c`: Zone management (`allocate_from_zone`, `create_zone`, `allocate_large`)
- `src/block.c`: Block operations (`find_free_block`, `split_block`, `merge_blocks`)
- `src/free.c`: Deallocation with LARGE block unmapping
- `src/realloc.c`: Reallocation with copy semantics
- `src/utils.c`: Debug output (`show_alloc_mem`) and utilities

### Memory Management Strategy
1. **Allocation**: Find suitable free block in existing zones, split if oversized
2. **New zones**: Created when no suitable blocks found (linked list of zones)
3. **Fragmentation control**: Block splitting and adjacent block merging
4. **Large blocks**: Direct mmap/munmap, stored in separate linked list
5. **Thread safety**: All operations protected by single global mutex

### Current Limitations
- No backward coalescing (only merges with next block)
- LARGE blocks not unmapped on free (implemented but could be enhanced)
- Single global mutex limits concurrent performance
- show_alloc_mem_ex() is stub implementation

## Development Notes

- All allocations are 16-byte aligned via `ALIGN()` macro
- Block headers precede user data: `[t_block][user_data]`
- Zone sizes calculated to support 100+ allocations minimum
- Uses bundled libft for utility functions
- Thread safety implemented but performance-limited by global locking