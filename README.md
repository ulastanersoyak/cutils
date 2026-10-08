# cutils

c utility library for personal use. containers, allocators and a few helpers, written in c23 with embedded targets in mind: no heap required, every size calculation overflow checked, and each module can be left out of the build.

## modules

| module | header | |
| --- | --- | --- |
| vector | `cutils/vector.h` | dynamic array |
| list | `cutils/list.h` | doubly linked list with iterators |
| map | `cutils/map.h` | ordered key-value store (red-black tree) with range queries |
| hashmap | `cutils/hashmap.h` | unordered key-value store (open addressing), O(1) average lookups |
| queue | `cutils/queue.h` | fifo ring buffer |
| pqueue | `cutils/pqueue.h` | priority queue (binary heap) with a custom comparison |
| stack | `cutils/stack.h` | lifo stack |
| string | `cutils/string.h` | growable string with search, substrings and printf style formatting |
| bitset | `cutils/bitset.h` | fixed size bit array |
| expected | `cutils/expected.h` | a value or an error code |
| arena | `cutils/arena.h` | bump allocator, freed all at once |
| pool | `cutils/pool.h` | fixed size block allocator with O(1) alloc and free |
| allocator | `cutils/allocator.h` | the allocator interface every container takes, plus the static pool |
| time | `cutils/time.h` | millisecond clock and timeouts, with a pluggable source for bare metal |

`cutils/cutils.h` includes all of them.

## quick start

```c
#include <cutils/cutils.h>
#include <stdio.h>

int
main (void)
{
  hashmap_t *ages = hashmap_create (sizeof (int), sizeof (int), NULL, NULL);

  int id = 7;
  int age = 31;
  hashmap_insert (ages, &id, &age);

  if (hashmap_get (ages, &id, &age))
    {
      printf ("%d is %d\n", id, age);
    }

  hashmap_destroy (ages);
  return 0;
}
```

containers copy elements in and out by value. a container is told its element size when it's created, and every function takes and fills pointers to elements.

## building

needs a c23 compiler (gcc 13+ or clang 18+) and cmake 3.22 or newer.

```bash
cmake -B build
cmake --build build
ctest --test-dir build    # run the tests
```

| option | default | |
| --- | --- | --- |
| `CUTILS_WITH_<MODULE>` | `ON` | build a module, e.g. `-DCUTILS_WITH_MAP=OFF` to leave the map out. the allocator and time code are always built |
| `CUTILS_BUILD_TESTS` | `ON` as the top level project, `OFF` as a subdirectory | build the tests in `tests/` |
| `CUTILS_BUILD_EXAMPLES` | same as tests | build the programs in `examples/` |
| `BUILD_SHARED_LIBS` | `OFF` | build a shared library instead of a static one |

### using it from another project

as a subdirectory, or with `FetchContent`:

```cmake
add_subdirectory(cutils)
target_link_libraries(app PRIVATE cutils::cutils)
```

or installed with `cmake --install build` and found by cmake:

```cmake
find_package(cutils REQUIRED)
target_link_libraries(app PRIVATE cutils::cutils)
```

## configuration

compile time options live in `include/cutils/config.h`. each one can be overridden without editing the file, either on the command line:

```bash
cmake -B build -DCMAKE_C_FLAGS="-DCUTILS_MAX_STATIC_MEMORY=8192"
```

or by pointing `CUTILS_USER_CONFIG` at a header of your own:

```bash
cmake -B build -DCMAKE_C_FLAGS='-DCUTILS_USER_CONFIG=\"my_cutils_config.h\"'
```

| option | default | |
| --- | --- | --- |
| `CUTILS_USE_STATIC_ALLOCATION` | 1 | the default allocator hands out memory from a static buffer |
| `CUTILS_USE_DYNAMIC_ALLOCATION` | 0 | the default allocator uses `aligned_alloc` and `free` instead |
| `CUTILS_MAX_STATIC_MEMORY` | 64KB | size of the static buffer |
| `CUTILS_ALIGNMENT` | 8 | default alignment, a power of two |
| `CUTILS_PLATFORM_BARE_METAL` | 1 | don't touch the os clock |
| `CUTILS_MAX_OPERATION_TIME_MS` | 1 | timeout for operations without a `_timeout` suffix |
| `CUTILS_VECTOR_MAX_CAPACITY` | 1024 | largest capacity a vector may grow to |

exactly one of the two allocation modes has to be enabled, and impossible combinations fail at compile time.

## errors

functions that can fail return `false` or `NULL`, and record why in a per module, per thread error code:

```c
if (!vector_get (numbers, 100, &value))
  {
    if (vector_get_error () == VECTOR_OUT_OF_RANGE)
      {
        /* ... */
      }
  }
```

## memory

every container has a `*_create` function that uses the default allocator, and a `*_create_with_allocator` function that takes any other. the allocator has to outlive the container.

the default allocator is a static pool: one buffer, no heap, but memory is only reclaimed by resetting the whole pool. that's fine for containers created once at startup. for containers that come and go, pick one of these instead.

**block pool**, for containers with same sized nodes (list, map). freed blocks are reused, so it can run forever on a fixed buffer:

```c
static uint8_t node_memory[4096];
cutils_block_pool_t nodes;
cutils_block_pool_init (&nodes, node_memory, sizeof node_memory, 64);

cutils_allocator_t node_allocator = cutils_block_pool_allocator (&nodes);
list_t *list = list_create_with_allocator (sizeof (int), &node_allocator);
```

**arena**, to throw away a batch of temporary containers in one go:

```c
arena_t *scratch = arena_create (2048, 8);
cutils_allocator_t scratch_allocator = arena_allocator (scratch);

string_t *line = string_create_with_allocator (0, &scratch_allocator);
/* ... */
arena_clear (scratch); /* releases line and everything else in scratch */
```

**a static pool of your own**, to give one container its own buffer:

```c
static uint8_t log_memory[1024];
cutils_static_pool_t log_pool;
cutils_static_pool_init (&log_pool, log_memory, sizeof log_memory, 8);

cutils_allocator_t log_allocator = cutils_static_pool_allocator (&log_pool);
```

or your own, by filling in a `cutils_allocator_t` with `allocate` and `deallocate` functions.

## a few more examples

ranges of keys with a map:

```c
// every key in [15, 55)
int low = 15;
int key;
int value;
for (map_iterator_t it = map_lower_bound (scores, &low);
     map_iterator_get (&it, &key, &value) && key < 55;
     map_iterator_next (&it))
  {
    printf ("%d: %d\n", key, value);
  }
```

string keys in a hashmap need a hash and an equality function, otherwise keys are hashed and compared byte by byte:

```c
typedef struct
{
  char text[16];
} name_t;

static uint32_t
hash_name (const void *key)
{
  uint32_t hash = 5381;
  for (const char *s = ((const name_t *)key)->text; *s != '\0'; s++)
    {
      hash = hash * 33u + (unsigned char)*s;
    }
  return hash;
}

static bool
equal_names (const void *a, const void *b)
{
  return strcmp (((const name_t *)a)->text, ((const name_t *)b)->text) == 0;
}

hashmap_t *phone_book = hashmap_create (sizeof (name_t), sizeof (int),
                                        hash_name, equal_names);
```

formatted strings, checked by the compiler like `printf`:

```c
string_t *message = string_from_format ("sensor %d: %.1f C", 3, 21.5);
string_append_format (message, " (%s)", "ok");
```

free slot tracking with a bitset:

```c
bitset_t *slots = bitset_create (128);
size_t slot = bitset_find_first_clear (slots);
bitset_set (slots, slot);
```

`examples/word_count.c` puts a few of these together.

### time on bare metal

bare metal builds have no clock, so timeouts never fire until the library is given one:

```c
static uint64_t
systick_ms (void)
{
  return hal_get_tick ();
}

cutils_set_time_source (systick_ms);
```

## layout

```
include/cutils/   public headers
src/              implementation, src/internal/ holds private headers
tests/            one test program per module, run with ctest
examples/         small programs using the library
scripts/          format.sh runs clang-format over the code
```
