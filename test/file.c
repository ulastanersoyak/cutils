#include "cutils/file.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_file_exists (void);
static void test_file_size (void);
static void test_file_read_all (void);
static void test_file_read_lines (void);
static void test_file_read_bytes (void);
static void create_test_file (const char *path, const char *content);

void
run_file_tests (void)
{
  test_file_exists ();
  test_file_size ();
  test_file_read_all ();
  test_file_read_lines ();
  test_file_read_bytes ();
}

static void
create_test_file (const char *path, const char *content)
{
  FILE *file = fopen (path, "w");
  assert (file != NULL);
  fputs (content, file);
  fclose (file);
}

static void
test_file_exists (void)
{
  // test normal case
  create_test_file ("test.txt", "hello");
  assert (file_exists ("test.txt") == true);
  remove ("test.txt");

  // test non-existent file
  assert (file_exists ("nonexistent.txt") == false);
  assert (file_get_error () == FILE_NOT_FOUND);

  // test empty file
  create_test_file ("empty.txt", "");
  assert (file_exists ("empty.txt") == true);
  remove ("empty.txt");

  TEST_PASS ("file_exists");
}

static void
test_file_size (void)
{
  // test normal case
  create_test_file ("test.txt", "hello");
  assert (file_size ("test.txt") == 5);
  remove ("test.txt");

  // test empty file
  create_test_file ("empty.txt", "");
  assert (file_size ("empty.txt") == 0);
  assert (file_get_error () == FILE_OK); // ensure it's not an error state
  remove ("empty.txt");

  // test non-existent file
  assert (file_size ("nonexistent.txt") == 0);
  assert (file_get_error () == FILE_NOT_FOUND);

  // test null path
  assert (file_size (NULL) == 0);
  assert (file_get_error () == FILE_NULL_PTR);

  TEST_PASS ("file_size");
}

static void
test_file_read_all (void)
{
  // test normal case
  const char *content = "hello\nworld";
  create_test_file ("test.txt", content);

  char *result = file_read_all ("test.txt");
  assert (result != NULL);
  assert (strcmp (result, content) == 0);
  free (result);
  remove ("test.txt");

  // test empty file
  create_test_file ("empty.txt", "");
  result = file_read_all ("empty.txt");
  assert (result != NULL);
  assert (result[0] == '\0');
  free (result);
  remove ("empty.txt");

  // test non-existent file
  result = file_read_all ("nonexistent.txt");
  assert (result == NULL);
  assert (file_get_error () == FILE_NOT_FOUND);

  // test null path
  result = file_read_all (NULL);
  assert (result == NULL);
  assert (file_get_error () == FILE_NULL_PTR);

  TEST_PASS ("file_read_all");
}

static void
test_file_read_lines (void)
{
  size_t line_count;
  char **lines;

  // test normal case
  create_test_file ("test.txt", "line1\nline2\nline3");
  lines = file_read_lines ("test.txt", &line_count);
  assert (lines != NULL);
  assert (line_count == 3);
  assert (strcmp (lines[0], "line1") == 0);
  assert (strcmp (lines[1], "line2") == 0);
  assert (strcmp (lines[2], "line3") == 0);

  for (size_t i = 0; i < line_count; i++)
    {
      free (lines[i]);
    }
  free (lines);
  remove ("test.txt");

  // test empty file
  create_test_file ("empty.txt", "");
  lines = file_read_lines ("empty.txt", &line_count);
  assert (lines != NULL);
  assert (line_count == 1);
  assert (lines[0][0] == '\0');
  free (lines[0]);
  free (lines);
  remove ("empty.txt");

  // test file with empty lines
  create_test_file ("empty_lines.txt", "\n\nline\n\n");
  lines = file_read_lines ("empty_lines.txt", &line_count);
  assert (lines != NULL);
  assert (line_count == 5);
  assert (lines[0][0] == '\0');
  assert (lines[1][0] == '\0');
  assert (strcmp (lines[2], "line") == 0);
  assert (lines[3][0] == '\0');
  assert (lines[4][0] == '\0');

  for (size_t i = 0; i < line_count; i++)
    {
      free (lines[i]);
    }
  free (lines);
  remove ("empty_lines.txt");

  // test null parameters
  lines = file_read_lines (NULL, &line_count);
  assert (lines == NULL);
  assert (file_get_error () == FILE_NULL_PTR);

  lines = file_read_lines ("test.txt", NULL);
  assert (lines == NULL);
  assert (file_get_error () == FILE_NULL_PTR);

  TEST_PASS ("file_read_lines");
}

static void
test_file_read_bytes (void)
{
  // test normal case
  const char content[] = "hello world";
  create_test_file ("test.txt", content);

  char buffer[12];
  assert (file_read_bytes ("test.txt", buffer, 11) == true);
  assert (memcmp (buffer, content, 11) == 0);
  remove ("test.txt");

  // test partial read
  create_test_file ("test.txt", "hello world");
  assert (file_read_bytes ("test.txt", buffer, 5) == true);
  assert (memcmp (buffer, "hello", 5) == 0);
  remove ("test.txt");

  // test read more than file size
  create_test_file ("test.txt", "hello");
  assert (file_read_bytes ("test.txt", buffer, 10) == false);
  assert (file_get_error () == FILE_IO_ERROR);
  remove ("test.txt");

  // test null parameters
  assert (file_read_bytes (NULL, buffer, 5) == false);
  assert (file_get_error () == FILE_NULL_PTR);

  assert (file_read_bytes ("test.txt", NULL, 5) == false);
  assert (file_get_error () == FILE_NULL_PTR);

  TEST_PASS ("file_read_bytes");
}
