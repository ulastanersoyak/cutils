#include "cutils/string.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_string_create (void);
static void test_string_from_cstr (void);
static void test_string_copy (void);
static void test_string_move (void);
static void test_string_equals (void);
static void test_string_starts_with (void);
static void test_string_ends_with (void);
static void test_string_trim (void);
static void test_string_trim_left (void);
static void test_string_trim_right (void);
static void test_string_to_upper_lower (void);
static void test_string_substring (void);
static void test_string_concat (void);
static void test_string_find (void);
static void test_string_find_last (void);
static void test_string_contains (void);
static void test_string_reverse (void);
static void test_string_to_from_int (void);

void
run_string_tests (void)
{
  test_string_create ();
  test_string_from_cstr ();
  test_string_copy ();
  test_string_move ();
  test_string_equals ();
  test_string_starts_with ();
  test_string_ends_with ();
  test_string_trim ();
  test_string_trim_left ();
  test_string_trim_right ();
  test_string_to_upper_lower ();
  test_string_substring ();
  test_string_concat ();
  test_string_find ();
  test_string_find_last ();
  test_string_contains ();
  test_string_reverse ();
  test_string_to_from_int ();
}

static void
test_string_create (void)
{
  const char *test = "Hello, World!";
  string_t *str = string_create (test, strlen (test));
  assert (str != NULL);
  assert (str->len == strlen (test));
  assert (memcmp (str->data, test, str->len) == 0);
  assert (str->data[str->len] == '\0');
  string_destroy (str);

  // test NULL input
  str = string_create (NULL, 5);
  assert (str == NULL);
  assert (string_get_error () == STRING_NULL_PTR);

  // test overflow
  str = string_create (test, SIZE_MAX);
  assert (str == NULL);
  assert (string_get_error () == STRING_OVERFLOW);

  TEST_PASS ("string_create");
}

static void
test_string_from_cstr (void)
{
  const char *test = "Hello";
  string_t *str = string_from_cstr (test);
  assert (str != NULL);
  assert (str->len == strlen (test));
  assert (strcmp (str->data, test) == 0);
  string_destroy (str);

  // test NULL input
  str = string_from_cstr (NULL);
  assert (str == NULL);
  assert (string_get_error () == STRING_NULL_PTR);

  TEST_PASS ("string_from_cstr");
}

static void
test_string_copy (void)
{
  string_t *orig = string_from_cstr ("Test String");
  string_t *copy = string_copy (orig);

  assert (copy != NULL);
  assert (copy->len == orig->len);
  assert (strcmp (copy->data, orig->data) == 0);
  assert (copy->data != orig->data); // different memory locations

  string_destroy (copy);
  string_destroy (orig);

  // test NULL input
  copy = string_copy (NULL);
  assert (copy == NULL);
  assert (string_get_error () == STRING_NULL_PTR);

  TEST_PASS ("string_copy");
}

static void
test_string_move (void)
{
  string_t *orig = string_from_cstr ("Move Test");
  char *orig_data = orig->data;

  string_t *moved = string_move (orig);
  assert (moved != NULL);
  assert (moved->data == orig_data);
  assert (orig->data == NULL);
  assert (orig->len == 0);

  string_destroy (moved);
  string_destroy (orig);

  // test NULL input
  moved = string_move (NULL);
  assert (moved == NULL);
  assert (string_get_error () == STRING_NULL_PTR);

  TEST_PASS ("string_move");
}

static void
test_string_equals (void)
{
  string_t *str1 = string_from_cstr ("Test");
  string_t *str2 = string_from_cstr ("Test");
  string_t *str3 = string_from_cstr ("Different");

  assert (string_equals (str1, str2) == true);
  assert (string_equals (str1, str3) == false);
  assert (string_equals (str1, str1) == true); // same pointer
  assert (string_equals (str1, NULL) == false);
  assert (string_get_error () == STRING_NULL_PTR);

  string_destroy (str1);
  string_destroy (str2);
  string_destroy (str3);

  TEST_PASS ("string_equals");
}

static void
test_string_starts_with (void)
{
  string_t *str = string_from_cstr ("Hello World");
  string_t *prefix = string_from_cstr ("Hello");
  string_t *not_prefix = string_from_cstr ("World");

  assert (string_starts_with (str, prefix) == true);
  assert (string_starts_with (str, not_prefix) == false);
  assert (string_starts_with (str, NULL) == false);
  assert (string_get_error () == STRING_NULL_PTR);

  string_destroy (str);
  string_destroy (prefix);
  string_destroy (not_prefix);

  TEST_PASS ("string_starts_with");
}

static void
test_string_ends_with (void)
{
  string_t *str = string_from_cstr ("Hello World");
  string_t *suffix = string_from_cstr ("World");
  string_t *not_suffix = string_from_cstr ("Hello");

  assert (string_ends_with (str, suffix) == true);
  assert (string_ends_with (str, not_suffix) == false);
  assert (string_ends_with (str, NULL) == false);
  assert (string_get_error () == STRING_NULL_PTR);

  string_destroy (str);
  string_destroy (suffix);
  string_destroy (not_suffix);

  TEST_PASS ("string_ends_with");
}

static void
test_string_trim (void)
{
  string_t *str = string_from_cstr ("  Hello World  ");
  string_t *trimmed = string_trim (str);

  assert (trimmed != NULL);
  assert (strcmp (trimmed->data, "Hello World") == 0);

  string_destroy (trimmed);
  string_destroy (str);

  // test all whitespace
  str = string_from_cstr ("   ");
  trimmed = string_trim (str);
  assert (trimmed != NULL);
  assert (trimmed->len == 0);
  assert (trimmed->data[0] == '\0');

  string_destroy (trimmed);
  string_destroy (str);

  TEST_PASS ("string_trim");
}

static void
test_string_to_upper_lower (void)
{
  string_t *str = string_from_cstr ("Hello123");
  string_t *upper = string_to_upper (str);
  string_t *lower = string_to_lower (str);

  assert (strcmp (upper->data, "HELLO123") == 0);
  assert (strcmp (lower->data, "hello123") == 0);

  string_destroy (str);
  string_destroy (upper);
  string_destroy (lower);

  TEST_PASS ("string_to_upper_lower");
}

static void
test_string_find (void)
{
  string_t *str = string_from_cstr ("Hello World Hello");
  string_t *pattern = string_from_cstr ("Hello");

  assert (string_find (str, pattern) == 0);
  assert (string_find_last (str, pattern) == 12);

  string_destroy (str);
  string_destroy (pattern);

  TEST_PASS ("string_find");
}

static void
test_string_reverse (void)
{
  string_t *str = string_from_cstr ("Hello");
  string_t *reversed = string_reverse (str);

  assert (strcmp (reversed->data, "olleH") == 0);

  string_destroy (str);
  string_destroy (reversed);

  TEST_PASS ("string_reverse");
}

static void
test_string_to_from_int (void)
{
  int64_t value = 12345;
  string_t *str = string_from_int (value);
  bool success = false;
  int64_t result = string_to_int (str, &success);

  assert (success == true);
  assert (result == value);

  string_destroy (str);

  TEST_PASS ("string_to_from_int");
}

static void
test_string_trim_left (void)
{
  string_t *str = string_from_cstr ("  Hello World");
  string_t *trimmed = string_trim_left (str);

  assert (trimmed != NULL);
  assert (strcmp (trimmed->data, "Hello World") == 0);

  string_destroy (trimmed);
  string_destroy (str);

  // test all whitespace
  str = string_from_cstr ("   ");
  trimmed = string_trim_left (str);
  assert (trimmed != NULL);
  assert (trimmed->len == 0);
  assert (trimmed->data[0] == '\0');

  string_destroy (trimmed);
  string_destroy (str);

  // test no whitespace
  str = string_from_cstr ("Hello");
  trimmed = string_trim_left (str);
  assert (trimmed != NULL);
  assert (strcmp (trimmed->data, "Hello") == 0);

  string_destroy (trimmed);
  string_destroy (str);

  TEST_PASS ("string_trim_left");
}

static void
test_string_trim_right (void)
{
  // test normal case
  string_t *str = string_from_cstr ("Hello World  ");
  string_t *trimmed = string_trim_right (str);

  assert (trimmed != NULL);
  assert (strcmp (trimmed->data, "Hello World") == 0);

  string_destroy (trimmed);
  string_destroy (str);

  // test all whitespace
  str = string_from_cstr ("   ");
  trimmed = string_trim_right (str);
  assert (trimmed != NULL);
  assert (trimmed->len == 0); // this was failing
  assert (trimmed->data[0] == '\0');

  string_destroy (trimmed);
  string_destroy (str);

  // test no whitespace
  str = string_from_cstr ("Hello");
  trimmed = string_trim_right (str);
  assert (trimmed != NULL);
  assert (strcmp (trimmed->data, "Hello") == 0);

  string_destroy (trimmed);
  string_destroy (str);

  TEST_PASS ("string_trim_right");
}

static void
test_string_substring (void)
{
  string_t *str = string_from_cstr ("Hello World");
  string_t *sub = string_substring (str, 6, 5); // World"

  assert (sub != NULL);
  assert (strcmp (sub->data, "World") == 0);
  assert (sub->len == 5);

  string_destroy (sub);

  // test start > length
  sub = string_substring (str, str->len + 1, 1);
  assert (sub == NULL);
  assert (string_get_error () == STRING_INVALID_ARG);

  // test length > remaining chars
  sub = string_substring (str, 6, 10);
  assert (sub == NULL);
  assert (string_get_error () == STRING_INVALID_ARG);

  // test zero length
  sub = string_substring (str, 0, 0);
  assert (sub != NULL);
  assert (sub->len == 0);
  assert (sub->data[0] == '\0');

  string_destroy (sub);
  string_destroy (str);

  TEST_PASS ("string_substring");
}

static void
test_string_concat (void)
{
  string_t *str1 = string_from_cstr ("Hello ");
  string_t *str2 = string_from_cstr ("World");
  string_t *result = string_concat (str1, str2);

  assert (result != NULL);
  assert (strcmp (result->data, "Hello World") == 0);
  assert (result->len == str1->len + str2->len);

  string_destroy (result);

  // test with empty strings
  string_t *empty = string_from_cstr ("");
  result = string_concat (str1, empty);
  assert (result != NULL);
  assert (strcmp (result->data, str1->data) == 0);

  string_destroy (result);
  string_destroy (empty);

  // test NULL inputs
  result = string_concat (NULL, str2);
  assert (result == NULL);
  assert (string_get_error () == STRING_NULL_PTR);

  string_destroy (str1);
  string_destroy (str2);

  TEST_PASS ("string_concat");
}

static void
test_string_contains (void)
{
  string_t *str = string_from_cstr ("Hello World");
  string_t *substr1 = string_from_cstr ("World");
  string_t *substr2 = string_from_cstr ("Goodbye");
  string_t *empty = string_from_cstr ("");

  assert (string_contains (str, substr1) == true);
  assert (string_contains (str, substr2) == false);
  assert (string_contains (str, empty)
          == true); // empty string is always contained

  // test NULL inputs
  assert (string_contains (NULL, substr1) == false);
  assert (string_get_error () == STRING_NULL_PTR);

  assert (string_contains (str, NULL) == false);
  assert (string_get_error () == STRING_NULL_PTR);

  string_destroy (str);
  string_destroy (substr1);
  string_destroy (substr2);
  string_destroy (empty);

  TEST_PASS ("string_contains");
}

static void
test_string_find_last (void)
{
  string_t *str = string_from_cstr ("Hello World Hello");
  string_t *pattern = string_from_cstr ("Hello");
  string_t *not_found = string_from_cstr ("Goodbye");

  // test normal case
  assert (string_find_last (str, pattern) == 12);

  // test pattern not found
  assert (string_find_last (str, not_found) == SIZE_MAX);

  // test empty pattern
  string_t *empty = string_from_cstr ("");
  assert (string_find_last (str, empty) == SIZE_MAX);

  // test NULL inputs
  assert (string_find_last (NULL, pattern) == SIZE_MAX);
  assert (string_get_error () == STRING_NULL_PTR);

  assert (string_find_last (str, NULL) == SIZE_MAX);
  assert (string_get_error () == STRING_NULL_PTR);

  string_destroy (str);
  string_destroy (pattern);
  string_destroy (not_found);
  string_destroy (empty);

  // test pattern longer than string
  str = string_from_cstr ("Hi");
  pattern = string_from_cstr ("Hello");
  assert (string_find_last (str, pattern) == SIZE_MAX);

  string_destroy (str);
  string_destroy (pattern);

  TEST_PASS ("string_find_last");
}
