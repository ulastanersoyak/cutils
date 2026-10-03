#include "cutils/pqueue.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_pqueue (void)
{
  priority_queue_t *q = priority_queue_create (sizeof (int), 4, compare_ints);
  CHECK (q != NULL);
  clobber_stack ();
  // enough pushes and pops to exhaust the 64KB pool if swaps allocate
  srand (1);
  int ok = 1;
  for (int round = 0; round < 200 && ok; round++)
    {
      for (int i = 0; i < 64; i++)
        {
          int v = rand () % 1000;
          ok &= priority_queue_push (q, &v);
        }
      int prev = 1 << 30, x;
      while (priority_queue_pop (q, &x))
        {
          if (x > prev)
            ok = 0;
          prev = x;
        }
    }
  CHECK (ok);
  priority_queue_t *z = priority_queue_create (sizeof (int), 0, compare_ints);
  CHECK (z != NULL);
  priority_queue_destroy (q);
  priority_queue_destroy (z);
}

int
main (void)
{
  test_pqueue ();
  return test_result ("pqueue");
}
