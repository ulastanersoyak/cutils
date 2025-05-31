#include "cutils/arena.h"
#include "cutils/config.h"
#include "cutils/time.h"

#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local arena_result_t g_last_error = ARENA_OK;

// rounds size up to a multiple of alignment and checks that it still fits in
// the space the arena has left. none of the arithmetic can wrap around
static bool
fits_aligned (const arena_t *arena, size_t size, size_t *aligned_size)
{
  size_t mask = arena->alignment - 1;
  if (size > SIZE_MAX - mask)
    {
      return false;
    }
  *aligned_size = (size + mask) & ~mask;
  return *aligned_size <= arena->size - arena->used;
}

arena_t *
arena_create_with_allocator (size_t size, size_t alignment,
                             cutils_allocator_t *allocator)
{
  g_last_error = ARENA_OK;

  if (size == 0 || alignment == 0 || allocator == NULL)
    {
      g_last_error = ARENA_INVALID_ARG;
      return NULL;
    }

  // Ensure alignment is a power of 2
  if ((alignment & (alignment - 1)) != 0)
    {
      g_last_error = ARENA_ALIGNMENT_ERROR;
      return NULL;
    }

  arena_t *arena = cutils_allocate_aligned (allocator, sizeof (arena_t),
                                            CUTILS_ALIGNMENT);
  if (arena == NULL)
    {
      g_last_error = ARENA_NO_MEMORY;
      return NULL;
    }

  arena->data = cutils_allocate_aligned (allocator, size, alignment);
  if (arena->data == NULL)
    {
      cutils_deallocate (allocator, arena);
      g_last_error = ARENA_NO_MEMORY;
      return NULL;
    }

  arena->size = size;
  arena->used = 0;
  arena->alignment = alignment;
  arena->allocator = allocator;

  return arena;
}

arena_t *
arena_create (size_t size, size_t alignment)
{
  return arena_create_with_allocator (size, alignment,
                                      cutils_default_allocator ());
}

void
arena_destroy (arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return;
    }

  cutils_deallocate (arena->allocator, arena->data);
  cutils_deallocate (arena->allocator, arena);
}

void *
arena_alloc_timeout (arena_t *arena, size_t size, uint32_t timeout_ms)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return NULL;
    }

  if (size == 0)
    {
      g_last_error = ARENA_INVALID_ARG;
      return NULL;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  size_t aligned_size;
  if (!fits_aligned (arena, size, &aligned_size))
    {
      g_last_error = ARENA_OVERFLOW;
      return NULL;
    }

  if (cutils_timeout_expired (start_time, timeout_ms))
    {
      g_last_error = ARENA_TIMEOUT;
      return NULL;
    }

  void *ptr = (char *)arena->data + arena->used;
  arena->used += aligned_size;

  return ptr;
}

void *
arena_alloc (arena_t *arena, size_t size)
{
  return arena_alloc_timeout (arena, size, CUTILS_MAX_OPERATION_TIME_MS);
}

size_t
arena_size (const arena_t *arena)
{
  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }
  return arena->size;
}

size_t
arena_used (const arena_t *arena)
{
  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }
  return arena->used;
}

size_t
arena_free (const arena_t *arena)
{
  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }
  return arena->size - arena->used;
}

bool
arena_is_empty (const arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return true;
    }

  return arena->used == 0;
}

bool
arena_is_full (const arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return true;
    }

  return arena->used == arena->size;
}

bool
arena_clear (arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return false;
    }

  arena->used = 0;
  return true;
}

size_t
arena_memory_usage (const arena_t *arena)
{
  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }
  return sizeof (arena_t) + arena->size;
}

bool
arena_can_alloc (const arena_t *arena, size_t required_size)
{
  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return false;
    }

  size_t aligned_size;
  return fits_aligned (arena, required_size, &aligned_size);
}

static void *
arena_allocate (void *context, size_t size, size_t alignment)
{
  arena_t *arena = (arena_t *)context;

  // pad to whichever alignment is stricter. both are powers of two, so the
  // padded offset stays a multiple of the arena's own alignment
  if (alignment < arena->alignment)
    {
      alignment = arena->alignment;
    }
  uintptr_t current = (uintptr_t)arena->data + arena->used;
  size_t padding
      = (size_t)((alignment - (current & (alignment - 1))) & (alignment - 1));
  if (padding > arena->size - arena->used)
    {
      g_last_error = ARENA_OVERFLOW;
      return NULL;
    }

  size_t used_before = arena->used;
  arena->used += padding;
  void *ptr = arena_alloc (arena, size);
  if (ptr == NULL)
    {
      arena->used = used_before;
    }
  return ptr;
}

static void
arena_deallocate (void *context, void *ptr)
{
  // arena memory is released all at once by arena_clear or arena_destroy
  (void)context;
  (void)ptr;
}

cutils_allocator_t
arena_allocator (arena_t *arena)
{
  cutils_allocator_t allocator;
  allocator.allocate = arena_allocate;
  allocator.deallocate = arena_deallocate;
  allocator.context = arena;
  return allocator;
}

arena_result_t
arena_get_error (void)
{
  return g_last_error;
}
