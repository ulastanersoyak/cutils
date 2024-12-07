#include "cutils/expected.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_expected_success (void);
static void test_expected_error (void);
static void test_expected_has_value (void);
static void test_expected_value (void);
static void test_expected_error_value (void);

void
run_expected_tests (void)
{
  test_expected_success ();
  test_expected_error ();
  test_expected_has_value ();
  test_expected_value ();
  test_expected_error_value ();
}

static void
test_expected_success (void)
{
  int value = 42;
  expected_t *exp = expected_success (&value, sizeof (int));

  assert (exp != NULL);
  assert (exp->has_value == true);
  assert (*(int *)exp->data.value == value);
  expected_destroy (exp);

  // test NULL input
  exp = expected_success (NULL, sizeof (int));
  assert (exp == NULL);
  assert (expected_get_error () == EXPECTED_NULL_PTR);

  // test zero size
  exp = expected_success (&value, 0);
  assert (exp == NULL);
  assert (expected_get_error () == EXPECTED_INVALID_ARG);

  TEST_PASS ("expected_success");
}

static void
test_expected_error (void)
{
  const char *error_msg = "Error occurred";
  expected_t *exp = expected_error (error_msg, strlen (error_msg) + 1);

  assert (exp != NULL);
  assert (exp->has_value == false);
  assert (strcmp ((char *)exp->data.error, error_msg) == 0);
  expected_destroy (exp);

  // test NULL input
  exp = expected_error (NULL, 10);
  assert (exp == NULL);
  assert (expected_get_error () == EXPECTED_NULL_PTR);

  // test zero size
  exp = expected_error (error_msg, 0);
  assert (exp == NULL);
  assert (expected_get_error () == EXPECTED_INVALID_ARG);

  TEST_PASS ("expected_error");
}

static void
test_expected_has_value (void)
{
  int value = 42;
  const char *error_msg = "Error";

  expected_t *success = expected_success (&value, sizeof (int));
  expected_t *failure = expected_error (error_msg, strlen (error_msg) + 1);

  assert (expected_has_value (success) == true);
  assert (expected_has_value (failure) == false);

  // test NULL input
  assert (expected_has_value (NULL) == false);
  assert (expected_get_error () == EXPECTED_NULL_PTR);

  expected_destroy (success);
  expected_destroy (failure);

  TEST_PASS ("expected_has_value");
}

static void
test_expected_value (void)
{
  int value = 42;
  expected_t *exp = expected_success (&value, sizeof (int));

  void *result = expected_value (exp);
  assert (result != NULL);
  assert (*(int *)result == value);

  // test NULL input
  result = expected_value (NULL);
  assert (result == NULL);
  assert (expected_get_error () == EXPECTED_NULL_PTR);

  // test getting value from error expected
  const char *error_msg = "Error";
  expected_t *error_exp = expected_error (error_msg, strlen (error_msg) + 1);
  result = expected_value (error_exp);
  assert (result == NULL);
  assert (expected_get_error () == EXPECTED_INVALID_VALUE);

  expected_destroy (exp);
  expected_destroy (error_exp);

  TEST_PASS ("expected_value");
}

static void
test_expected_error_value (void)
{
  const char *error_msg = "Error occurred";
  expected_t *exp = expected_error (error_msg, strlen (error_msg) + 1);

  void *result = expected_error_value (exp);
  assert (result != NULL);
  assert (strcmp ((char *)result, error_msg) == 0);

  // test NULL input
  result = expected_error_value (NULL);
  assert (result == NULL);
  assert (expected_get_error () == EXPECTED_NULL_PTR);

  // test getting error from success expected
  int value = 42;
  expected_t *success_exp = expected_success (&value, sizeof (int));
  result = expected_error_value (success_exp);
  assert (result == NULL);
  assert (expected_get_error () == EXPECTED_INVALID_VALUE);

  expected_destroy (exp);
  expected_destroy (success_exp);

  TEST_PASS ("expected_error_value");
}
