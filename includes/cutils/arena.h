#ifndef CUTILS_ARENA_H
#define CUTILS_ARENA_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *data;
  size_t size;
  size_t used;
  size_t alignment;
  cutils_allocator_t *allocator;
} arena_t;

typedef enum
{
  ARENA_OK = 0,
  ARENA_NULL_PTR = 1,
  ARENA_NO_MEMORY = 2,
  ARENA_INVALID_ARG = 3,
  ARENA_OVERFLOW = 4,
  ARENA_TIMEOUT = 5,
  ARENA_ALIGNMENT_ERROR = 6
} arena_result_t;

/**
 * Creates a new arena with the specified allocator.
 *
 * @param size Size of the arena in bytes
 * @param alignment Alignment requirement in bytes
 * @param allocator Allocator to use
 * @return Newly allocated arena or NULL on error
 */
arena_t *arena_create_with_allocator (size_t size, size_t alignment,
                                      cutils_allocator_t *allocator);

/**
 * Creates a new arena using the default allocator.
 *
 * @param size Size of the arena in bytes
 * @param alignment Alignment requirement in bytes
 * @return Newly allocated arena or NULL on error
 */
arena_t *arena_create (size_t size, size_t alignment);

/**
 * Destroys arena and frees all allocated memory.
 *
 * @param arena Arena to destroy
 */
void arena_destroy (arena_t *arena);

/**
 * Allocates memory from the arena with timeout.
 *
 * @param arena Arena to allocate from
 * @param size Size of memory to allocate
 * @param timeout_ms Timeout in milliseconds
 * @return Pointer to allocated memory or NULL on error
 */
void *arena_alloc_timeout (arena_t *arena, size_t size, uint32_t timeout_ms);

/**
 * Allocates memory from the arena.
 *
 * @param arena Arena to allocate from
 * @param size Size of memory to allocate
 * @return Pointer to allocated memory or NULL on error
 */
void *arena_alloc (arena_t *arena, size_t size);

/**
 * Gets the total size of the arena.
 *
 * @param arena Arena to get size from
 * @return Total size in bytes
 */
size_t arena_size (const arena_t *arena);

/**
 * Gets the used size of the arena.
 *
 * @param arena Arena to get used size from
 * @return Used size in bytes
 */
size_t arena_used (const arena_t *arena);

/**
 * Gets the free size of the arena.
 *
 * @param arena Arena to get free size from
 * @return Free size in bytes
 */
size_t arena_free (const arena_t *arena);

/**
 * Checks if the arena is empty.
 *
 * @param arena Arena to check
 * @return true if empty, false otherwise
 */
bool arena_is_empty (const arena_t *arena);

/**
 * Checks if the arena is full.
 *
 * @param arena Arena to check
 * @return true if full, false otherwise
 */
bool arena_is_full (const arena_t *arena);

/**
 * Clears the arena.
 *
 * @param arena Arena to clear
 * @return true if successful, false otherwise
 */
bool arena_clear (arena_t *arena);

/**
 * Gets the memory usage of the arena.
 *
 * @param arena Arena to get memory usage from
 * @return Memory usage in bytes
 */
size_t arena_memory_usage (const arena_t *arena);

/**
 * Checks if an allocation would succeed without actually performing it.
 *
 * @param arena Arena to check
 * @param required_size Required size
 * @return true if allocation would succeed, false otherwise
 */
bool arena_can_alloc (const arena_t *arena, size_t required_size);

/**
 * Gets the last arena operation error.
 *
 * @return Last error code
 */
arena_result_t arena_get_error (void);

#endif // CUTILS_ARENA_H
