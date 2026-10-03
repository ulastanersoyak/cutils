#include "cutils/list.h"
#include "cutils/map.h"
#include "cutils/pool.h"
#include "test.h"
#include <stdint.h>

static void
test_blocks (void)
{
  static uint8_t memory[4096 + 3];
  cutils_block_pool_t pool;

  // a misaligned buffer is aligned and the block size rounded up
  CHECK (cutils_block_pool_init (&pool, memory + 3, 4096, 20));
  CHECK (pool.block_size == 24 && (uintptr_t)pool.blocks % 8 == 0);

  size_t total = cutils_block_pool_available (&pool);
  void *a = cutils_block_pool_alloc (&pool);
  void *b = cutils_block_pool_alloc (&pool);
  CHECK (a != NULL && b != NULL && a != b);
  CHECK (cutils_block_pool_available (&pool) == total - 2);

  cutils_block_pool_free (&pool, (char *)a + 1); // not a block, ignored
  CHECK (cutils_block_pool_available (&pool) == total - 2);
  cutils_block_pool_free (&pool, a);
  cutils_block_pool_free (&pool, b);
  CHECK (cutils_block_pool_available (&pool) == total);

  size_t taken = 0;
  while (cutils_block_pool_alloc (&pool) != NULL)
    {
      taken++;
    }
  CHECK (taken == total);
  CHECK (!cutils_block_pool_init (&pool, memory, 4, 64));
}

static void
test_containers_on_pool (void)
{
  static uint8_t memory[4096];
  cutils_block_pool_t pool;
  cutils_block_pool_init (&pool, memory, sizeof memory, 64);
  cutils_allocator_t allocator = cutils_block_pool_allocator (&pool);
  CHECK (cutils_allocate_aligned (&allocator, 65, 8) == NULL);

  map_t *map = map_create_with_allocator (sizeof (int), sizeof (int),
                                          compare_ints, &allocator);
  list_t *list = list_create_with_allocator (sizeof (int), &allocator);
  CHECK (map != NULL && list != NULL);

  // far more allocations than the buffer holds, so blocks must be reused
  for (int round = 0; round < 10000; round++)
    {
      int key = round % 7;
      if (!map_insert (map, &key, &round))
        {
          map_remove (map, &key, NULL);
        }
      CHECK (list_push_back (list, &round));
      if (list_length (list) > 5)
        {
          list_pop_front (list, NULL);
        }
    }

  map_destroy (map);
  list_destroy (list);
  CHECK (cutils_block_pool_available (&pool) == pool.block_count);
}

int
main (void)
{
  test_blocks ();
  test_containers_on_pool ();
  return test_result ("pool");
}
