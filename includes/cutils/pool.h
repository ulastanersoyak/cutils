#ifndef CUTILS_POOL_H
#define CUTILS_POOL_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Fixed size block pool.
 *
 * Splits a caller supplied buffer into equally sized blocks. Allocating and
 * freeing a block are O(1) and never fragment, which makes it a good fit for
 * list and map nodes on systems without a heap. Unlike the static pool,
 * freed blocks are reused.
 */
typedef struct
{
  uint8_t *blocks;   /* first block, aligned to CUTILS_ALIGNMENT */
  size_t block_size; /* bytes per block, after rounding */
  size_t block_count;
  size_t used;
  void *free_list; /* singly linked through the free blocks themselves */
} cutils_block_pool_t;

/**
 * Initializes a pool over memory.
 *
 * @param pool Pool to initialize
 * @param memory Buffer the blocks are carved from
 * @param memory_size Size of memory in bytes
 * @param block_size Requested block size, rounded up so every block can hold
 *                   a pointer and stays aligned to CUTILS_ALIGNMENT
 * @return true if at least one block fits, false otherwise
 */
bool cutils_block_pool_init (cutils_block_pool_t *pool, void *memory,
                             size_t memory_size, size_t block_size);

/**
 * Takes a block from the pool.
 *
 * @param pool Pool to allocate from
 * @return Pointer to a block of pool->block_size bytes, or NULL if the pool
 *         is empty
 */
void *cutils_block_pool_alloc (cutils_block_pool_t *pool);

/**
 * Returns a block to the pool. Pointers that don't belong to the pool are
 * ignored.
 *
 * @param pool Pool the block came from
 * @param block Block to return, may be NULL
 */
void cutils_block_pool_free (cutils_block_pool_t *pool, void *block);

/**
 * Gets the number of blocks still available.
 *
 * @param pool Pool to query
 * @return Free blocks
 */
size_t cutils_block_pool_available (const cutils_block_pool_t *pool);

/**
 * Wraps the pool in the generic allocator interface so containers can use
 * it. Requests larger than a block, or needing more alignment than
 * CUTILS_ALIGNMENT, fail.
 *
 * @param pool Pool to wrap, must outlive every container using it
 * @return Allocator backed by the pool
 */
cutils_allocator_t cutils_block_pool_allocator (cutils_block_pool_t *pool);

#endif /* CUTILS_POOL_H */
