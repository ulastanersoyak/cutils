#include "cutils/vector.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_vector_create (void);
static void test_vector_copy (void);
static void test_vector_push_pop (void);
static void test_vector_get_set (void);
static void test_vector_insert_remove (void);
static void test_vector_reserve_shrink (void);
static void test_vector_clear (void);
static void test_vector_is_empty (void);
static void test_vector_back (void);

void
run_vector_tests (void)
{
  test_vector_create ();
  test_vector_copy ();
  test_vector_push_pop ();
  test_vector_get_set ();
  test_vector_insert_remove ();
  test_vector_reserve_shrink ();
  test_vector_clear ();
  test_vector_is_empty ();
  test_vector_back ();
}

static void
test_vector_create (void)
{
  // test normal creation
  vector_t *vec = vector_create (4, sizeof (int32_t));
  assert (vec != NULL);
  assert (vec->len == 0);
  assert (vec->capacity == 4);
  assert (vec->elem_len == sizeof (int32_t));
  vector_destroy (vec);

  // test zero capacity (should use default)
  vec = vector_create (0, sizeof (int32_t));
  assert (vec != NULL);
  assert (vec->capacity == VECTOR_INIT_CAPACITY);
  vector_destroy (vec);

  // test zero element size
  vec = vector_create (4, 0);
  assert (vec == NULL);
  assert (vector_get_error () == VECTOR_INVALID_ARG);

  // test overflow
  vec = vector_create (SIZE_MAX, sizeof (int32_t));
  assert (vec == NULL);
  assert (vector_get_error () == VECTOR_OVERFLOW);

  TEST_PASS ("vector_create");
}

static void
test_vector_copy (void)
{
  vector_t *orig = vector_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 };

  for (int32_t i = 0; i < 4; i++)
    {
      vector_push (orig, &values[i]);
    }

  vector_t *copy = vector_copy (orig);
  assert (copy != NULL);
  assert (copy->len == orig->len);
  assert (copy->capacity == orig->capacity);
  assert (copy->elem_len == orig->elem_len);

  // verify data independence
  int32_t val;
  for (size_t i = 0; i < orig->len; i++)
    {
      vector_get (copy, i, &val);
      assert (val == values[i]);
    }

  vector_destroy (copy);
  vector_destroy (orig);

  // test null input
  copy = vector_copy (NULL);
  assert (copy == NULL);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  TEST_PASS ("vector_copy");
}

static void
test_vector_push_pop (void)
{
  vector_t *vec = vector_create (2, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 }; // will force resize
  int32_t val;

  // test pushing
  for (int32_t i = 0; i < 4; i++)
    {
      assert (vector_push (vec, &values[i]) == true);
    }
  assert (vec->len == 4);
  assert (vec->capacity >= 4);

  // test popping
  for (int32_t i = 3; i >= 0; i--)
    {
      assert (vector_pop (vec, &val) == true);
      assert (val == values[i]);
    }
  assert (vec->len == 0);

  // test pop empty vector
  assert (vector_pop (vec, &val) == false);
  assert (vector_get_error () == VECTOR_OUT_OF_RANGE);

  // test null parameters
  assert (vector_push (NULL, &val) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);
  assert (vector_push (vec, NULL) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  vector_destroy (vec);
  TEST_PASS ("vector_push_pop");
}

static void
test_vector_get_set (void)
{
  vector_t *vec = vector_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 };
  int32_t val;

  for (int32_t i = 0; i < 4; i++)
    {
      vector_push (vec, &values[i]);
    }

  // test get
  for (size_t i = 0; i < 4; i++)
    {
      assert (vector_get (vec, i, &val) == true);
      assert (val == values[i]);
    }

  // test set
  int32_t new_val = 42;
  assert (vector_set (vec, 2, &new_val) == true);
  assert (vector_get (vec, 2, &val) == true);
  assert (val == new_val);

  // test invalid index
  assert (vector_get (vec, 4, &val) == false);
  assert (vector_get_error () == VECTOR_OUT_OF_RANGE);
  assert (vector_set (vec, 4, &val) == false);
  assert (vector_get_error () == VECTOR_OUT_OF_RANGE);

  // test null parameters
  assert (vector_get (NULL, 0, &val) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);
  assert (vector_set (NULL, 0, &val) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  vector_destroy (vec);
  TEST_PASS ("vector_get_set");
}

static void
test_vector_insert_remove (void)
{
  vector_t *vec = vector_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3 };
  int32_t val;

  for (int32_t i = 0; i < 3; i++)
    {
      vector_push (vec, &values[i]);
    }

  // test insert
  int32_t insert_val = 42;
  assert (vector_insert (vec, 1, &insert_val) == true);
  assert (vector_get (vec, 1, &val) == true);
  assert (val == insert_val);
  assert (vec->len == 4);

  // test remove
  assert (vector_remove (vec, 1, &val) == true);
  assert (val == insert_val);
  assert (vec->len == 3);

  // test invalid index
  assert (vector_insert (vec, 4, &val) == false);
  assert (vector_get_error () == VECTOR_OUT_OF_RANGE);
  assert (vector_remove (vec, 3, &val) == false);
  assert (vector_get_error () == VECTOR_OUT_OF_RANGE);

  vector_destroy (vec);
  TEST_PASS ("vector_insert_remove");
}

static void
test_vector_reserve_shrink (void)
{
  vector_t *vec = vector_create (2, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 };

  // test reserve
  assert (vector_reserve (vec, 8) == true);
  assert (vec->capacity == 8);

  for (int32_t i = 0; i < 4; i++)
    {
      vector_push (vec, &values[i]);
    }

  // test shrink
  assert (vector_shrink (vec) == true);
  assert (vec->capacity == vec->len);

  vector_destroy (vec);

  // test null parameter
  assert (vector_reserve (NULL, 8) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);
  assert (vector_shrink (NULL) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  TEST_PASS ("vector_reserve_shrink");
}

static void
test_vector_clear (void)
{
  vector_t *vec = vector_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 };

  for (int32_t i = 0; i < 4; i++)
    {
      vector_push (vec, &values[i]);
    }

  assert (vector_clear (vec) == true);
  assert (vec->len == 0);
  assert (vec->capacity > 0); // capacity shouldn't change

  vector_destroy (vec);

  // test null parameter
  assert (vector_clear (NULL) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  TEST_PASS ("vector_clear");
}

static void
test_vector_is_empty (void)
{
  vector_t *vec = vector_create (4, sizeof (int32_t));
  assert (vector_is_empty (vec) == true);

  int32_t val = 1;
  vector_push (vec, &val);
  assert (vector_is_empty (vec) == false);

  vector_pop (vec, &val);
  assert (vector_is_empty (vec) == true);

  // test null parameter
  assert (vector_is_empty (NULL) == true);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  vector_destroy (vec);
  TEST_PASS ("vector_is_empty");
}

static void
test_vector_back (void)
{
  vector_t *vec = vector_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3 };
  int32_t val;

  // test empty vector
  assert (vector_back (vec, &val) == false);
  assert (vector_get_error () == VECTOR_OUT_OF_RANGE);

  for (int32_t i = 0; i < 3; i++)
    {
      vector_push (vec, &values[i]);
    }

  // test normal case
  assert (vector_back (vec, &val) == true);
  assert (val == values[2]);
  assert (vec->len == 3); // length shouldn't change

  // test null parameters
  assert (vector_back (NULL, &val) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);
  assert (vector_back (vec, NULL) == false);
  assert (vector_get_error () == VECTOR_NULL_PTR);

  vector_destroy (vec);
  TEST_PASS ("vector_back");
}
