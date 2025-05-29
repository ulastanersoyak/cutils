#include "cutils/pool.h"
#include "cutils/config.h"

#include <stdint.h>

// every block must be able to hold the free list link and keep the next
// block aligned
static size_t
round_block_size (size_t block_size)
{
  size_t unit = CUTILS_ALIGNMENT > sizeof (void *) ? CUTILS_ALIGNMENT
                                                   : sizeof (void *);
  if (block_size < unit)
    {
      return unit;
    }
  if (block_size > SIZE_MAX - (unit - 1))
    {
      return 0;
    }
  return (block_size + unit - 1) / unit * unit;
}

bool
cutils_block_pool_init (cutils_block_pool_t *pool, void *memory,
                        size_t memory_size, size_t block_size)
{
  if (pool == NULL || memory == NULL)
    {
      return false;
    }

  pool->blocks = NULL;
  pool->block_size = 0;
  pool->block_count = 0;
  pool->used = 0;
  pool->free_list = NULL;

  size_t size = round_block_size (block_size);
  uintptr_t start = (uintptr_t)memory;
  size_t padding = (size_t)((CUTILS_ALIGNMENT - (start % CUTILS_ALIGNMENT))
                            % CUTILS_ALIGNMENT);
  if (size == 0 || padding >= memory_size)
    {
      return false;
    }

  size_t count = (memory_size - padding) / size;
  if (count == 0)
    {
      return false;
    }

  pool->blocks = (uint8_t *)memory + padding;
  pool->block_size = size;
  pool->block_count = count;

  // thread the free list through the blocks, lowest address first
  for (size_t i = count; i > 0; i--)
    {
      void *block = pool->blocks + ((i - 1) * size);
      *(void **)block = pool->free_list;
      pool->free_list = block;
    }

  return true;
}

void *
cutils_block_pool_alloc (cutils_block_pool_t *pool)
{
  if (pool == NULL || pool->free_list == NULL)
    {
      return NULL;
    }

  void *block = pool->free_list;
  pool->free_list = *(void **)block;
  pool->used++;
  return block;
}

void
cutils_block_pool_free (cutils_block_pool_t *pool, void *block)
{
  if (pool == NULL || block == NULL)
    {
      return;
    }

  uintptr_t first = (uintptr_t)pool->blocks;
  uintptr_t address = (uintptr_t)block;
  if (address < first
      || address >= first + (pool->block_count * pool->block_size)
      || (address - first) % pool->block_size != 0)
    {
      return;
    }

  *(void **)block = pool->free_list;
  pool->free_list = block;
  pool->used--;
}

size_t
cutils_block_pool_available (const cutils_block_pool_t *pool)
{
  if (pool == NULL)
    {
      return 0;
    }
  return pool->block_count - pool->used;
}

static void *
pool_allocate (void *context, size_t size, size_t alignment)
{
  cutils_block_pool_t *pool = (cutils_block_pool_t *)context;
  if (size > pool->block_size || alignment > CUTILS_ALIGNMENT)
    {
      return NULL;
    }
  return cutils_block_pool_alloc (pool);
}

static void
pool_deallocate (void *context, void *ptr)
{
  cutils_block_pool_free ((cutils_block_pool_t *)context, ptr);
}

cutils_allocator_t
cutils_block_pool_allocator (cutils_block_pool_t *pool)
{
  cutils_allocator_t allocator;
  allocator.allocate = pool_allocate;
  allocator.deallocate = pool_deallocate;
  allocator.context = pool;
  return allocator;
}
