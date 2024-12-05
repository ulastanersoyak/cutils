#include "cutils/sort.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static sort_result_t last_error = SORT_OK;

// utility function to set error and return false
static bool
set_error_and_return (sort_result_t error)
{
  last_error = error;
  return false;
}

// utility function to validate common parameters
static bool
validate_params (const void *array, size_t length, size_t elem_size,
                 sort_compare_fn compare)
{
  if (!array || !compare)
    return set_error_and_return (SORT_NULL_PTR);
  if (length == 0 || elem_size == 0)
    return set_error_and_return (SORT_INVALID_ARG);
  return true;
}

// Utility function for swapping elements
static void
swap_elements (char *a, char *b, size_t elem_size)
{
  char temp;
  for (size_t i = 0; i < elem_size; i++)
    {
      temp = a[i];
      a[i] = b[i];
      b[i] = temp;
    }
}

sort_result_t
sort_get_error (void)
{
  return last_error;
}

static void
quicksort_recursive (char *array, size_t length, size_t elem_size,
                     sort_compare_fn compare)
{
  if (length <= 1)
    return;

  // use middle element as pivot
  size_t pivot_idx = length / 2;
  char *pivot = array + pivot_idx * elem_size;

  swap_elements (pivot, array + (length - 1) * elem_size, elem_size);

  size_t store_idx = 0;
  for (size_t i = 0; i < length - 1; i++)
    {
      if (compare (array + i * elem_size, array + (length - 1) * elem_size)
          <= 0)
        {
          if (i != store_idx)
            {
              swap_elements (array + store_idx * elem_size,
                             array + i * elem_size, elem_size);
            }
          store_idx++;
        }
    }

  swap_elements (array + store_idx * elem_size,
                 array + (length - 1) * elem_size, elem_size);

  quicksort_recursive (array, store_idx, elem_size, compare);
  quicksort_recursive (array + (store_idx + 1) * elem_size,
                       length - (store_idx + 1), elem_size, compare);
}

bool
sort_quick (void *array, size_t length, size_t elem_size,
            sort_compare_fn compare)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  last_error = SORT_OK;
  quicksort_recursive (array, length, elem_size, compare);
  return true;
}

// MergeSort implementation
static bool
merge_arrays (char *array, size_t left, size_t mid, size_t right,
              size_t elem_size, sort_compare_fn compare, char *temp)
{
  size_t i = left;
  size_t j = mid;
  size_t k = 0;

  while (i < mid && j < right)
    {
      if (compare (array + i * elem_size, array + j * elem_size) <= 0)
        {
          memcpy (temp + k * elem_size, array + i * elem_size, elem_size);
          i++;
        }
      else
        {
          memcpy (temp + k * elem_size, array + j * elem_size, elem_size);
          j++;
        }
      k++;
    }

  while (i < mid)
    {
      memcpy (temp + k * elem_size, array + i * elem_size, elem_size);
      i++;
      k++;
    }

  while (j < right)
    {
      memcpy (temp + k * elem_size, array + j * elem_size, elem_size);
      j++;
      k++;
    }

  memcpy (array + left * elem_size, temp, k * elem_size);
  return true;
}

static bool
mergesort_recursive (char *array, size_t length, size_t elem_size,
                     sort_compare_fn compare, char *temp)
{
  if (length <= 1)
    return true;

  size_t mid = length / 2;

  if (!mergesort_recursive (array, mid, elem_size, compare, temp))
    return false;
  if (!mergesort_recursive (array + mid * elem_size, length - mid, elem_size,
                            compare, temp))
    return false;
  if (!merge_arrays (array, 0, mid, length, elem_size, compare, temp))
    return false;

  return true;
}

bool
sort_merge (void *array, size_t length, size_t elem_size,
            sort_compare_fn compare)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  char *temp = malloc (length * elem_size);
  if (!temp)
    return set_error_and_return (SORT_NO_MEMORY);

  last_error = SORT_OK;
  bool result = mergesort_recursive (array, length, elem_size, compare, temp);
  free (temp);

  return result;
}

static void
heapify (char *array, size_t length, size_t elem_size, sort_compare_fn compare,
         size_t root)
{
  size_t largest = root;
  size_t left = 2 * root + 1;
  size_t right = 2 * root + 2;

  if (left < length
      && compare (array + left * elem_size, array + largest * elem_size) > 0)
    largest = left;

  if (right < length
      && compare (array + right * elem_size, array + largest * elem_size) > 0)
    largest = right;

  if (largest != root)
    {
      swap_elements (array + root * elem_size, array + largest * elem_size,
                     elem_size);
      heapify (array, length, elem_size, compare, largest);
    }
}

bool
sort_heap (void *array, size_t length, size_t elem_size,
           sort_compare_fn compare)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  last_error = SORT_OK;
  char *arr = array;

  for (int64_t i = (int64_t)(length / 2 - 1); i >= 0; i--)
    heapify (arr, length, elem_size, compare, (size_t)i);

  for (size_t i = length - 1; i > 0; i--)
    {
      swap_elements (arr, arr + i * elem_size, elem_size);

      heapify (arr, i, elem_size, compare, 0);
    }

  return true;
}

bool
sort_insertion (void *array, size_t length, size_t elem_size,
                sort_compare_fn compare)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  last_error = SORT_OK;
  char *arr = array;
  char *key = malloc (elem_size);
  if (!key)
    return set_error_and_return (SORT_NO_MEMORY);

  for (size_t i = 1; i < length; i++)
    {
      memcpy (key, arr + i * elem_size, elem_size);
      int64_t j = (int64_t)(i - 1);

      while (j >= 0 && compare (arr + (size_t)j * elem_size, key) > 0)
        {
          memcpy (arr + (size_t)(j + 1) * elem_size,
                  arr + (size_t)j * elem_size, elem_size);
          j--;
        }

      memcpy (arr + (size_t)(j + 1) * elem_size, key, elem_size);
    }

  free (key);
  return true;
}

bool
sort_shell (void *array, size_t length, size_t elem_size,
            sort_compare_fn compare)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  last_error = SORT_OK;
  char *arr = array;
  char *temp = malloc (elem_size);
  if (!temp)
    return set_error_and_return (SORT_NO_MEMORY);

  // using Knuth's sequence
  size_t gap = 1;
  while (gap < length / 3)
    gap = gap * 3 + 1;

  while (gap > 0)
    {
      for (size_t i = gap; i < length; i++)
        {
          memcpy (temp, arr + i * elem_size, elem_size);
          size_t j = i;

          while (j >= gap && compare (arr + (j - gap) * elem_size, temp) > 0)
            {
              memcpy (arr + j * elem_size, arr + (j - gap) * elem_size,
                      elem_size);
              j -= gap;
            }

          memcpy (arr + j * elem_size, temp, elem_size);
        }
      gap /= 3;
    }

  free (temp);
  return true;
}

bool
sort_array (void *array, size_t length, size_t elem_size,
            sort_compare_fn compare, sort_algorithm_t algorithm)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  switch (algorithm)
    {
    case SORT_ALGORITHM_QUICK:
      return sort_quick (array, length, elem_size, compare);
    case SORT_ALGORITHM_MERGE:
      return sort_merge (array, length, elem_size, compare);
    case SORT_ALGORITHM_HEAP:
      return sort_heap (array, length, elem_size, compare);
    case SORT_ALGORITHM_INSERTION:
      return sort_insertion (array, length, elem_size, compare);
    case SORT_ALGORITHM_SHELL:
      return sort_shell (array, length, elem_size, compare);
    default:
      return set_error_and_return (SORT_INVALID_ARG);
    }
}

bool
sort_is_sorted (const void *array, size_t length, size_t elem_size,
                sort_compare_fn compare)
{
  if (!validate_params (array, length, elem_size, compare))
    return false;

  last_error = SORT_OK;
  const char *arr = array;

  for (size_t i = 0; i < length - 1; i++)
    {
      if (compare (arr + i * elem_size, arr + (i + 1) * elem_size) > 0)
        return false;
    }

  return true;
}

// common comparison functions
int
sort_compare_int (const void *a, const void *b)
{
  int va = *(const int *)a;
  int vb = *(const int *)b;
  return (va > vb) - (va < vb);
}

int
sort_compare_uint (const void *a, const void *b)
{
  unsigned int va = *(const unsigned int *)a;
  unsigned int vb = *(const unsigned int *)b;
  return (va > vb) - (va < vb);
}

int
sort_compare_float (const void *a, const void *b)
{
  float va = *(const float *)a;
  float vb = *(const float *)b;
  return (va > vb) - (va < vb);
}

int
sort_compare_double (const void *a, const void *b)
{
  double va = *(const double *)a;
  double vb = *(const double *)b;
  return (va > vb) - (va < vb);
}

int
sort_compare_string (const void *a, const void *b)
{
  const char *const *str_a = a; // note the double pointer
  const char *const *str_b = b;
  return strcmp (*str_a, *str_b);
}
