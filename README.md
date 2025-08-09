# Custom malloc (libft_malloc)

A pedagogical memory allocator implementation (malloc/free/realloc) with TINY/SMALL/LARGE zones, a global mutex for thread safety, and integrated unit tests.

## Features
- malloc/free/realloc
- Zones: TINY (<= 512), SMALL (<= 4096), LARGE (direct mmap)
- 16-byte alignment
- Block split/merge (fragmentation-friendly)
- Global mutex (`pthread_mutex_t`) for serialization
- Shared library: `libft_malloc_<HOSTTYPE>.so` (+ symlink `libft_malloc.so`)
- Built-in unit tests and LD_PRELOAD wrapper script

## Requirements
- GCC/Clang
- make
- pthreads (glibc)
- Linux (uses `mmap` and `pthread`)

## Build
```zsh
# Optional: add a suffix to the library filename
export HOSTTYPE=Testing
# Build libft (local copy) + shared library
make
# Create the expected symlink: libft_malloc.so -> libft_malloc_<HOSTTYPE>.so
make symlink
```

Artifacts:
- `libft_malloc_<HOSTTYPE>.so` (e.g., `libft_malloc_Testing.so`)
- `libft_malloc.so` (symlink if you run `make symlink`)
- Object files in `.tmp/`

## Usage (LD_PRELOAD)
```zsh
# Preload the library (using the symlink)
export LD_LIBRARY_PATH=.
export LD_PRELOAD=./libft_malloc.so
./your_program
```

Wrapper provided:
```zsh
./run_linux.sh ./your_program
```

## Unit tests
```zsh
# Run the integrated test suite
make test
# With ASan/UBSan
make test-debug
# With valgrind (if installed)
make test-valgrind
```
Test coverage: malloc(0), free(NULL), realloc (NULL/0/resize), large allocations, multiple allocations, fragmentation, stress.

## Simple example tests
```zsh
# test0
gcc -o test0 test/test0.c && ./run_linux.sh ./test0
# test1
gcc -o test1 test/test1.c && ./run_linux.sh ./test1
```

## Correction commands (reference)
See `test/correction_cmd.txt` for typical correction commands (export HOSTTYPE, nm, time, wrapper, etc.).

## Project structure
```
include/       # headers (malloc.h, libft.h)
libft/         # bundled libft (local copy)
src/           # malloc.c, free.c, realloc.c, zones.c, block.c, utils.c
.tmp/          # objects and intermediates
test/          # unit tests + helpers + simple tests
run_linux.sh   # LD_PRELOAD wrapper
```

## Implementation details
- `malloc`: routes to TINY/SMALL via `allocate_from_zone`, LARGE via `allocate_large`
- `allocate_from_zone`: creates/initializes a zone if needed, finds/splits a block
- `allocate_large`: direct `mmap`, links into `g_heap.large`
- `free`: marks block free + `merge_blocks`
- `realloc`: allocates if needed, copies (`ft_memcpy`), frees the old block
- Alignment: `ALIGN(...)` macro (16 bytes)

## Known limitations
- No `munmap` of LARGE blocks on `free` yet (possible enhancement)
- No full bi-directional coalescing (e.g., with previous block)
- `show_alloc_mem(_ex)` not implemented
- Multi-thread performance is limited by global mutex

## Troubleshooting
- If the symlink is missing: `make symlink`
- If `LD_PRELOAD` is ignored: ensure `LD_LIBRARY_PATH` includes `.`
- If `nm` shows nothing: verify you point to the actual `.so` or its symlink

## License
Educational project. Use at your own risk.
