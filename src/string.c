#include "cutils/string.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local string_result_t g_last_error = STRING_OK;

static bool
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

static bool
resize_if_needed (string_t *str, size_t required_capacity, uint32_t timeout_ms)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }

  if (required_capacity <= str->capacity)
    {
      return true;
    }

  uint64_t start_time = cutils_get_current_time_ms ();
  if (!check_timeout ((uint32_t)start_time, timeout_ms))
    {
      g_last_error = STRING_TIMEOUT;
      return false;
    }

  size_t new_capacity = str->capacity;
  while (new_capacity < required_capacity)
    {
      if (new_capacity > SIZE_MAX / 2)
        {
          g_last_error = STRING_OVERFLOW;
          return false;
        }
      new_capacity *= 2;
    }

  char *new_data = cutils_allocate_aligned (str->allocator, new_capacity,
                                            CUTILS_ALIGNMENT);
  if (new_data == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return false;
    }

  if (str->data != NULL)
    {
      memcpy (new_data, str->data, str->length);
      cutils_deallocate (str->allocator, str->data);
    }

  str->data = new_data;
  str->capacity = new_capacity;
  return true;
}

string_t *
string_create_with_allocator (size_t initial_capacity,
                              cutils_allocator_t *allocator)
{
  g_last_error = STRING_OK;

  if (allocator == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  string_t *str = cutils_allocate_aligned (allocator, sizeof (string_t),
                                           CUTILS_ALIGNMENT);
  if (str == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  if (initial_capacity > 0)
    {
      str->data = cutils_allocate_aligned (allocator, initial_capacity,
                                           CUTILS_ALIGNMENT);
      if (str->data == NULL)
        {
          cutils_deallocate (allocator, str);
          g_last_error = STRING_NO_MEMORY;
          return NULL;
        }
    }
  else
    {
      str->data = NULL;
    }

  str->length = 0;
  str->capacity = initial_capacity;
  str->allocator = allocator;

  return str;
}

string_t *
string_create (size_t initial_capacity)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return string_create_with_allocator (initial_capacity, &default_allocator);
}

string_t *
string_from_cstr_with_allocator (const char *cstr,
                                 cutils_allocator_t *allocator)
{
  g_last_error = STRING_OK;

  if (cstr == NULL || allocator == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  size_t len = strlen (cstr);
  string_t *str = string_create_with_allocator (len + 1, allocator);
  if (str == NULL)
    {
      return NULL;
    }

  memcpy (str->data, cstr, len + 1);
  str->length = len;

  return str;
}

string_t *
string_from_cstr (const char *cstr)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return string_from_cstr_with_allocator (cstr, &default_allocator);
}

void
string_destroy (string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return;
    }

  if (str->data != NULL)
    {
      cutils_deallocate (str->allocator, str->data);
    }
  cutils_deallocate (str->allocator, str);
}

bool
string_append_timeout (string_t *str, const char *cstr, uint32_t timeout_ms)
{
  g_last_error = STRING_OK;

  if (str == NULL || cstr == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return false;
    }

  size_t len = strlen (cstr);
  if (!resize_if_needed (str, str->length + len + 1, timeout_ms))
    {
      return false;
    }

  memcpy (str->data + str->length, cstr, len + 1);
  str->length += len;

  return true;
}

bool
string_append (string_t *str, const char *cstr)
{
  return string_append_timeout (str, cstr, UINT32_MAX);
}

bool
string_append_char_timeout (string_t *str, char c, uint32_t timeout_ms)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return false;
    }

  if (!resize_if_needed (str, str->length + 2, timeout_ms))
    {
      return false;
    }

  str->data[str->length++] = c;
  str->data[str->length] = '\0';

  return true;
}

bool
string_append_char (string_t *str, char c)
{
  return string_append_char_timeout (str, c, UINT32_MAX);
}

size_t
string_length (const string_t *str)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return 0;
    }
  return str->length;
}

size_t
string_capacity (const string_t *str)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return 0;
    }
  return str->capacity;
}

bool
string_is_empty (const string_t *str)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return true;
    }
  return str->length == 0;
}

bool
string_clear (string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }

  str->length = 0;
  if (str->data != NULL)
    {
      str->data[0] = '\0';
    }
  return true;
}

size_t
string_memory_usage (const string_t *str)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return 0;
    }
  return sizeof (string_t) + str->capacity;
}

bool
string_can_perform_operation (const string_t *str, size_t required_capacity)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }
  return required_capacity <= str->capacity;
}

string_result_t
string_get_error (void)
{
  return g_last_error;
}

const char *
string_cstr (const string_t *str)
{
  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }
  return str->data;
}

size_t
string_find (const string_t *str, const char *substr)
{
  if (str == NULL || substr == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return SIZE_MAX;
    }

  char *pos = strstr (str->data, substr);
  if (pos == NULL)
    {
      g_last_error = STRING_NOT_FOUND;
      return SIZE_MAX;
    }

  return (size_t)(pos - str->data);
}

size_t
string_find_char (const string_t *str, char c)
{
  if (str == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return SIZE_MAX;
    }

  char *pos = strchr (str->data, c);
  if (pos == NULL)
    {
      g_last_error = STRING_NOT_FOUND;
      return SIZE_MAX;
    }

  return (size_t)(pos - str->data);
}

string_t *
string_substring (const string_t *str, size_t start, size_t length)
{
  if (str == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  if (start >= str->length)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  if (length > str->length - start)
    {
      length = str->length - start;
    }

  string_t *substr = string_create_with_allocator (length + 1, str->allocator);
  if (substr == NULL)
    {
      return NULL;
    }

  memcpy (substr->data, str->data + start, length);
  substr->data[length] = '\0';
  substr->length = length;

  return substr;
}

int
string_compare (const string_t *str1, const string_t *str2)
{
  if (str1 == NULL || str2 == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return 0;
    }
  return strcmp (str1->data, str2->data);
}

int
string_compare_cstr (const string_t *str, const char *cstr)
{
  if (str == NULL || cstr == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return 0;
    }
  return strcmp (str->data, cstr);
}
