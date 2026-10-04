#include "cutils/bitset.h"
#include "test.h"
#include <stdint.h>

static void
test_single_bits (void)
{
  // 70 bits: two full words and 6 bits of a third
  bitset_t *bits = bitset_create (70);
  CHECK (bits != NULL && bitset_size (bits) == 70);
  clobber_stack ();
  CHECK (bitset_count (bits) == 0);
  CHECK (bitset_find_first_set (bits) == SIZE_MAX);
  CHECK (bitset_find_first_clear (bits) == 0);

  CHECK (bitset_set (bits, 0) && bitset_set (bits, 31)
         && bitset_set (bits, 32));
  CHECK (bitset_set (bits, 69));
  CHECK (bitset_test (bits, 31) && bitset_test (bits, 69));
  CHECK (!bitset_test (bits, 30));
  CHECK (bitset_count (bits) == 4);
  CHECK (bitset_find_first_clear (bits) == 1);

  CHECK (bitset_toggle (bits, 31) && !bitset_test (bits, 31));
  CHECK (bitset_clear (bits, 0) && bitset_find_first_set (bits) == 32);

  CHECK (!bitset_set (bits, 70));
  CHECK (bitset_get_error () == BITSET_OUT_OF_RANGE);
  CHECK (!bitset_test (NULL, 0) && bitset_get_error () == BITSET_NULL_PTR);
  bitset_destroy (bits);
}

static void
test_whole_set (void)
{
  bitset_t *bits = bitset_create (70);
  CHECK (bitset_set_all (bits));
  // the 26 unused bits of the last word must not be counted
  CHECK (bitset_count (bits) == 70);
  CHECK (bitset_find_first_clear (bits) == SIZE_MAX);
  CHECK (bitset_clear (bits, 65) && bitset_find_first_clear (bits) == 65);
  CHECK (bitset_clear_all (bits) && bitset_count (bits) == 0);
  bitset_destroy (bits);

  // a size that fills its words exactly
  bits = bitset_create (64);
  CHECK (bitset_set_all (bits) && bitset_count (bits) == 64);
  bitset_destroy (bits);

  CHECK (bitset_create (0) == NULL);
}

int
main (void)
{
  test_single_bits ();
  test_whole_set ();
  return test_result ("bitset");
}
