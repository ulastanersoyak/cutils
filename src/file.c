#include "cutils/file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

static thread_local file_result_t g_last_error = FILE_OK;

[[nodiscard]] file_result_t
file_get_error (void)
{
  return g_last_error;
}

[[nodiscard]] bool
file_exists (const char *path)
{
  FILE *file = fopen (path, "r");
  if (file == NULL)
    {
      g_last_error = FILE_NOT_FOUND;
      return false;
    }

  fclose (file);
  return true;
}

[[nodiscard]] size_t
file_size (const char *path)
{
  g_last_error = FILE_OK;

  if (path == NULL)
    {
      g_last_error = FILE_NULL_PTR;
      return 0;
    }

  FILE *file = fopen (path, "rb");
  if (file == NULL)
    {
      g_last_error = FILE_NOT_FOUND;
      return 0;
    }

  if (fseek (file, 0, SEEK_END) != 0)
    {
      g_last_error = FILE_IO_ERROR;
      fclose (file);
      return 0;
    }

  long size = ftell (file);
  fclose (file);

  if (size < 0)
    {
      g_last_error = FILE_IO_ERROR;
      return 0;
    }

  return (size_t)size;
}

[[nodiscard]] char *
file_read_all (const char *path)
{
  g_last_error = FILE_OK;

  if (path == NULL)
    {
      g_last_error = FILE_NULL_PTR;
      return NULL;
    }

  size_t file_length = file_size (path);
  if (file_length == 0 && g_last_error != FILE_OK)
    {
      return NULL; // error already set by file_size
    }

  char *buffer = malloc (file_length + 1);
  if (buffer == NULL)
    {
      g_last_error = FILE_NO_MEMORY;
      return NULL;
    }

  if (!file_read_bytes (path, buffer, file_length))
    {
      free (buffer);
      return NULL; // error already set by file_read_bytes
    }

  buffer[file_length] = '\0';
  return buffer;
}

[[nodiscard]] char **
file_read_lines (const char *path, size_t *line_count)
{
  g_last_error = FILE_OK;

  if (path == NULL || line_count == NULL)
    {
      g_last_error = FILE_NULL_PTR;
      return NULL;
    }

  char *content = file_read_all (path);
  if (content == NULL)
    {
      return NULL; // error already set by file_read_all
    }

  // first pass: count lines and validate content
  size_t count = 1; // at least one line even without \n
  size_t max_line_len = 0;
  size_t current_line_len = 0;

  for (char *p = content; *p != '\0'; p++)
    {
      if (*p == '\n')
        {
          count++;
          if (current_line_len > max_line_len)
            {
              max_line_len = current_line_len;
            }
          current_line_len = 0;
        }
      else
        {
          current_line_len++;
        }
    }
  // Check last line
  if (current_line_len > max_line_len)
    {
      max_line_len = current_line_len;
    }

  // Allocate array of pointers
  char **lines = malloc (count * sizeof (char *));
  if (lines == NULL)
    {
      g_last_error = FILE_NO_MEMORY;
      free (content);
      return NULL;
    }

  // Second pass: split into lines
  char *line_start = content;
  size_t index = 0;

  for (char *p = content;; p++)
    {
      if (*p == '\n' || *p == '\0')
        {
          size_t line_len = (size_t)(p - line_start);

          lines[index] = malloc (line_len + 1);
          if (lines[index] == NULL)
            {
              g_last_error = FILE_NO_MEMORY;
              for (size_t i = 0; i < index; i++)
                {
                  free (lines[i]);
                }
              free (lines);
              free (content);
              return NULL;
            }
          memcpy (lines[index], line_start, line_len);
          lines[index][line_len] = '\0';
          index++;

          if (*p == '\0')
            {
              break;
            }
          line_start = p + 1;
        }
    }

  *line_count = index;
  free (content);
  return lines;
}

[[nodiscard]] bool
file_read_bytes (const char *path, void *buffer, size_t size)
{
  g_last_error = FILE_OK;

  if (path == NULL || buffer == NULL)
    {
      g_last_error = FILE_NULL_PTR;
      return false;
    }

  size_t actual_size = file_size (path);
  if (actual_size == 0 && g_last_error != FILE_OK)
    {
      return false; // error already set by file_size
    }

  if (size > actual_size)
    {
      g_last_error = FILE_IO_ERROR;
      return false;
    }

  FILE *file = fopen (path, "rb");
  if (file == NULL)
    {
      g_last_error = FILE_NOT_FOUND;
      return false;
    }

  size_t bytes_read = fread (buffer, 1, size, file);
  fclose (file);

  if (bytes_read != size)
    {
      g_last_error = FILE_IO_ERROR;
      return false;
    }

  return true;
}
