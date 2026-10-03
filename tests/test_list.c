#include "cutils/list.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_list (void)
{
  list_t *l = list_create (sizeof (int));
  CHECK (l != NULL);
  clobber_stack ();
  for (int i = 0; i < 10; i++)
    CHECK (list_push_back (l, &i));
  int x = 99;
  CHECK (list_insert (l, 5, &x));
  CHECK (list_get (l, 5, &x) && x == 99);
  CHECK (list_get (l, 9, &x) && x == 8);
  CHECK (list_remove (l, 5, &x) && x == 99);
  list_t *c = list_copy (l);
  CHECK (c && list_length (c) == 10);
  CHECK (list_get (c, 9, &x) && x == 9);
  list_destroy (c);
  // every index, from both ends
  for (int i = 0; i < 10; i++)
    CHECK (list_get (l, (size_t)i, &x) && x == i);
  int y = 77;
  CHECK (list_set (l, 8, &y) && list_get (l, 8, &x) && x == 77);
  CHECK (list_insert (l, 7, &y) && list_get (l, 7, &x) && x == 77);
  CHECK (list_get (l, 10, &x) && x == 9);
  CHECK (list_remove (l, 8, &x) && x == 7);
  CHECK (list_length (l) == 10);
  list_destroy (l);
}

int
main (void)
{
  test_list ();
  return test_result ("list");
}
