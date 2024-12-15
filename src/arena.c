#include "cutils/arena.h"

#include <stdint.h>
#include <stdlib.h>

static arena_result_t g_last_error = ARENA_OK;

static arena_block_t *
allocate_block (arena_t *arena)
{
  arena_block_t *new_block = (arena_block_t *)malloc (sizeof (arena_block_t));
  if (new_block == NULL)
    {
      g_last_error = ARENA_NO_MEMORY;
      return NULL;
    }

  new_block->data = malloc (arena->block_size);
  if (new_block->data == NULL)
    {
      free (new_block);
      g_last_error = ARENA_NO_MEMORY;
      return NULL;
    }

  new_block->size = arena->block_size;
  new_block->used = 0;
  new_block->next = NULL;

  return new_block;
}

[[nodiscard]] arena_result_t
arena_get_error (void)
{
  return g_last_error;
}

[[nodiscard]] arena_t *
arena_create (size_t block_size)
{
  g_last_error = ARENA_OK;

  if (block_size == 0)
    {
      g_last_error = ARENA_INVALID_ARG;
      return NULL;
    }

  arena_t *arena = (arena_t *)malloc (sizeof (arena_t));

  if (arena == NULL)
    {
      g_last_error = ARENA_NO_MEMORY;
      return NULL;
    }

  arena_block_t *block = allocate_block (arena);
  if (block == NULL)
    {
      return NULL; // error is set by allocate_block
    }

  arena->first = block;
  arena->current = block;
  arena->block_size = block_size;
  arena->total_size = 0;
  arena->total_used = 0;

  return arena;
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

  arena_block_t *current_block = arena->first;
  while (current_block != NULL)
    {
      arena_block_t *next_block = current_block->next;
      free (current_block->data);
      free (current_block);
      current_block = next_block;
    }

  free (arena);
}

[[nodiscard]] void *
arena_alloc (arena_t *arena, size_t size)
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

  if (arena->current->used + size > arena->current->size)
    {
      arena_block_t *new_block = allocate_block (arena);
      if (new_block == NULL)
        {
          return NULL; // error is set by allocate_block
        }

      arena->current->next = new_block;
      arena->current = new_block;
    }

  void *ptr = (char *)arena->current->data + arena->current->used;
  arena->current->used += size;
  arena->total_used += size;
  return ptr;
}

[[nodiscard]] void *
arena_alloc_aligned (arena_t *arena, size_t size, size_t alignment)
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

  if (alignment == 0 || (alignment & (alignment - 1)) != 0) // check if pow of
                                                            // 2
    {
      g_last_error = ARENA_INVALID_ARG;
      return NULL;
    }

  uintptr_t current = (uintptr_t)arena->current->data + arena->current->used;
  uintptr_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
  size_t padding = aligned - current;

  if (arena->current->used + padding + size > arena->current->size)
    {
      arena_block_t *new_block = allocate_block (arena);
      if (new_block == NULL)
        {
          return NULL; // error is set by allocate_block
        }
      arena->current->next = new_block;
      arena->current = new_block;

      current = (uintptr_t)arena->current->data;
      aligned = (current + (alignment - 1)) & ~(alignment - 1);
      padding = aligned - current;
    }

  arena->current->used += padding;

  void *ptr = (char *)arena->current->data + arena->current->used;
  arena->current->used += size;
  arena->total_used += size;

  return ptr;
}

bool
arena_reset (arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return false;
    }

  for (arena_block_t *block = arena->first; block != NULL; block = block->next)
    {
      block->used = 0;
    }

  arena->current = arena->first;
  arena->total_used = 0;

  return true;
}

[[nodiscard]] size_t
arena_total_size (const arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }

  size_t total_size = 0;

  for (arena_block_t *block = arena->first; block != NULL; block = block->next)
    {
      total_size += block->size;
    }

  return total_size;
}

[[nodiscard]] size_t
arena_used_size (const arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }

  size_t total_size = 0;

  for (arena_block_t *block = arena->first; block != NULL; block = block->next)
    {
      total_size += block->used;
    }

  return total_size;
}

[[nodiscard]] size_t
arena_block_count (const arena_t *arena)
{
  g_last_error = ARENA_OK;

  if (arena == NULL)
    {
      g_last_error = ARENA_NULL_PTR;
      return 0;
    }

  size_t block_count = 0;

  for (arena_block_t *block = arena->first; block != NULL; block = block->next)
    {
      block_count++;
    }

  return block_count;
}
