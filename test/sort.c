#include "cutils/sort.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_sort_integers (void);
static void test_sort_strings (void);
static void test_sort_floats (void);
static void test_sort_edge_cases (void);
static void test_sort_structs (void);
static void test_sort_is_sorted (void);

void
run_sort_tests (void)
{
  test_sort_integers ();
  test_sort_strings ();
  test_sort_floats ();
  test_sort_edge_cases ();
  test_sort_structs ();
  test_sort_is_sorted ();
}

static void
test_sort_integers (void)
{
  // Test quicksort
  int arr1[] = { 5, 2, 9, 1, 7, 6, 3 };
  size_t len = sizeof (arr1) / sizeof (arr1[0]);
  bool result = sort_quick (arr1, len, sizeof (int), sort_compare_int);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (arr1[i] <= arr1[i + 1]);
    }

  // Test mergesort
  int arr2[] = { 5, 2, 9, 1, 7, 6, 3 };
  result = sort_merge (arr2, len, sizeof (int), sort_compare_int);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (arr2[i] <= arr2[i + 1]);
    }

  // Test heapsort
  int arr3[] = { 5, 2, 9, 1, 7, 6, 3 };
  result = sort_heap (arr3, len, sizeof (int), sort_compare_int);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (arr3[i] <= arr3[i + 1]);
    }

  // Test with duplicates
  int arr4[] = { 5, 2, 5, 1, 2, 6, 2 };
  result = sort_quick (arr4, len, sizeof (int), sort_compare_int);
  assert (result == true);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (arr4[i] <= arr4[i + 1]);
    }

  TEST_PASS ("sort_integers");
}

static void
test_sort_strings (void)
{
  const char *arr[] = { "zebra", "apple", "orange", "banana" };
  size_t len = sizeof (arr) / sizeof (arr[0]);

  // test different sorting algorithms
  bool result
      = sort_quick (arr, len, sizeof (const char *), sort_compare_string);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (strcmp (arr[i], arr[i + 1]) <= 0);
    }

  // test with empty strings
  const char *empty[] = { "", "test", "", "hello", "" };
  len = sizeof (empty) / sizeof (empty[0]);
  result = sort_merge (empty, len, sizeof (const char *), sort_compare_string);
  assert (result == true);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (strcmp (empty[i], empty[i + 1]) <= 0);
    }

  TEST_PASS ("sort_strings");
}

static void
test_sort_floats (void)
{
  float arr[] = { 3.14f, 1.41f, 2.71f, 0.577f };
  size_t len = sizeof (arr) / sizeof (arr[0]);

  bool result = sort_quick (arr, len, sizeof (float), sort_compare_float);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (arr[i] <= arr[i + 1]);
    }

  TEST_PASS ("sort_floats");
}

static void
test_sort_edge_cases (void)
{
  int arr[] = { 1 };
  bool result;

  // Test NULL array
  result = sort_quick (NULL, 1, sizeof (int), sort_compare_int);
  assert (result == false);
  assert (sort_get_error () == SORT_NULL_PTR);

  // Test NULL compare function
  result = sort_quick (arr, 1, sizeof (int), NULL);
  assert (result == false);
  assert (sort_get_error () == SORT_NULL_PTR);

  // Test zero length
  result = sort_quick (arr, 0, sizeof (int), sort_compare_int);
  assert (result == false);
  assert (sort_get_error () == SORT_INVALID_ARG);

  // Test zero element size
  result = sort_quick (arr, 1, 0, sort_compare_int);
  assert (result == false);
  assert (sort_get_error () == SORT_INVALID_ARG);

  // Test single element
  result = sort_quick (arr, 1, sizeof (int), sort_compare_int);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);

  TEST_PASS ("sort_edge_cases");
}

typedef struct
{
  int key;
  const char *value;
} test_pair_t;

static int
compare_pair (const void *a, const void *b)
{
  const test_pair_t *pa = a;
  const test_pair_t *pb = b;
  return (pa->key > pb->key) - (pa->key < pb->key);
}

static void
test_sort_structs (void)
{
  test_pair_t pairs[] = {
    { 5, "five" }, { 3, "three" }, { 1, "one" }, { 4, "four" }, { 2, "two" }
  };
  size_t len = sizeof (pairs) / sizeof (pairs[0]);

  bool result = sort_quick (pairs, len, sizeof (test_pair_t), compare_pair);
  assert (result == true);
  assert (sort_get_error () == SORT_OK);
  for (size_t i = 0; i < len - 1; i++)
    {
      assert (pairs[i].key <= pairs[i + 1].key);
    }

  TEST_PASS ("sort_structs");
}

static void
test_sort_is_sorted (void)
{
  int sorted[] = { 1, 2, 3, 4, 5 };
  int unsorted[] = { 5, 2, 3, 1, 4 };

  assert (sort_is_sorted (sorted, 5, sizeof (int), sort_compare_int) == true);
  assert (sort_is_sorted (unsorted, 5, sizeof (int), sort_compare_int)
          == false);

  // Test edge cases
  assert (sort_is_sorted (NULL, 5, sizeof (int), sort_compare_int) == false);
  assert (sort_get_error () == SORT_NULL_PTR);

  assert (sort_is_sorted (sorted, 0, sizeof (int), sort_compare_int) == false);
  assert (sort_get_error () == SORT_INVALID_ARG);

  TEST_PASS ("sort_is_sorted");
}
