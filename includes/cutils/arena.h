#ifndef CUTILS_ARENA_H
#define CUTILS_ARENA_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  ARENA_OK,
  ARENA_NULL_PTR,
  ARENA_NO_MEMORY,
  ARENA_INVALID_VALUE,
  ARENA_INVALID_ARG,
  ARENA_OUT_OF_MEMORY
} arena_result_t;

typedef struct arena_block arena_block_t;

// forward declaration of internal block structure
struct arena_block
{
  void *data;
  size_t size;
  size_t used;
  arena_block_t *next;
};

typedef struct
{
  arena_block_t *current;
  arena_block_t *first;
  size_t block_size;
  size_t total_size;
  size_t total_used;
  bool allow_resize;
} arena_t;

/**
 * Gets the last arena operation error.
 *
 * @return Last error code
 */
[[nodiscard]] arena_result_t arena_get_error (void);

/**
 * Creates a new arena with specified block size.
 *
 * @param block_size Size of each memory block
 * @return Newly allocated arena or NULL on error
 * @note Sets error to ARENA_INVALID_ARG if block_size is 0
 * @note Sets error to ARENA_NO_MEMORY if allocation fails
 */
[[nodiscard]] arena_t *arena_create (size_t block_size);

/**
 * Destroys arena and frees all allocated memory.
 *
 * @param arena Arena to destroy
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 */
void arena_destroy (arena_t *arena);

/**
 * Allocates memory from the arena.
 *
 * @param arena Arena to allocate from
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory or NULL on error
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 * @note Sets error to ARENA_INVALID_ARG if size is 0
 * @note Sets error to ARENA_OUT_OF_MEMORY if allocation fails
 */
[[nodiscard]] void *arena_alloc (arena_t *arena, size_t size);

/**
 * Allocates aligned memory from the arena.
 *
 * @param arena Arena to allocate from
 * @param size Number of bytes to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return Aligned pointer to allocated memory or NULL on error
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 * @note Sets error to ARENA_INVALID_ARG if size is 0 or alignment is invalid
 * @note Sets error to ARENA_OUT_OF_MEMORY if allocation fails
 */
[[nodiscard]] void *arena_alloc_aligned (arena_t *arena, size_t size,
                                         size_t alignment);

/**
 * Resets arena to initial state without freeing memory.
 *
 * @param arena Arena to reset
 * @return true if successful, false otherwise
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 */
bool arena_reset (arena_t *arena);

/**
 * Gets total memory currently allocated by arena.
 *
 * @param arena Arena to query
 * @return Total allocated size in bytes or 0 on error
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 */
size_t arena_total_size (const arena_t *arena);

/**
 * Gets total memory currently used in arena.
 *
 * @param arena Arena to query
 * @return Total used size in bytes or 0 on error
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 */
size_t arena_used_size (const arena_t *arena);

/**
 * Enables or disables arena auto-resizing.
 *
 * @param arena Arena to modify
 * @param allow_resize true to enable resizing, false to disable
 * @return true if successful, false otherwise
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 */
bool arena_set_resize (arena_t *arena, bool allow_resize);

/**
 * Creates a temporary scope that can be rolled back.
 *
 * @param arena Arena to create scope in
 * @return Checkpoint that can be used to restore arena state
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 */
[[nodiscard]] size_t arena_checkpoint (arena_t *arena);

/**
 * Rolls back arena to previous checkpoint.
 *
 * @param arena Arena to roll back
 * @param checkpoint Checkpoint to restore to
 * @return true if successful, false otherwise
 * @note Sets error to ARENA_NULL_PTR if arena is NULL
 * @note Sets error to ARENA_INVALID_VALUE if checkpoint is invalid
 */
bool arena_rollback (arena_t *arena, size_t checkpoint);

#endif // CUTILS_ARENA_H
