#include "cutils/queue.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_queue (void)
{
  queue_t *q = queue_create (sizeof (int), 0);
  CHECK (q != NULL);
  if (!q)
    return;
  clobber_stack ();
  int x;
  for (int round = 0; round < 3; round++)
    {
      for (int i = 0; i < 20; i++)
        CHECK (queue_enqueue (q, &i));
      for (int i = 0; i < 15; i++)
        CHECK (queue_dequeue (q, &x) && x == i);
      for (int i = 15; i < 20; i++)
        CHECK (queue_dequeue (q, &x) && x == i);
    }
  queue_destroy (q);
}

int
main (void)
{
  test_queue ();
  return test_result ("queue");
}
