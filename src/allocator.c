#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#if CUTILS_USE_STATIC_ALLOCATION
static uint8_t g_static_memory[CUTILS_MAX_STATIC_MEMORY];
static cutils_static_pool_t g_default_pool;
#endif

static bool
is_valid_alignment (size_t alignment)
{
  return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

// bytes needed to move the end of the pool up to the next multiple of
// alignment
static size_t
pool_padding (const cutils_static_pool_t *pool, size_t alignment)
{
  uintptr_t current = (uintptr_t)(pool->memory + pool->used);
  return (size_t)((alignment - (current & (alignment - 1))) & (alignment - 1));
}

// written so that none of the additions can wrap around
static bool
pool_fits (const cutils_static_pool_t *pool, size_t size, size_t alignment)
{
  size_t free_space = pool->size - pool->used;
  size_t padding = pool_padding (pool, alignment);
  return padding <= free_space && size <= free_space - padding;
}

static void *
static_allocate (void *context, size_t size, size_t alignment)
{
  cutils_static_pool_t *pool = (cutils_static_pool_t *)context;

  if (!pool_fits (pool, size, alignment))
    {
      return NULL;
    }

  size_t padding = pool_padding (pool, alignment);
  void *ptr = pool->memory + pool->used + padding;
  pool->used += padding + size;
  return ptr;
}

[[maybe_unused]] static void
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
  // aligned_alloc requires the size to be a multiple of the alignment
  if (size > SIZE_MAX - (alignment - 1))
    {
      return NULL;
    }
  return aligned_alloc (alignment, (size + alignment - 1) & ~(alignment - 1));
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

// containers keep a pointer to their allocator for their whole lifetime, so
// the default one has to outlive every caller's stack frame
static cutils_allocator_t g_default_allocator;
static bool g_default_allocator_ready = false;

cutils_allocator_t *
cutils_default_allocator (void)
{
  if (!g_default_allocator_ready)
    {
#if CUTILS_USE_STATIC_ALLOCATION
      cutils_static_pool_init (&g_default_pool, g_static_memory,
                               CUTILS_MAX_STATIC_MEMORY, CUTILS_ALIGNMENT);
      g_default_allocator.context = &g_default_pool;
      g_default_allocator.allocate = static_allocate;
      g_default_allocator.deallocate = static_deallocate;
#else
      g_default_allocator.context = NULL;
      g_default_allocator.allocate = dynamic_allocate;
      g_default_allocator.deallocate = dynamic_deallocate;
#endif
      g_default_allocator_ready = true;
    }
  return &g_default_allocator;
}

cutils_allocator_t
cutils_create_default_allocator (void)
{
  return *cutils_default_allocator ();
}

void *
cutils_allocate_aligned (cutils_allocator_t *allocator, size_t size,
                         size_t alignment)
{
  if (allocator == NULL || allocator->allocate == NULL || size == 0)
    {
      return NULL;
    }

  if (alignment == 0)
    {
      alignment = CUTILS_ALIGNMENT;
    }

  if (!is_valid_alignment (alignment))
    {
      return NULL;
    }

  return allocator->allocate (allocator->context, size, alignment);
}

void
cutils_deallocate (cutils_allocator_t *allocator, void *ptr)
{
  if (allocator == NULL || allocator->deallocate == NULL || ptr == NULL)
    {
      return;
    }

  allocator->deallocate (allocator->context, ptr);
}

// the statistics below only make sense for static pools. any other context
// belongs to a user supplied allocator and has an unknown layout
static cutils_static_pool_t *
as_static_pool (const cutils_allocator_t *allocator)
{
  if (allocator == NULL || allocator->allocate != static_allocate)
    {
      return NULL;
    }
  return (cutils_static_pool_t *)allocator->context;
}

size_t
cutils_get_memory_usage (cutils_allocator_t *allocator)
{
  cutils_static_pool_t *pool = as_static_pool (allocator);
  return pool != NULL ? pool->used : 0;
}

bool
cutils_can_allocate (cutils_allocator_t *allocator, size_t size,
                     size_t alignment)
{
  if (allocator == NULL || size == 0)
    {
      return false;
    }

  if (alignment == 0)
    {
      alignment = CUTILS_ALIGNMENT;
    }

  if (!is_valid_alignment (alignment))
    {
      return false;
    }

  cutils_static_pool_t *pool = as_static_pool (allocator);
  if (pool != NULL)
    {
      return pool_fits (pool, size, alignment);
    }

  // other allocators can't be asked ahead of time
  return true;
}

void
cutils_reset_allocator (cutils_allocator_t *allocator)
{
  cutils_static_pool_t *pool = as_static_pool (allocator);
  if (pool != NULL)
    {
      pool->used = 0;
    }
}
