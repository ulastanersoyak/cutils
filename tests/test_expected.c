#include "cutils/expected.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_expected (void)
{
  int v = 42;
  expected_t *e = expected_from_data (&v, sizeof v);
  CHECK (e && *(int *)expected_get_data (e) == 42);
  clobber_stack ();
  expected_destroy (e);
  expected_t *z = expected_from_data (&v, 0);
  CHECK (z != NULL);
  expected_destroy (z);
  expected_t *err = expected_from_error (EXPECTED_TIMEOUT);
  CHECK (err && expected_has_error (err) && !expected_get_data (err));
  expected_destroy (err);
}

int
main (void)
{
  test_expected ();
  return test_result ("expected");
}
