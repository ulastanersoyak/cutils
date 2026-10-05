#include "cutils/hashmap.h"
#include "cutils/pool.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>

static void
test_basics (void)
{
  hashmap_t *map = hashmap_create (sizeof (int), sizeof (int), NULL, NULL);
  CHECK (map != NULL);
  clobber_stack ();

  // enough keys to grow the table several times
  for (int i = 0; i < 500; i++)
    {
      int value = i * 2;
      CHECK (hashmap_insert (map, &i, &value));
    }
  CHECK (hashmap_size (map) == 500);

  int key = 7;
  int value = 0;
  CHECK (hashmap_get (map, &key, &value) && value == 14);
  CHECK (!hashmap_insert (map, &key, &value));
  CHECK (hashmap_get_error () == HASHMAP_KEY_EXISTS);

  value = 99;
  CHECK (hashmap_insert_or_assign (map, &key, &value));
  CHECK (hashmap_get (map, &key, &value) && value == 99);
  CHECK (hashmap_size (map) == 500);

  int *ptr = hashmap_get_ptr (map, &key);
  CHECK (ptr != NULL && *ptr == 99);
  *ptr = 5;
  CHECK (hashmap_get (map, &key, &value) && value == 5);

  key = 1000;
  CHECK (!hashmap_contains (map, &key));
  CHECK (!hashmap_get (map, &key, &value));
  CHECK (hashmap_get_error () == HASHMAP_KEY_NOT_FOUND);
  hashmap_destroy (map);
}

// every insert and remove, checked against a plain array
static void
test_random_operations (void)
{
  static uint8_t memory[64 * 1024];
  cutils_static_pool_t pool;
  cutils_static_pool_init (&pool, memory, sizeof memory, 8);
  cutils_allocator_t allocator = cutils_static_pool_allocator (&pool);
  hashmap_t *map = hashmap_create_with_allocator (sizeof (int), sizeof (int),
                                                  NULL, NULL, &allocator);
  static bool present[300];
  size_t count = 0;
  srand (99);

  for (int op = 0; op < 50000; op++)
    {
      int key = rand () % 300;
      int value = key + 1;
      int out = 0;
      if (rand () % 2)
        {
          bool inserted = hashmap_insert (map, &key, &value);
          CHECK (inserted == !present[key]);
          count += inserted;
          present[key] = true;
        }
      else
        {
          bool removed = hashmap_remove (map, &key, &out);
          CHECK (removed == present[key] && (!removed || out == value));
          count -= removed;
          present[key] = false;
        }
    }

  CHECK (hashmap_size (map) == count);
  for (int key = 0; key < 300; key++)
    {
      CHECK (hashmap_contains (map, &key) == present[key]);
    }

  // iteration visits every entry exactly once
  size_t cursor = 0;
  size_t visited = 0;
  int key;
  int value;
  static bool seen[300];
  while (hashmap_next (map, &cursor, &key, &value))
    {
      CHECK (present[key] && !seen[key] && value == key + 1);
      seen[key] = true;
      visited++;
    }
  CHECK (visited == count);

  CHECK (hashmap_clear (map) && hashmap_size (map) == 0);
  key = 3;
  CHECK (!hashmap_contains (map, &key));
  hashmap_destroy (map);
}

// string keys need their own hash and equality
typedef struct
{
  char name[16];
} name_t;

static uint32_t
hash_name (const void *key)
{
  const char *s = ((const name_t *)key)->name;
  uint32_t hash = 5381;
  while (*s != '\0')
    {
      hash = hash * 33u + (unsigned char)*s++;
    }
  return hash;
}

static bool
equal_names (const void *a, const void *b)
{
  return strcmp (((const name_t *)a)->name, ((const name_t *)b)->name) == 0;
}

static void
test_custom_hash (void)
{
  hashmap_t *map
      = hashmap_create (sizeof (name_t), sizeof (int), hash_name, equal_names);
  name_t a = { "alpha" };
  name_t b;
  memset (&b, 0x7F, sizeof b); // garbage after the terminator
  strcpy (b.name, "alpha");
  int value = 1;
  CHECK (hashmap_insert (map, &a, &value));
  CHECK (hashmap_get (map, &b, &value) && value == 1);
  hashmap_destroy (map);
}

int
main (void)
{
  test_basics ();
  test_random_operations ();
  test_custom_hash ();
  return test_result ("hashmap");
}
