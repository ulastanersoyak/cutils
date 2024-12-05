#include "cutils/string.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static thread_local string_result_t g_last_error = STRING_OK;

[[nodiscard]] string_result_t
string_get_error (void)
{
  return g_last_error;
}

[[nodiscard]] string_t *
string_create (const char *data, size_t len)
{
  g_last_error = STRING_OK;

  if (data == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (len > SIZE_MAX - 1)
    {
      g_last_error = STRING_OVERFLOW;
      return NULL;
    }

  string_t *str = (string_t *)malloc (sizeof (string_t));
  if (str == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  str->data = (char *)malloc (len + 1);
  if (str->data == NULL)
    {
      free (str);
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  memcpy (str->data, data, len);
  str->data[len] = '\0';
  str->len = len;

  return str;
}

[[nodiscard]] string_t *
string_from_cstr (const char *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  return string_create (str, strlen (str));
}

[[nodiscard]] string_t *
string_copy (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  return string_create (str->data, str->len);
}

[[nodiscard]] string_t *
string_move (string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  string_t *new_str = (string_t *)malloc (sizeof (string_t));
  if (new_str == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  new_str->data = str->data;
  new_str->len = str->len;

  str->data = NULL;
  str->len = 0;

  return new_str;
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
      free (str->data);
      str->data = NULL;
      str->len = 0;
    }

  free (str);
}

[[nodiscard]] bool
string_equals (const string_t *first, const string_t *second)
{
  g_last_error = STRING_OK;

  if (first == second)
    {
      return true;
    }

  if (first == NULL || second == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }

  if (first->data == NULL || second->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return false;
    }

  if (first->len != second->len)
    {
      return false;
    }

  return memcmp (first->data, second->data, first->len) == 0;
}

[[nodiscard]] bool
string_starts_with (const string_t *str, const string_t *prefix)
{
  g_last_error = STRING_OK;

  if (str == NULL || prefix == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }

  if (str->data == NULL || prefix->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return false;
    }

  if (prefix->len > str->len)
    {
      return false;
    }

  return memcmp (str->data, prefix->data, prefix->len) == 0;
}

[[nodiscard]] bool
string_ends_with (const string_t *str, const string_t *suffix)
{
  g_last_error = STRING_OK;

  if (str == NULL || suffix == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }

  if (str->data == NULL || suffix->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return false;
    }

  if (suffix->len > str->len)
    {
      return false;
    }

  return memcmp (str->data + (str->len - suffix->len), suffix->data,
                 suffix->len)
         == 0;
}

[[nodiscard]] string_t *
string_trim (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  size_t start = 0;
  while (start < str->len && isspace ((unsigned char)str->data[start]))
    {
      start++;
    }

  if (start == str->len)
    {
      string_t *result = malloc (sizeof (string_t));
      if (result == NULL)
        {
          g_last_error = STRING_NO_MEMORY;
          return NULL;
        }

      result->data = malloc (1);
      if (result->data == NULL)
        {
          free (result);
          g_last_error = STRING_NO_MEMORY;
          return NULL;
        }

      result->data[0] = '\0';
      result->len = 0;
      return result;
    }

  size_t end = str->len - 1;
  while (end > start && isspace ((unsigned char)str->data[end]))
    {
      end--;
    }

  size_t new_len = end - start + 1;
  string_t *result = malloc (sizeof (string_t));
  if (result == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  result->data = malloc (new_len + 1);
  if (result->data == NULL)
    {
      free (result);
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  memcpy (result->data, str->data + start, new_len);
  result->data[new_len] = '\0';
  result->len = new_len;

  return result;
}

[[nodiscard]] string_t *
string_trim_left (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  size_t start = 0;
  while (start < str->len && isspace ((unsigned char)str->data[start]))
    {
      start++;
    }

  size_t new_len = str->len - start;
  string_t *result = malloc (sizeof (string_t));
  if (result == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  result->data = malloc (new_len + 1);
  if (result->data == NULL)
    {
      free (result);
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  memcpy (result->data, str->data + start, new_len);
  result->data[new_len] = '\0';
  result->len = new_len;

  return result;
}

[[nodiscard]] string_t *
string_trim_right (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  if (str->len == 0)
    {
      return string_copy (str);
    }

  size_t end = str->len - 1;
  while (isspace ((unsigned char)str->data[end]))
    {
      end--;
    }

  size_t new_len = end + 1;
  string_t *result = malloc (sizeof (string_t));
  if (result == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  result->data = malloc (new_len + 1);
  if (result->data == NULL)
    {
      free (result);
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  memcpy (result->data, str->data, new_len);
  result->data[new_len] = '\0';
  result->len = new_len;

  return result;
}

[[nodiscard]] string_t *
string_to_upper (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  string_t *upper_str = string_copy (str);
  if (upper_str == NULL)
    {
      // error is already set by string_copy
      return NULL;
    }

  for (size_t i = 0; i < str->len; i++)
    {
      upper_str->data[i] = (char)toupper ((unsigned char)str->data[i]);
    }

  return upper_str;
}

[[nodiscard]] string_t *
string_to_lower (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  string_t *lower_str = string_copy (str);
  if (lower_str == NULL)
    {
      // error is already set by string_copy
      return NULL;
    }

  for (size_t i = 0; i < str->len; i++)
    {
      lower_str->data[i] = (char)tolower ((unsigned char)str->data[i]);
    }

  return lower_str;
}

[[nodiscard]] string_t *
string_substring (const string_t *str, size_t start, size_t length)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  if (start > str->len)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  if (length > str->len - start)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  string_t *result = malloc (sizeof (string_t));
  if (result == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  result->data = malloc (length + 1);
  if (result->data == NULL)
    {
      free (result);
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  memcpy (result->data, str->data + start, length);
  result->data[length] = '\0';
  result->len = length;

  return result;
}

[[nodiscard]] string_t *
string_concat (const string_t *first, const string_t *second)
{
  g_last_error = STRING_OK;

  if (first == NULL || second == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (first->data == NULL || second->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  if (SIZE_MAX - first->len < second->len)
    {
      g_last_error = STRING_OVERFLOW;
      return NULL;
    }

  size_t concat_len = first->len + second->len;

  string_t *concated_str = malloc (sizeof (string_t));
  if (concated_str == NULL)
    {
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  concated_str->data = malloc (concat_len + 1);
  if (concated_str->data == NULL)
    {
      free (concated_str);
      g_last_error = STRING_NO_MEMORY;
      return NULL;
    }

  concated_str->len = concat_len;
  memcpy (concated_str->data, first->data, first->len);
  memcpy (concated_str->data + first->len, second->data, second->len);
  concated_str->data[concat_len] = '\0';

  return concated_str;
}

[[nodiscard]] size_t
string_find (const string_t *str, const string_t *pattern)
{
  g_last_error = STRING_OK;

  if (str == NULL || pattern == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return SIZE_MAX;
    }

  if (str->data == NULL || pattern->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return SIZE_MAX;
    }

  if (pattern->len == 0 || pattern->len > str->len)
    {
      return SIZE_MAX;
    }

  for (size_t i = 0; i <= str->len - pattern->len; i++)
    {
      if (memcmp (str->data + i, pattern->data, pattern->len) == 0)
        {
          return i;
        }
    }

  return SIZE_MAX;
}

[[nodiscard]] size_t
string_find_last (const string_t *str, const string_t *pattern)
{
  g_last_error = STRING_OK;

  if (str == NULL || pattern == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return SIZE_MAX;
    }

  if (str->data == NULL || pattern->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return SIZE_MAX;
    }

  if (pattern->len == 0 || pattern->len > str->len)
    {
      return SIZE_MAX;
    }

  for (size_t i = str->len - pattern->len; i != SIZE_MAX; i--)
    {
      if (memcmp (str->data + i, pattern->data, pattern->len) == 0)
        {
          return i;
        }
    }

  return SIZE_MAX;
}

[[nodiscard]] bool
string_contains (const string_t *str, const string_t *substr)
{
  g_last_error = STRING_OK;

  if (str == NULL || substr == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return false;
    }

  if (str->data == NULL || substr->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return false;
    }

  if (substr->len == 0)
    {
      return true;
    }

  if (substr->len > str->len)
    {
      return false;
    }

  for (size_t i = 0; i <= str->len - substr->len; i++)
    {
      if (memcmp (str->data + i, substr->data, substr->len) == 0)
        {
          return true;
        }
    }

  return false;
}

[[nodiscard]] string_t *
string_reverse (const string_t *str)
{
  g_last_error = STRING_OK;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return NULL;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return NULL;
    }

  string_t *result = string_copy (str);
  if (result == NULL)
    {
      return NULL;
    }

  for (size_t i = 0; i < result->len / 2; i++)
    {
      char temp = result->data[i];
      result->data[i] = result->data[result->len - 1 - i];
      result->data[result->len - 1 - i] = temp;
    }

  return result;
}

[[nodiscard]] int64_t
string_to_int (const string_t *str, bool *success)
{
  g_last_error = STRING_OK;
  *success = false;

  if (str == NULL)
    {
      g_last_error = STRING_NULL_PTR;
      return 0;
    }

  if (str->data == NULL)
    {
      g_last_error = STRING_INVALID_ARG;
      return 0;
    }

  char *endptr;
  int64_t result = strtoll (str->data, &endptr, 10);

  if (endptr == str->data || *endptr != '\0')
    {
      g_last_error = STRING_INVALID_ARG;
      return 0;
    }

  if (errno == ERANGE)
    {
      g_last_error = STRING_OVERFLOW;
      return 0;
    }

  *success = true;
  return result;
}

[[nodiscard]] string_t *
string_from_int (int64_t value)
{
  g_last_error = STRING_OK;

  char buffer[32]; // large enough for any 64-bit integer
  int len = snprintf (buffer, sizeof (buffer), "%" PRId64, value);

  if (len < 0 || (size_t)len >= sizeof (buffer))
    {
      g_last_error = STRING_OVERFLOW;
      return NULL;
    }

  return string_create (buffer, (size_t)len);
}
