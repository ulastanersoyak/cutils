#include "cutils/expected.h"
#include "cutils/config.h"
#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local expected_result_t g_last_error = EXPECTED_OK;

[[maybe_unused]] static bool
check_timeout ([[maybe_unused]] uint32_t start_time_ms,
               [[maybe_unused]] uint32_t timeout_ms)
{
#if CUTILS_PLATFORM_BARE_METAL
  // Implement platform-specific time check
  return true;
#else
  uint32_t current_time = get_current_time_ms ();
  return (current_time - start_time_ms) <= timeout_ms;
#endif
}

expected_t *
expected_create_with_allocator (size_t size, cutils_allocator_t *allocator)
{
  g_last_error = EXPECTED_OK;

  if (allocator == NULL)
    {
      g_last_error = EXPECTED_INVALID_ARG;
      return NULL;
    }

  expected_t *expected = cutils_allocate_aligned (
      allocator, sizeof (expected_t), CUTILS_ALIGNMENT);
  if (expected == NULL)
    {
      g_last_error = EXPECTED_NO_MEMORY;
      return NULL;
    }

  if (size > 0)
    {
      expected->data
          = cutils_allocate_aligned (allocator, size, CUTILS_ALIGNMENT);
      if (expected->data == NULL)
        {
          cutils_deallocate (allocator, expected);
          g_last_error = EXPECTED_NO_MEMORY;
          return NULL;
        }
    }
  else
    {
      expected->data = NULL;
    }

  expected->size = size;
  expected->error = EXPECTED_OK;
  expected->allocator = allocator;

  return expected;
}

expected_t *
expected_create (size_t size)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return expected_create_with_allocator (size, &default_allocator);
}

expected_t *
expected_from_data_with_allocator (const void *data, size_t size,
                                   cutils_allocator_t *allocator)
{
  g_last_error = EXPECTED_OK;

  if (data == NULL || allocator == NULL)
    {
      g_last_error = EXPECTED_INVALID_ARG;
      return NULL;
    }

  expected_t *expected = expected_create_with_allocator (size, allocator);
  if (expected == NULL)
    {
      return NULL;
    }

  memcpy (expected->data, data, size);
  expected->error = EXPECTED_OK;

  return expected;
}

expected_t *
expected_from_data (const void *data, size_t size)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return expected_from_data_with_allocator (data, size, &default_allocator);
}

expected_t *
expected_from_error_with_allocator (expected_result_t error,
                                    cutils_allocator_t *allocator)
{
  g_last_error = EXPECTED_OK;

  if (allocator == NULL)
    {
      g_last_error = EXPECTED_INVALID_ARG;
      return NULL;
    }

  expected_t *expected = expected_create_with_allocator (0, allocator);
  if (expected == NULL)
    {
      return NULL;
    }

  expected->error = error;

  return expected;
}

expected_t *
expected_from_error (expected_result_t error)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return expected_from_error_with_allocator (error, &default_allocator);
}

void
expected_destroy (expected_t *expected)
{
  g_last_error = EXPECTED_OK;

  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return;
    }

  if (expected->data != NULL)
    {
      cutils_deallocate (expected->allocator, expected->data);
    }
  cutils_deallocate (expected->allocator, expected);
}

void *
expected_get_data (const expected_t *expected)
{
  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return NULL;
    }

  if (expected->error != EXPECTED_OK)
    {
      g_last_error = expected->error;
      return NULL;
    }

  return expected->data;
}

expected_result_t
expected_get_error (const expected_t *expected)
{
  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return EXPECTED_NULL_PTR;
    }

  return expected->error;
}

bool
expected_has_value (const expected_t *expected)
{
  g_last_error = EXPECTED_OK;

  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return false;
    }

  return expected->error == EXPECTED_OK;
}

bool
expected_has_error (const expected_t *expected)
{
  g_last_error = EXPECTED_OK;

  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return true;
    }

  return expected->error != EXPECTED_OK;
}

size_t
expected_size (const expected_t *expected)
{
  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return 0;
    }
  return expected->size;
}

size_t
expected_memory_usage (const expected_t *expected)
{
  if (expected == NULL)
    {
      g_last_error = EXPECTED_NULL_PTR;
      return 0;
    }
  return sizeof (expected_t) + expected->size;
}

expected_result_t
expected_get_last_error (void)
{
  return g_last_error;
}
