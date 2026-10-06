#include "cutils/string.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_string (void)
{
  string_t *s = string_create (0);
  CHECK (s != NULL);
  CHECK (string_cstr (s) && strcmp (string_cstr (s), "") == 0);
  CHECK (string_find (s, "x") == SIZE_MAX);
  CHECK (string_append (s, "hello"));
  CHECK (string_append_char (s, '!'));
  CHECK (strcmp (string_cstr (s), "hello!") == 0);
  string_t *t = string_create (16);
  CHECK (t && strcmp (string_cstr (t), "") == 0);
  CHECK (string_compare_cstr (t, "") == 0);
  CHECK (string_find (s, "llo") == 2);
  string_t *sub = string_substring (s, 1, 3);
  CHECK (sub && strcmp (string_cstr (sub), "ell") == 0);
  string_t *e = string_substring (s, 6, 10);
  CHECK (e && string_length (e) == 0 && strcmp (string_cstr (e), "") == 0);
  string_destroy (e);
  CHECK (string_substring (s, 7, 1) == NULL);
  string_t *big = string_create (0);
  for (int i = 0; i < 500; i++)
    CHECK (string_append_char (big, (char)('a' + i % 26)));
  CHECK (string_length (big) == 500 && string_cstr (big)[500] == '\0');
  CHECK (string_cstr (big)[27] == 'b');
  string_clear (big);
  CHECK (string_is_empty (big) && strcmp (string_cstr (big), "") == 0);
  string_destroy (big);
  CHECK (string_starts_with (s, "hel") && string_starts_with (s, ""));
  CHECK (!string_starts_with (s, "hello!!") && !string_starts_with (s, "e"));
  CHECK (string_ends_with (s, "lo!") && string_ends_with (s, "hello!"));
  CHECK (!string_ends_with (s, "xhello!") && !string_ends_with (s, "lo"));
  string_t *cp = string_copy (s);
  CHECK (cp && string_compare (cp, s) == 0);
  CHECK (string_append (cp, "?") && string_compare (cp, s) != 0);
  CHECK (string_reserve (cp, 200) && string_capacity (cp) >= 201);
  CHECK (strcmp (string_cstr (cp), "hello!?") == 0);
  string_destroy (cp);
  clobber_stack ();
  string_destroy (sub);
  string_destroy (t);
  string_destroy (s);
}

static void
test_format (void)
{
  string_t *str = string_from_format ("%d-%s", 42, "x");
  CHECK (str != NULL && strcmp (string_cstr (str), "42-x") == 0);
  CHECK (string_append_format (str, " %05.1f|%c", 3.14159, 'z'));
  CHECK (strcmp (string_cstr (str), "42-x 003.1|z") == 0);

  // long enough to grow the buffer several times
  for (int i = 0; i < 100; i++)
    {
      CHECK (string_append_format (str, "[%03d]", i));
    }
  CHECK (string_length (str) == 12 + 100 * 5);
  CHECK (string_ends_with (str, "[099]"));
  CHECK (!string_append_format (NULL, "x"));
  string_destroy (str);
}

int
main (void)
{
  test_string ();
  test_format ();
  return test_result ("string");
}
