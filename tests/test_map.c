#include "cutils/map.h"
#include "cutils/pool.h"
#include "test.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int
rb_check (map_node_t *n, map_node_t *parent)
{
  if (!n)
    return 1;
  if (n->parent != parent)
    return -1;
  if (n->is_red
      && ((n->left && n->left->is_red) || (n->right && n->right->is_red)))
    return -1;
  int l = rb_check (n->left, n), r = rb_check (n->right, n);
  if (l < 0 || r < 0 || l != r)
    return -1;
  return l + !n->is_red;
}

static void
test_map (void)
{
  map_t *m = map_create (sizeof (int), sizeof (int), compare_ints);
  CHECK (m != NULL);
  clobber_stack ();
  for (int i = 0; i < 200; i++)
    {
      int v = i * 10;
      CHECK (map_insert (m, &i, &v));
    }
  CHECK (rb_check (m->root, NULL) > 0);
  srand (7);
  int removed = 0;
  for (int i = 0; i < 400; i++)
    {
      int k = rand () % 200;
      if (map_remove (m, &k, NULL))
        removed++;
      if (rb_check (m->root, NULL) <= 0)
        {
          CHECK (!"red black property broken after remove");
          break;
        }
    }
  CHECK (map_size (m) == (size_t)(200 - removed));
  // in order iteration stays sorted
  map_iterator_t it = map_begin (m);
  int prev = -1, k, v, count = 0;
  while (map_iterator_is_valid (&it))
    {
      map_iterator_get (&it, &k, &v);
      CHECK (k > prev && v == k * 10);
      prev = k;
      count++;
      map_iterator_next (&it);
    }
  CHECK ((size_t)count == map_size (m));
  size_t before = map_size (m);
  int key = 1000, val = 1, got = 0;
  CHECK (map_insert_or_assign (m, &key, &val) && map_size (m) == before + 1);
  val = 2;
  CHECK (map_insert_or_assign (m, &key, &val) && map_size (m) == before + 1);
  CHECK (map_get (m, &key, &got) && got == 2);
  CHECK (!map_insert (m, &key, &val) && map_get_error () == MAP_KEY_EXISTS);
  map_destroy (m);
}

// random inserts and removes, checking the red-black rules and the contents
// against a plain array after every operation. runs on a block pool because
// the default static pool never reclaims removed nodes
static void
test_map_stress (void)
{
  static uint8_t memory[128 * 1024];
  cutils_block_pool_t pool;
  CHECK (cutils_block_pool_init (&pool, memory, sizeof memory, 64));
  cutils_allocator_t allocator = cutils_block_pool_allocator (&pool);
  map_t *map = map_create_with_allocator (sizeof (int), sizeof (int),
                                          compare_ints, &allocator);
  static bool present[500];
  size_t count = 0;
  srand (12345);

  for (int op = 0; op < 20000; op++)
    {
      int key = rand () % 500;
      int value = key * 3;
      int out = -1;
      if (rand () % 2)
        {
          bool inserted = map_insert (map, &key, &value);
          CHECK (inserted == !present[key]);
          if (inserted)
            {
              present[key] = true;
              count++;
            }
        }
      else
        {
          bool removed = map_remove (map, &key, &out);
          CHECK (removed == present[key] && (!removed || out == value));
          if (removed)
            {
              present[key] = false;
              count--;
            }
        }
      if (rb_check (map->root, NULL) <= 0 || map_size (map) != count)
        {
          CHECK (!"map invariant broken");
          break;
        }
    }

  for (int key = 0; key < 500; key++)
    {
      CHECK (map_contains (map, &key) == present[key]);
    }
  map_destroy (map);
}

static void
test_ordered_queries (void)
{
  map_t *map = map_create (sizeof (int), sizeof (int), compare_ints);
  // keys 0, 10, 20, ..., 90
  for (int i = 0; i < 10; i++)
    {
      int key = i * 10;
      CHECK (map_insert (map, &key, &i));
    }

  int key = 30;
  int k;
  int v;
  map_iterator_t it = map_find (map, &key);
  CHECK (map_iterator_get (&it, &k, &v) && k == 30 && v == 3);
  key = 35;
  it = map_find (map, &key);
  CHECK (!map_iterator_is_valid (&it));

  it = map_lower_bound (map, &key);
  CHECK (map_iterator_get (&it, &k, &v) && k == 40);
  key = 40;
  it = map_lower_bound (map, &key);
  CHECK (map_iterator_get (&it, &k, &v) && k == 40);
  it = map_upper_bound (map, &key);
  CHECK (map_iterator_get (&it, &k, &v) && k == 50);

  // walk the range [15, 55)
  key = 15;
  int sum = 0;
  for (it = map_lower_bound (map, &key);
       map_iterator_get (&it, &k, &v) && k < 55; map_iterator_next (&it))
    {
      sum += k;
    }
  CHECK (sum == 20 + 30 + 40 + 50);

  key = 90;
  it = map_upper_bound (map, &key);
  CHECK (!map_iterator_is_valid (&it));
  key = -5;
  it = map_lower_bound (map, &key);
  CHECK (map_iterator_get (&it, &k, &v) && k == 0);
  map_destroy (map);
}

int
main (void)
{
  test_map ();
  test_map_stress ();
  test_ordered_queries ();
  return test_result ("map");
}
