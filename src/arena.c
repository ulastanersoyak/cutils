#include "cutils/arena.h"

#include <stdlib.h>

static arena_result_t g_last_error = ARENA_OK;

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

  arena_block_t *block = (arena_block_t *)malloc (sizeof (arena_block_t));

  if (block == NULL)
    {
      g_last_error = ARENA_NO_MEMORY;
      free (arena);
      return NULL;
    }

  block->data = malloc (block_size);

  if (block->data == NULL)
    {
      g_last_error = ARENA_NO_MEMORY;
      free (arena);
      free (block);
      return NULL;
    }

  block->next = NULL;
  block->size = block_size;
  block->used = 0;

  arena->first = block;
  arena->current = block;
  arena->block_size = block_size;

  return arena;
}
