#include "cutils/vector.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_vector (void)
{
  vector_t *v = vector_create (0, sizeof (int));
  CHECK (v != NULL);
  clobber_stack ();
  for (int i = 0; i < 100; i++)
    CHECK (vector_push (v, &i));
  CHECK (vector_length (v) == 100);
  CHECK (vector_capacity (v) >= 100);
  CHECK (vector_element_size (v) == sizeof (int));
  int x = -1;
  CHECK (vector_get (v, 57, &x) && x == 57);
  int ins = 1000;
  CHECK (vector_insert (v, 0, &ins));
  CHECK (vector_get (v, 0, &x) && x == 1000);
  CHECK (vector_get (v, 1, &x) && x == 0);
  CHECK (vector_remove (v, 0, &x) && x == 1000);
  CHECK (vector_reserve (v, 300));
  CHECK (vector_capacity (v) >= 300);
  CHECK (vector_get (v, 99, &x) && x == 99);
  CHECK (vector_shrink (v));
  CHECK (vector_capacity (v) == 100);
  CHECK (vector_get (v, 42, &x) && x == 42);
  vector_t *c = vector_copy (v);
  CHECK (c && vector_length (c) == 100);
  CHECK (vector_get (c, 7, &x) && x == 7);
  int y = 5;
  CHECK (vector_push (c, &y));
  vector_destroy (c);
  CHECK (vector_pop (v, NULL) && vector_length (v) == 99);
  int *p = vector_at (v, 10);
  CHECK (p && *p == 10);
  *p = 555;
  CHECK (vector_get (v, 10, &x) && x == 555);
  CHECK (vector_at (v, 99) == NULL
         && vector_get_error () == VECTOR_OUT_OF_RANGE);
  CHECK (((int *)vector_data (v))[3] == 3);
  CHECK (vector_swap_remove (v, 2, &x) && x == 2);
  CHECK (vector_get (v, 2, &x) && x == 98 && vector_length (v) == 98);
  CHECK (vector_swap_remove (v, 97, &x) && x == 97 && vector_length (v) == 97);
  vector_destroy (v);
  vector_destroy (NULL);
  CHECK (vector_get_error () == VECTOR_NULL_PTR);
}

int
main (void)
{
  test_vector ();
  return test_result ("vector");
}
