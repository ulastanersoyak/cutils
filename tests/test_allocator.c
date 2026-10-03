#include "cutils/allocator.h"
#include "test.h"
#include <stdint.h>
#include <string.h>

static void *
counting_allocate (void *context, size_t size, size_t alignment)
{
  static char buffer[256];
  (void)alignment;
  *(size_t *)context += size;
  return buffer;
}

static void
counting_deallocate (void *context, void *ptr)
{
  (void)context;
  (void)ptr;
}

static void
test_static_pool (void)
{
  static uint8_t memory[100];
  cutils_static_pool_t pool;
  cutils_static_pool_init (&pool, memory, sizeof memory, 8);
  cutils_allocator_t allocator = cutils_static_pool_allocator (&pool);

  void *p = cutils_allocate_aligned (&allocator, 10, 0);
  CHECK (p != NULL && (uintptr_t)p % CUTILS_ALIGNMENT == 0);
  CHECK (cutils_allocate_aligned (&allocator, 10, 6) == NULL);
  CHECK (cutils_allocate_aligned (&allocator, SIZE_MAX - 3, 8) == NULL);
  CHECK (!cutils_can_allocate (&allocator, SIZE_MAX, 8));
  CHECK (cutils_get_memory_usage (&allocator) >= 10);

  void *q = cutils_allocate_aligned (&allocator, 1, 64);
  CHECK (q != NULL && (uintptr_t)q % 64 == 0);

  cutils_reset_allocator (&allocator);
  CHECK (cutils_get_memory_usage (&allocator) == 0);
}

static void
test_reallocate (void)
{
  static uint8_t memory[256];
  cutils_static_pool_t pool;
  cutils_static_pool_init (&pool, memory, sizeof memory, 8);
  cutils_allocator_t allocator = cutils_static_pool_allocator (&pool);

  char *p = cutils_allocate_aligned (&allocator, 4, 0);
  memcpy (p, "abc", 4);
  char *q = cutils_reallocate (&allocator, p, 4, 32, 0);
  CHECK (q != NULL && strcmp (q, "abc") == 0);
  CHECK (cutils_reallocate (&allocator, q, 32, 4096, 0) == NULL);
  CHECK (strcmp (q, "abc") == 0);
}

static void
test_custom_allocator (void)
{
  // statistics must not treat a custom context as a static pool
  size_t counter = 0;
  cutils_allocator_t custom
      = { counting_allocate, counting_deallocate, &counter };
  CHECK (cutils_get_memory_usage (&custom) == 0);
  cutils_reset_allocator (&custom);
  CHECK (counter == 0);
  CHECK (cutils_allocate_aligned (&custom, 5, 0) != NULL && counter == 5);
}

int
main (void)
{
  test_static_pool ();
  test_reallocate ();
  test_custom_allocator ();
  return test_result ("allocator");
}
