#include "cutils/map.h"
#include "test.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_map_create (void);
static void test_map_insert (void);
static void test_map_get (void);
static void test_map_remove (void);
static void test_map_contains (void);
static void test_map_size (void);

// Test utilities
static size_t
test_hash (const void *key)
{
  return *(const size_t *)key;
}

static bool
test_eq (const void *key1, const void *key2)
{
  return *(const int *)key1 == *(const int *)key2;
}

void
run_map_tests (void)
{
  test_map_create ();
  test_map_insert ();
  test_map_get ();
  test_map_remove ();
  test_map_contains ();
  test_map_size ();
}

static void
test_map_create (void)
{
  map_t *map = map_create (sizeof (int), sizeof (double), 16, test_hash,
                           test_eq, NULL, NULL);
  assert (map != NULL);
  assert (map->bucket_count == 16);
  assert (map->size == 0);
  map_destroy (map);

  // test NULL hash function
  map = map_create (sizeof (int), sizeof (double), 16, NULL, test_eq, NULL,
                    NULL);
  assert (map == NULL);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL equality function
  map = map_create (sizeof (int), sizeof (double), 16, test_hash, NULL, NULL,
                    NULL);
  assert (map == NULL);
  assert (map_get_error () == MAP_NULL_PTR);

  TEST_PASS ("map_create");
}

static void
test_map_insert (void)
{
  map_t *map = map_create (sizeof (int), sizeof (double), 16, test_hash,
                           test_eq, NULL, NULL);
  assert (map != NULL);

  int key = 1;
  double value = 42.0;

  // test successful insert
  map_result_t result = map_insert (map, &key, &value);
  assert (result == MAP_OK);
  assert (map->size == 1);

  // test duplicate key
  result = map_insert (map, &key, &value);
  assert (result == MAP_DUPLICATE_KEY);
  assert (map_get_error () == MAP_DUPLICATE_KEY);

  // test NULL map
  result = map_insert (NULL, &key, &value);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL key
  result = map_insert (map, NULL, &value);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL value
  result = map_insert (map, &key, NULL);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  map_destroy (map);
  TEST_PASS ("map_insert");
}

static void
test_map_get (void)
{
  map_t *map = map_create (sizeof (int), sizeof (double), 16, test_hash,
                           test_eq, NULL, NULL);
  assert (map != NULL);

  int key = 1;
  double value = 42.0;
  double retrieved_value = 0.0;

  // insert a value
  map_result_t result = map_insert (map, &key, &value);
  assert (result == MAP_OK);

  // test successful get
  result = map_get (map, &key, &retrieved_value);
  assert (result == MAP_OK);
  const double epsilon = 1e-10;
  assert (fabs (retrieved_value - value) < epsilon);

  // test non-existent key
  int missing_key = 2;
  result = map_get (map, &missing_key, &retrieved_value);
  assert (result == MAP_KEY_NOT_FOUND);
  assert (map_get_error () == MAP_KEY_NOT_FOUND);

  // test NULL map
  result = map_get (NULL, &key, &retrieved_value);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL key
  result = map_get (map, NULL, &retrieved_value);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL value_out
  result = map_get (map, &key, NULL);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  map_destroy (map);
  TEST_PASS ("map_get");
}

static void
test_map_remove (void)
{
  map_t *map = map_create (sizeof (int), sizeof (double), 16, test_hash,
                           test_eq, NULL, NULL);
  assert (map != NULL);

  int key = 1;
  double value = 42.0;

  // insert a value
  map_result_t result = map_insert (map, &key, &value);
  assert (result == MAP_OK);

  // test successful remove
  result = map_remove (map, &key);
  assert (result == MAP_OK);
  assert (map->size == 0);

  // test removing non-existent key
  result = map_remove (map, &key);
  assert (result == MAP_KEY_NOT_FOUND);
  assert (map_get_error () == MAP_KEY_NOT_FOUND);

  // test NULL map
  result = map_remove (NULL, &key);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL key
  result = map_remove (map, NULL);
  assert (result == MAP_NULL_PTR);
  assert (map_get_error () == MAP_NULL_PTR);

  map_destroy (map);
  TEST_PASS ("map_remove");
}

static void
test_map_contains (void)
{
  map_t *map = map_create (sizeof (int), sizeof (double), 16, test_hash,
                           test_eq, NULL, NULL);
  assert (map != NULL);

  int key = 1;
  double value = 42.0;

  // test empty map
  assert (map_contains (map, &key) == false);

  // insert a value
  map_result_t result = map_insert (map, &key, &value);
  assert (result == MAP_OK);

  // test existing key
  assert (map_contains (map, &key) == true);

  // test non-existent key
  int missing_key = 2;
  assert (map_contains (map, &missing_key) == false);

  // test NULL map
  assert (map_contains (NULL, &key) == false);
  assert (map_get_error () == MAP_NULL_PTR);

  // test NULL key
  assert (map_contains (map, NULL) == false);
  assert (map_get_error () == MAP_NULL_PTR);

  map_destroy (map);
  TEST_PASS ("map_contains");
}

static void
test_map_size (void)
{
  map_t *map = map_create (sizeof (int), sizeof (double), 16, test_hash,
                           test_eq, NULL, NULL);
  assert (map != NULL);

  // test empty map
  assert (map_size (map) == 0);

  int key = 1;
  double value = 42.0;

  // test after insert
  map_result_t result = map_insert (map, &key, &value);
  assert (result == MAP_OK);
  assert (map_size (map) == 1);

  // test after remove
  result = map_remove (map, &key);
  assert (result == MAP_OK);
  assert (map_size (map) == 0);

  // test NULL map
  assert (map_size (NULL) == 0);

  map_destroy (map);
  TEST_PASS ("map_size");
}