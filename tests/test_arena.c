#include "cutils/arena.h"
#include "cutils/string.h"
#include "cutils/vector.h"
#include "test.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void
test_arena (void)
{
  arena_t *a = arena_create (256, 8);
  CHECK (a != NULL);
  clobber_stack ();
  void *p = arena_alloc (a, 10);
  CHECK (p != NULL && arena_used (a) == 16);
  CHECK (arena_alloc (a, SIZE_MAX - 4) == NULL);
  CHECK (arena_get_error () == ARENA_OVERFLOW);
  arena_destroy (a);
}

static void
test_arena_allocator (void)
{
  arena_t *arena = arena_create (2048, 8);
  cutils_allocator_t allocator = arena_allocator (arena);

  for (int round = 0; round < 3; round++)
    {
      string_t *str = string_create_with_allocator (0, &allocator);
      vector_t *vec
          = vector_create_with_allocator (4, sizeof (int), &allocator);
      for (int i = 0; i < 40; i++)
        {
          CHECK (string_append_char (str, 'x'));
          CHECK (vector_push (vec, &i));
        }
      int x;
      CHECK (vector_get (vec, 39, &x) && x == 39 && string_length (str) == 40);
      string_destroy (str);
      vector_destroy (vec);
      CHECK (arena_used (arena) > 0);
      arena_clear (arena);
    }

  void *p = cutils_allocate_aligned (&allocator, 3, 64);
  CHECK (p != NULL && (uintptr_t)p % 64 == 0);
  size_t used = arena_used (arena);
  CHECK (cutils_allocate_aligned (&allocator, 4096, 64) == NULL);
  CHECK (arena_used (arena) == used);
  arena_destroy (arena);
}

int
main (void)
{
  test_arena ();
  test_arena_allocator ();
  return test_result ("arena");
}
