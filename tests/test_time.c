#include "cutils/time.h"
#include "cutils/vector.h"
#include "test.h"

static uint64_t fake_now = 1000;

static uint64_t
fixed_clock (void)
{
  return fake_now;
}

// every read is 50ms later, so anything that reads the clock twice times out
static uint64_t
slow_clock (void)
{
  fake_now += 50;
  return fake_now;
}

int
main (void)
{
#if CUTILS_PLATFORM_BARE_METAL
  CHECK (!cutils_timeout_expired (0, 0));
#endif

  cutils_set_time_source (fixed_clock);
  CHECK (cutils_get_current_time_ms () == 1000);
  CHECK (!cutils_timeout_expired (1000, 5));
  fake_now = 1006;
  CHECK (cutils_timeout_expired (1000, 5));

  vector_t *vec = vector_create (1, sizeof (int));
  int x = 1;
  CHECK (vector_push (vec, &x));
  cutils_set_time_source (slow_clock);
  CHECK (!vector_push (vec, &x) && vector_get_error () == VECTOR_TIMEOUT);
  cutils_set_time_source (NULL);
  vector_destroy (vec);

  return test_result ("time");
}
