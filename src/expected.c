#include "cutils/expected.h"

#include <stdlib.h>
#include <string.h>

static expected_result_t g_last_error = EXPECTED_OK;

[[nodiscard]] expected_result_t
expected_get_error (void)
{
  return g_last_error;
}

[[nodiscard]] expected_t *
expected_success (const void *value, size_t size)
{
  g_last_error = EXPECTED_OK;

  if (value == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return NULL;
    }

  if (size == 0)
    {
      g_last_error = EXPECTED_INVALID_ARG;
      return NULL;
    }

  expected_t *success = (expected_t *)malloc (sizeof (expected_t));
  if (success == NULL)
    {
      g_last_error = EXPECTED_NO_MEMORY;
      return NULL;
    }

  success->data.value = malloc (size);
  if (success->data.value == NULL)
    {
      free (success);
      g_last_error = EXPECTED_NO_MEMORY;
      return NULL;
    }

  memcpy (success->data.value, value, size);
  success->has_value = true;
  return success;
}

[[nodiscard]] expected_t *
expected_error (const void *error, size_t size)
{
  g_last_error = EXPECTED_OK;

  if (error == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return NULL;
    }

  if (size == 0)
    {
      g_last_error = EXPECTED_INVALID_ARG;
      return NULL;
    }

  expected_t *failure = (expected_t *)malloc (sizeof (expected_t));
  if (failure == NULL)
    {
      g_last_error = EXPECTED_NO_MEMORY;
      return NULL;
    }

  failure->data.error = malloc (size);
  if (failure->data.error == NULL)
    {
      free (failure);
      g_last_error = EXPECTED_NO_MEMORY;
      return NULL;
    }

  memcpy (failure->data.error, error, size);
  failure->has_value = false;
  return failure;
}

bool
expected_has_value (const expected_t *exp)
{
  g_last_error = EXPECTED_OK;

  if (exp == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return false;
    }
  return exp->has_value;
}

[[nodiscard]] void *
expected_value (const expected_t *exp)
{
  g_last_error = EXPECTED_OK;

  if (exp == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return NULL;
    }

  if (exp->has_value == false)
    {
      g_last_error = EXPECTED_INVALID_VALUE;
      return NULL;
    }

  return exp->data.value;
}

[[nodiscard]] void *
expected_error_value (const expected_t *exp)
{
  g_last_error = EXPECTED_OK;

  if (exp == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return NULL;
    }

  if (exp->has_value == true)
    {
      g_last_error = EXPECTED_INVALID_VALUE;
      return NULL;
    }

  return exp->data.error;
}

void
expected_destroy (expected_t *exp)
{
  if (exp == NULL)
    {
      return;
    }

  free (exp->data.value); // union means this frees either value or error
  free (exp);
}
