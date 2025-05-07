#ifndef CUTILS_ALLOCATOR_H
#define CUTILS_ALLOCATOR_H

#include "cutils/config.h"
#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *(*allocate) (void *context, size_t size, size_t alignment);
  void (*deallocate) (void *context, void *ptr);
  void *context;
} cutils_allocator_t;

/* Static memory pool */
typedef struct
{
  uint8_t *memory;
  size_t size;
  size_t used;
  size_t alignment;
} cutils_static_pool_t;

/* Initialize static memory pool */
void cutils_static_pool_init (cutils_static_pool_t *pool, void *memory,
                              size_t size, size_t alignment);

/* Create default allocator based on configuration */
cutils_allocator_t cutils_create_default_allocator (void);

/* Allocate memory with alignment */
void *cutils_allocate_aligned (cutils_allocator_t *allocator, size_t size,
                               size_t alignment);

/* Deallocate memory */
void cutils_deallocate (cutils_allocator_t *allocator, void *ptr);

/* Get memory usage statistics */
size_t cutils_get_memory_usage (cutils_allocator_t *allocator);

/* Check if allocation would succeed */
bool cutils_can_allocate (cutils_allocator_t *allocator, size_t size,
                          size_t alignment);

/* Reset allocator (for static pools) */
void cutils_reset_allocator (cutils_allocator_t *allocator);

#endif /* CUTILS_ALLOCATOR_H */