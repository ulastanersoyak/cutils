# cutils

lightweight c utility library designed for embedded systems. it provides essential data structures and utilities with a focus on memory efficiency and safety.

## features

- data structures:
  - vector (dynamic array)
  - list (doubly linked list)
  - map (red-black tree, ordered key-value store)
  - queue (ring buffer) and priority queue (binary heap)
  - stack
  - string
  - expected (value or error)

- memory management:
  - every container takes a custom allocator, or uses the default one
  - static allocation support (64KB limit by default), no heap needed
  - block pool: fixed size blocks with O(1) alloc and free, reuses freed memory
  - arena allocator, which containers can also allocate from

- safety features:
  - bounds checking
  - overflow protection on every size calculation
  - null pointer checks
  - per module error codes (`vector_get_error ()` and so on)

- real-time support:
  - `*_timeout` variants of operations that may allocate
  - pluggable time source for bare metal targets
  - no recursion in the heap operations, bounded stack use

## building

needs a c23 compiler and cmake 3.22 or newer.

```bash
mkdir build && cd build
cmake ..
make
```

every module can be left out to save flash. the allocator and time code are always built.

```bash
cmake .. -DCUTILS_WITH_MAP=OFF -DCUTILS_WITH_STRING=OFF
```

the modules are `arena`, `expected`, `list`, `map`, `pool`, `pqueue`, `queue`, `stack`, `string` and `vector`.

### using it from another project

as a subdirectory:

```cmake
add_subdirectory(cutils)
target_link_libraries(app PRIVATE cutils::cutils)
```

or installed (`make install`) and found with cmake:

```cmake
find_package(cutils REQUIRED)
target_link_libraries(app PRIVATE cutils::cutils)
```

include `<cutils/cutils.h>` for everything, or the header of a single module such as `<cutils/vector.h>`.

## configuration

every option in `includes/cutils/config.h` can be overridden without editing the file, either on the command line:

```bash
cmake .. -DCMAKE_C_FLAGS="-DCUTILS_MAX_STATIC_MEMORY=8192"
```

or with a header of your own:

```bash
cmake .. -DCMAKE_C_FLAGS='-DCUTILS_USER_CONFIG=\"my_cutils_config.h\"'
```

the most useful ones:

| option | default | |
| --- | --- | --- |
| `CUTILS_USE_STATIC_ALLOCATION` | 1 | default allocator hands out memory from a static buffer |
| `CUTILS_USE_DYNAMIC_ALLOCATION` | 0 | default allocator uses `aligned_alloc` and `free` |
| `CUTILS_MAX_STATIC_MEMORY` | 64KB | size of the static buffer |
| `CUTILS_ALIGNMENT` | 8 | default alignment, must be a power of two |
| `CUTILS_PLATFORM_BARE_METAL` | 1 | don't use the os clock |
| `CUTILS_MAX_OPERATION_TIME_MS` | 1 | timeout used by operations without a `_timeout` suffix |
| `CUTILS_VECTOR_MAX_CAPACITY` | 1024 | largest capacity a vector may grow to |

exactly one of the two allocation modes must be enabled.

## usage

```c
#include <cutils/vector.h>
#include <stdio.h>

int
main (void)
{
  vector_t *numbers = vector_create (0, sizeof (int));
  if (numbers == NULL)
    {
      return 1;
    }

  for (int i = 0; i < 10; i++)
    {
      vector_push (numbers, &i);
    }

  int value;
  if (vector_get (numbers, 3, &value))
    {
      printf ("%d\n", value);
    }

  vector_destroy (numbers);
  return 0;
}
```

functions that can fail return `false` or `NULL` and record why in a per module, per thread error code:

```c
if (!vector_get (numbers, 100, &value))
  {
    if (vector_get_error () == VECTOR_OUT_OF_RANGE)
      {
        /* ... */
      }
  }
```

### allocators

the default static pool never reclaims memory, which suits containers that are created once at startup. for containers that are created and destroyed repeatedly, give them an allocator that reuses memory.

a block pool, for containers with same sized nodes such as list and map:

```c
static uint8_t node_memory[4096];
cutils_block_pool_t pool;
cutils_block_pool_init (&pool, node_memory, sizeof node_memory, 64);

cutils_allocator_t allocator = cutils_block_pool_allocator (&pool);
list_t *list = list_create_with_allocator (sizeof (int), &allocator);
```

an arena, to throw away a batch of temporary containers at once:

```c
arena_t *scratch = arena_create (2048, 8);
cutils_allocator_t scratch_allocator = arena_allocator (scratch);

string_t *line = string_create_with_allocator (0, &scratch_allocator);
/* ... */
arena_clear (scratch); /* frees line and everything else from scratch */
```

or your own, by filling in a `cutils_allocator_t` with `allocate` and `deallocate` functions. the allocator must outlive every container that uses it.

### time on bare metal

bare metal builds have no clock, so timeouts never fire until you give the library one:

```c
static uint64_t
systick_ms (void)
{
  return hal_get_tick ();
}

cutils_set_time_source (systick_ms);
```
