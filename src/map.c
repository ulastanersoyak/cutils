#include "cutils/map.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local map_result_t g_last_error = MAP_OK;

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

// Red-black tree helper functions
static void
rotate_left (map_t *map, map_node_t *node)
{
  map_node_t *right = node->right;
  node->right = right->left;

  if (right->left != NULL)
    {
      right->left->parent = node;
    }

  right->parent = node->parent;

  if (node->parent == NULL)
    {
      map->root = right;
    }
  else if (node == node->parent->left)
    {
      node->parent->left = right;
    }
  else
    {
      node->parent->right = right;
    }

  right->left = node;
  node->parent = right;
}

static void
rotate_right (map_t *map, map_node_t *node)
{
  map_node_t *left = node->left;
  node->left = left->right;

  if (left->right != NULL)
    {
      left->right->parent = node;
    }

  left->parent = node->parent;

  if (node->parent == NULL)
    {
      map->root = left;
    }
  else if (node == node->parent->right)
    {
      node->parent->right = left;
    }
  else
    {
      node->parent->left = left;
    }

  left->right = node;
  node->parent = left;
}

static void
fix_insert (map_t *map, map_node_t *node)
{
  while (node != map->root && node->parent->is_red)
    {
      if (node->parent == node->parent->parent->left)
        {
          map_node_t *uncle = node->parent->parent->right;

          if (uncle != NULL && uncle->is_red)
            {
              node->parent->is_red = false;
              uncle->is_red = false;
              node->parent->parent->is_red = true;
              node = node->parent->parent;
            }
          else
            {
              if (node == node->parent->right)
                {
                  node = node->parent;
                  rotate_left (map, node);
                }

              node->parent->is_red = false;
              node->parent->parent->is_red = true;
              rotate_right (map, node->parent->parent);
            }
        }
      else
        {
          map_node_t *uncle = node->parent->parent->left;

          if (uncle != NULL && uncle->is_red)
            {
              node->parent->is_red = false;
              uncle->is_red = false;
              node->parent->parent->is_red = true;
              node = node->parent->parent;
            }
          else
            {
              if (node == node->parent->left)
                {
                  node = node->parent;
                  rotate_right (map, node);
                }

              node->parent->is_red = false;
              node->parent->parent->is_red = true;
              rotate_left (map, node->parent->parent);
            }
        }
    }

  map->root->is_red = false;
}

static map_node_t *
find_node (const map_t *map, const void *key)
{
  map_node_t *current = map->root;

  while (current != NULL)
    {
      int cmp = map->compare (key, current->key);

      if (cmp == 0)
        {
          return current;
        }
      else if (cmp < 0)
        {
          current = current->left;
        }
      else
        {
          current = current->right;
        }
    }

  return NULL;
}

map_t *
map_create_with_allocator (size_t key_size, size_t value_size,
                           int (*compare) (const void *a, const void *b),
                           cutils_allocator_t *allocator)
{
  g_last_error = MAP_OK;

  if (key_size == 0 || value_size == 0 || compare == NULL || allocator == NULL)
    {
      g_last_error = MAP_INVALID_ARG;
      return NULL;
    }

  map_t *map
      = cutils_allocate_aligned (allocator, sizeof (map_t), CUTILS_ALIGNMENT);
  if (map == NULL)
    {
      g_last_error = MAP_NO_MEMORY;
      return NULL;
    }

  map->root = NULL;
  map->size = 0;
  map->key_size = key_size;
  map->value_size = value_size;
  map->allocator = allocator;
  map->compare = compare;

  return map;
}

map_t *
map_create (size_t key_size, size_t value_size,
            int (*compare) (const void *a, const void *b))
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return map_create_with_allocator (key_size, value_size, compare,
                                    &default_allocator);
}

static void
destroy_node (map_t *map, map_node_t *node)
{
  if (node == NULL)
    {
      return;
    }

  destroy_node (map, node->left);
  destroy_node (map, node->right);

  cutils_deallocate (map->allocator, node->key);
  cutils_deallocate (map->allocator, node->value);
  cutils_deallocate (map->allocator, node);
}

void
map_destroy (map_t *map)
{
  g_last_error = MAP_OK;

  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return;
    }

  destroy_node (map, map->root);
  cutils_deallocate (map->allocator, map);
}

bool
map_insert_timeout (map_t *map, const void *key, const void *value,
                    uint32_t timeout_ms)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL || value == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  // Check if key already exists
  if (find_node (map, key) != NULL)
    {
      g_last_error = MAP_KEY_EXISTS;
      return false;
    }

  // Create new node
  map_node_t *node = cutils_allocate_aligned (
      map->allocator, sizeof (map_node_t), CUTILS_ALIGNMENT);
  if (node == NULL)
    {
      g_last_error = MAP_NO_MEMORY;
      return false;
    }

  node->key = cutils_allocate_aligned (map->allocator, map->key_size,
                                       CUTILS_ALIGNMENT);
  if (node->key == NULL)
    {
      cutils_deallocate (map->allocator, node);
      g_last_error = MAP_NO_MEMORY;
      return false;
    }

  node->value = cutils_allocate_aligned (map->allocator, map->value_size,
                                         CUTILS_ALIGNMENT);
  if (node->value == NULL)
    {
      cutils_deallocate (map->allocator, node->key);
      cutils_deallocate (map->allocator, node);
      g_last_error = MAP_NO_MEMORY;
      return false;
    }

  if (!check_timeout ((uint32_t)start_time, timeout_ms))
    {
      cutils_deallocate (map->allocator, node->value);
      cutils_deallocate (map->allocator, node->key);
      cutils_deallocate (map->allocator, node);
      g_last_error = MAP_TIMEOUT;
      return false;
    }

  memcpy (node->key, key, map->key_size);
  memcpy (node->value, value, map->value_size);
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->is_red = true;

  // Insert node
  map_node_t *current = map->root;
  map_node_t *parent = NULL;

  while (current != NULL)
    {
      parent = current;
      if (map->compare (key, current->key) < 0)
        {
          current = current->left;
        }
      else
        {
          current = current->right;
        }
    }

  node->parent = parent;

  if (parent == NULL)
    {
      map->root = node;
    }
  else if (map->compare (key, parent->key) < 0)
    {
      parent->left = node;
    }
  else
    {
      parent->right = node;
    }

  fix_insert (map, node);
  map->size++;

  return true;
}

bool
map_insert (map_t *map, const void *key, const void *value)
{
  return map_insert_timeout (map, key, value, CUTILS_MAX_OPERATION_TIME_MS);
}

bool
map_remove (map_t *map, const void *key, void *out_value)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  map_node_t *node = find_node (map, key);
  if (node == NULL)
    {
      g_last_error = MAP_KEY_NOT_FOUND;
      return false;
    }

  if (out_value != NULL)
    {
      memcpy (out_value, node->value, map->value_size);
    }

  // TODO: Implement red-black tree deletion
  // This is a simplified version that doesn't maintain red-black properties

  if (node->left == NULL)
    {
      if (node->parent == NULL)
        {
          map->root = node->right;
        }
      else if (node == node->parent->left)
        {
          node->parent->left = node->right;
        }
      else
        {
          node->parent->right = node->right;
        }

      if (node->right != NULL)
        {
          node->right->parent = node->parent;
        }
    }
  else if (node->right == NULL)
    {
      if (node->parent == NULL)
        {
          map->root = node->left;
        }
      else if (node == node->parent->left)
        {
          node->parent->left = node->left;
        }
      else
        {
          node->parent->right = node->left;
        }

      if (node->left != NULL)
        {
          node->left->parent = node->parent;
        }
    }
  else
    {
      map_node_t *successor = node->right;

      while (successor->left != NULL)
        {
          successor = successor->left;
        }

      if (successor != node->right)
        {
          successor->parent->left = successor->right;
          if (successor->right != NULL)
            {
              successor->right->parent = successor->parent;
            }
          successor->right = node->right;
          node->right->parent = successor;
        }

      if (node->parent == NULL)
        {
          map->root = successor;
        }
      else if (node == node->parent->left)
        {
          node->parent->left = successor;
        }
      else
        {
          node->parent->right = successor;
        }

      successor->parent = node->parent;
      successor->left = node->left;
      node->left->parent = successor;
    }

  cutils_deallocate (map->allocator, node->key);
  cutils_deallocate (map->allocator, node->value);
  cutils_deallocate (map->allocator, node);
  map->size--;

  return true;
}

bool
map_get (const map_t *map, const void *key, void *out_value)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL || out_value == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  map_node_t *node = find_node (map, key);
  if (node == NULL)
    {
      g_last_error = MAP_KEY_NOT_FOUND;
      return false;
    }

  memcpy (out_value, node->value, map->value_size);
  return true;
}

bool
map_contains (const map_t *map, const void *key)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  return find_node (map, key) != NULL;
}

size_t
map_size (const map_t *map)
{
  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return 0;
    }
  return map->size;
}

bool
map_is_empty (const map_t *map)
{
  g_last_error = MAP_OK;

  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return true;
    }

  return map->size == 0;
}

bool
map_clear (map_t *map)
{
  g_last_error = MAP_OK;

  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  destroy_node (map, map->root);
  map->root = NULL;
  map->size = 0;

  return true;
}

size_t
map_memory_usage (const map_t *map)
{
  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return 0;
    }
  return sizeof (map_t)
         + (map->size
            * (sizeof (map_node_t) + map->key_size + map->value_size));
}

bool
map_can_perform_operation (const map_t *map,
                           [[maybe_unused]] size_t required_capacity)
{
  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  size_t required_memory
      = sizeof (map_node_t) + map->key_size + map->value_size;
  return cutils_can_allocate (map->allocator, required_memory,
                              CUTILS_ALIGNMENT);
}

map_result_t
map_get_error (void)
{
  return g_last_error;
}

// Iterator implementation
static map_node_t *
find_min (map_node_t *node)
{
  while (node->left != NULL)
    {
      node = node->left;
    }
  return node;
}

static map_node_t *
find_max (map_node_t *node)
{
  while (node->right != NULL)
    {
      node = node->right;
    }
  return node;
}

map_iterator_t
map_begin (map_t *map)
{
  map_iterator_t it = { map, NULL, false };

  if (map != NULL && map->root != NULL)
    {
      it.current = find_min (map->root);
      it.is_valid = true;
    }

  return it;
}

map_iterator_t
map_end (map_t *map)
{
  map_iterator_t it = { map, NULL, false };

  if (map != NULL && map->root != NULL)
    {
      it.current = find_max (map->root);
      it.is_valid = true;
    }

  return it;
}

bool
map_iterator_next (map_iterator_t *it)
{
  if (it == NULL || it->map == NULL || it->current == NULL)
    {
      return false;
    }

  if (it->current->right != NULL)
    {
      it->current = find_min (it->current->right);
    }
  else
    {
      map_node_t *parent = it->current->parent;
      while (parent != NULL && it->current == parent->right)
        {
          it->current = parent;
          parent = parent->parent;
        }
      it->current = parent;
    }

  it->is_valid = it->current != NULL;
  return it->is_valid;
}

bool
map_iterator_prev (map_iterator_t *it)
{
  if (it == NULL || it->map == NULL || it->current == NULL)
    {
      return false;
    }

  if (it->current->left != NULL)
    {
      it->current = find_max (it->current->left);
    }
  else
    {
      map_node_t *parent = it->current->parent;
      while (parent != NULL && it->current == parent->left)
        {
          it->current = parent;
          parent = parent->parent;
        }
      it->current = parent;
    }

  it->is_valid = it->current != NULL;
  return it->is_valid;
}

bool
map_iterator_get (map_iterator_t *it, void *out_key, void *out_value)
{
  if (it == NULL || it->map == NULL || it->current == NULL || out_key == NULL
      || out_value == NULL)
    {
      return false;
    }

  memcpy (out_key, it->current->key, it->map->key_size);
  memcpy (out_value, it->current->value, it->map->value_size);
  return true;
}

bool
map_iterator_set (map_iterator_t *it, const void *value)
{
  if (it == NULL || it->map == NULL || it->current == NULL || value == NULL)
    {
      return false;
    }

  memcpy (it->current->value, value, it->map->value_size);
  return true;
}

bool
map_iterator_is_valid (const map_iterator_t *it)
{
  return it != NULL && it->map != NULL && it->current != NULL && it->is_valid;
}
