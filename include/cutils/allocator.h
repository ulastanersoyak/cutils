#ifndef CUTILS_ALLOCATOR_H
#define CUTILS_ALLOCATOR_H

#include "cutils/config.h"
#include <stdbool.h>
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

/* Allocator that hands out memory from pool. Works in either allocation
   mode, so a container can use its own buffer instead of the default one.
   The pool must outlive every container using it */
cutils_allocator_t cutils_static_pool_allocator (cutils_static_pool_t *pool);

/* Default allocator based on configuration. The returned pointer stays
   valid for the lifetime of the program, so it can be stored in containers */
cutils_allocator_t *cutils_default_allocator (void);

/* Copy of the default allocator. Don't store a pointer to the copy in a
   container that outlives it, use cutils_default_allocator instead */
cutils_allocator_t cutils_create_default_allocator (void);

/* Allocate memory with alignment */
void *cutils_allocate_aligned (cutils_allocator_t *allocator, size_t size,
                               size_t alignment);

/* Deallocate memory */
void cutils_deallocate (cutils_allocator_t *allocator, void *ptr);

/* Move an allocation of old_size bytes into a new one of new_size bytes,
   copying the bytes both have in common. ptr may be NULL. On failure NULL is
   returned and ptr is left untouched */
void *cutils_reallocate (cutils_allocator_t *allocator, void *ptr,
                         size_t old_size, size_t new_size, size_t alignment);

/* Get memory usage statistics */
size_t cutils_get_memory_usage (cutils_allocator_t *allocator);

/* Check if allocation would succeed */
bool cutils_can_allocate (cutils_allocator_t *allocator, size_t size,
                          size_t alignment);

/* Reset allocator (for static pools) */
void cutils_reset_allocator (cutils_allocator_t *allocator);

#endif /* CUTILS_ALLOCATOR_H */