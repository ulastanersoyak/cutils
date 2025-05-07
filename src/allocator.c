#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <string.h>

#if CUTILS_USE_STATIC_ALLOCATION
static uint8_t g_static_memory[CUTILS_MAX_STATIC_MEMORY];
static cutils_static_pool_t g_default_pool;
#endif

static void *
static_allocate (void *context, size_t size, size_t alignment)
{
  cutils_static_pool_t *pool = (cutils_static_pool_t *)context;

  // Align the current position
  size_t current = (size_t)(pool->memory + pool->used);
  size_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
  size_t padding = aligned - current;

  // Check if we have enough space
  if (pool->used + padding + size > pool->size)
    {
      return NULL;
    }

  // Update used space
  pool->used += padding + size;

  return (void *)aligned;
}

static void
static_deallocate (void *context, void *ptr)
{
  // Static allocation doesn't support deallocation
  (void)context;
  (void)ptr;
}

[[maybe_unused]] static void *
dynamic_allocate ([[maybe_unused]] void *context, [[maybe_unused]] size_t size,
                  [[maybe_unused]] size_t alignment)
{
  (void)context;
#if CUTILS_USE_DYNAMIC_ALLOCATION
  return aligned_alloc (alignment, size);
#else
  return NULL;
#endif
}

[[maybe_unused]] static void
dynamic_deallocate (void *context, [[maybe_unused]] void *ptr)
{
  (void)context;
#if CUTILS_USE_DYNAMIC_ALLOCATION
  free (ptr);
#endif
}

void
cutils_static_pool_init (cutils_static_pool_t *pool, void *memory, size_t size,
                         size_t alignment)
{
  pool->memory = (uint8_t *)memory;
  pool->size = size;
  pool->used = 0;
  pool->alignment = alignment;
}

cutils_allocator_t
cutils_create_default_allocator (void)
{
  cutils_allocator_t allocator;

#if CUTILS_USE_STATIC_ALLOCATION
  if (g_default_pool.memory == NULL)
    {
      cutils_static_pool_init (&g_default_pool, g_static_memory,
                               CUTILS_MAX_STATIC_MEMORY, CUTILS_ALIGNMENT);
    }
  allocator.context = &g_default_pool;
  allocator.allocate = static_allocate;
  allocator.deallocate = static_deallocate;
#else
  allocator.context = NULL;
  allocator.allocate = dynamic_allocate;
  allocator.deallocate = dynamic_deallocate;
#endif

  return allocator;
}

void *
cutils_allocate_aligned (cutils_allocator_t *allocator, size_t size,
                         size_t alignment)
{
  if (allocator == NULL || size == 0)
    {
      return NULL;
    }

  return allocator->allocate (allocator->context, size, alignment);
}

void
cutils_deallocate (cutils_allocator_t *allocator, void *ptr)
{
  if (allocator == NULL || ptr == NULL)
    {
      return;
    }

  allocator->deallocate (allocator->context, ptr);
}

size_t
cutils_get_memory_usage (cutils_allocator_t *allocator)
{
  if (allocator == NULL)
    {
      return 0;
    }

#if CUTILS_USE_STATIC_ALLOCATION
  if (allocator->context)
    {
      cutils_static_pool_t *pool = (cutils_static_pool_t *)allocator->context;
      return pool->used;
    }
#endif

  return 0;
}

bool
cutils_can_allocate (cutils_allocator_t *allocator, size_t size,
                     size_t alignment)
{
  if (allocator == NULL || size == 0)
    {
      return false;
    }

#if CUTILS_USE_STATIC_ALLOCATION
  if (allocator->context)
    {
      cutils_static_pool_t *pool = (cutils_static_pool_t *)allocator->context;
      size_t current = (size_t)(pool->memory + pool->used);
      size_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
      size_t padding = aligned - current;
      return (pool->used + padding + size <= pool->size);
    }
#endif

  return true;
}

void
cutils_reset_allocator (cutils_allocator_t *allocator)
{
  if (allocator == NULL)
    {
      return;
    }

#if CUTILS_USE_STATIC_ALLOCATION
  if (allocator->context)
    {
      cutils_static_pool_t *pool = (cutils_static_pool_t *)allocator->context;
      pool->used = 0;
    }
#endif
}