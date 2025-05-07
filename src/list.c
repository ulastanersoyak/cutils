#include "cutils/list.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <string.h>

static thread_local list_result_t g_last_error = LIST_OK;

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

list_t *
list_create_with_allocator (size_t elem_len, cutils_allocator_t *allocator)
{
  g_last_error = LIST_OK;

  if (elem_len == 0 || allocator == NULL)
    {
      g_last_error = LIST_INVALID_ARG;
      return NULL;
    }

  list_t *list
      = cutils_allocate_aligned (allocator, sizeof (list_t), CUTILS_ALIGNMENT);
  if (list == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return NULL;
    }

  list->head = NULL;
  list->tail = NULL;
  list->len = 0;
  list->elem_len = elem_len;
  list->allocator = allocator;

  return list;
}

list_t *
list_create (size_t elem_len)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return list_create_with_allocator (elem_len, &default_allocator);
}

list_t *
list_copy (const list_t *list)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return NULL;
    }

  list_t *copy = list_create_with_allocator (list->elem_len, list->allocator);
  if (copy == NULL)
    {
      return NULL;
    }

  list_node_t *current = list->head;
  while (current != NULL)
    {
      if (!list_push_back (copy, current->data))
        {
          list_destroy (copy);
          return NULL;
        }
      current = current->next;
    }

  return copy;
}

void
list_destroy (list_t *list)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return;
    }

  list_node_t *current = list->head;
  while (current != NULL)
    {
      list_node_t *next = current->next;
      cutils_deallocate (list->allocator, current->data);
      cutils_deallocate (list->allocator, current);
      current = next;
    }

  cutils_deallocate (list->allocator, list);
}

bool
list_push_front_timeout (list_t *list, const void *elem, uint32_t timeout_ms)
{
  g_last_error = LIST_OK;

  if (list == NULL || elem == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  list_node_t *node = cutils_allocate_aligned (
      list->allocator, sizeof (list_node_t), CUTILS_ALIGNMENT);
  if (node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  node->data = cutils_allocate_aligned (list->allocator, list->elem_len,
                                        CUTILS_ALIGNMENT);
  if (node->data == NULL)
    {
      cutils_deallocate (list->allocator, node);
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  if (!check_timeout ((uint32_t)start_time, timeout_ms))
    {
      cutils_deallocate (list->allocator, node->data);
      cutils_deallocate (list->allocator, node);
      g_last_error = LIST_TIMEOUT;
      return false;
    }

  memcpy (node->data, elem, list->elem_len);
  node->next = list->head;
  node->prev = NULL;

  if (list->head != NULL)
    {
      list->head->prev = node;
    }
  else
    {
      list->tail = node;
    }

  list->head = node;
  list->len++;

  return true;
}

bool
list_push_front (list_t *list, const void *elem)
{
  return list_push_front_timeout (list, elem, CUTILS_MAX_OPERATION_TIME_MS);
}

bool
list_push_back_timeout (list_t *list, const void *elem, uint32_t timeout_ms)
{
  g_last_error = LIST_OK;

  if (list == NULL || elem == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  list_node_t *node = cutils_allocate_aligned (
      list->allocator, sizeof (list_node_t), CUTILS_ALIGNMENT);
  if (node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  node->data = cutils_allocate_aligned (list->allocator, list->elem_len,
                                        CUTILS_ALIGNMENT);
  if (node->data == NULL)
    {
      cutils_deallocate (list->allocator, node);
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  if (!check_timeout ((uint32_t)start_time, timeout_ms))
    {
      cutils_deallocate (list->allocator, node->data);
      cutils_deallocate (list->allocator, node);
      g_last_error = LIST_TIMEOUT;
      return false;
    }

  memcpy (node->data, elem, list->elem_len);
  node->next = NULL;
  node->prev = list->tail;

  if (list->tail != NULL)
    {
      list->tail->next = node;
    }
  else
    {
      list->head = node;
    }

  list->tail = node;
  list->len++;

  return true;
}

bool
list_push_back (list_t *list, const void *elem)
{
  return list_push_back_timeout (list, elem, CUTILS_MAX_OPERATION_TIME_MS);
}

bool
list_pop_front (list_t *list, void *out)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (list->head == NULL)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  list_node_t *node = list->head;
  if (out != NULL)
    {
      memcpy (out, node->data, list->elem_len);
    }

  list->head = node->next;
  if (list->head != NULL)
    {
      list->head->prev = NULL;
    }
  else
    {
      list->tail = NULL;
    }

  cutils_deallocate (list->allocator, node->data);
  cutils_deallocate (list->allocator, node);
  list->len--;

  return true;
}

bool
list_pop_back (list_t *list, void *out)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (list->tail == NULL)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  list_node_t *node = list->tail;
  if (out != NULL)
    {
      memcpy (out, node->data, list->elem_len);
    }

  list->tail = node->prev;
  if (list->tail != NULL)
    {
      list->tail->next = NULL;
    }
  else
    {
      list->head = NULL;
    }

  cutils_deallocate (list->allocator, node->data);
  cutils_deallocate (list->allocator, node);
  list->len--;

  return true;
}

bool
list_get (const list_t *list, size_t index, void *out)
{
  g_last_error = LIST_OK;

  if (list == NULL || out == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (index >= list->len)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  list_node_t *current = list->head;
  for (size_t i = 0; i < index; i++)
    {
      current = current->next;
    }

  memcpy (out, current->data, list->elem_len);
  return true;
}

bool
list_set (list_t *list, size_t index, const void *elem)
{
  g_last_error = LIST_OK;

  if (list == NULL || elem == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (index >= list->len)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  list_node_t *current = list->head;
  for (size_t i = 0; i < index; i++)
    {
      current = current->next;
    }

  memcpy (current->data, elem, list->elem_len);
  return true;
}

bool
list_insert_timeout (list_t *list, size_t index, const void *elem,
                     uint32_t timeout_ms)
{
  g_last_error = LIST_OK;

  if (list == NULL || elem == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (index > list->len)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  if (index == 0)
    {
      return list_push_front_timeout (list, elem, timeout_ms);
    }

  if (index == list->len)
    {
      return list_push_back_timeout (list, elem, timeout_ms);
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  list_node_t *current = list->head;
  for (size_t i = 0; i < index; i++)
    {
      current = current->next;
    }

  list_node_t *node = cutils_allocate_aligned (
      list->allocator, sizeof (list_node_t), CUTILS_ALIGNMENT);
  if (node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  node->data = cutils_allocate_aligned (list->allocator, list->elem_len,
                                        CUTILS_ALIGNMENT);
  if (node->data == NULL)
    {
      cutils_deallocate (list->allocator, node);
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  if (!check_timeout ((uint32_t)start_time, timeout_ms))
    {
      cutils_deallocate (list->allocator, node->data);
      cutils_deallocate (list->allocator, node);
      g_last_error = LIST_TIMEOUT;
      return false;
    }

  memcpy (node->data, elem, list->elem_len);
  node->next = current;
  node->prev = current->prev;
  current->prev->next = node;
  current->prev = node;
  list->len++;

  return true;
}

bool
list_insert (list_t *list, size_t index, const void *elem)
{
  return list_insert_timeout (list, index, elem, CUTILS_MAX_OPERATION_TIME_MS);
}

bool
list_remove (list_t *list, size_t index, void *out)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (index >= list->len)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  if (index == 0)
    {
      return list_pop_front (list, out);
    }

  if (index == list->len - 1)
    {
      return list_pop_back (list, out);
    }

  list_node_t *current = list->head;
  for (size_t i = 0; i < index; i++)
    {
      current = current->next;
    }

  if (out != NULL)
    {
      memcpy (out, current->data, list->elem_len);
    }

  current->prev->next = current->next;
  current->next->prev = current->prev;

  cutils_deallocate (list->allocator, current->data);
  cutils_deallocate (list->allocator, current);
  list->len--;

  return true;
}

bool
list_clear (list_t *list)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  while (list->head != NULL)
    {
      list_node_t *node = list->head;
      list->head = node->next;
      cutils_deallocate (list->allocator, node->data);
      cutils_deallocate (list->allocator, node);
    }

  list->tail = NULL;
  list->len = 0;

  return true;
}

bool
list_is_empty (const list_t *list)
{
  g_last_error = LIST_OK;

  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return true;
    }

  return list->len == 0;
}

bool
list_front (const list_t *list, void *out)
{
  g_last_error = LIST_OK;

  if (list == NULL || out == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (list->head == NULL)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  memcpy (out, list->head->data, list->elem_len);
  return true;
}

bool
list_back (const list_t *list, void *out)
{
  g_last_error = LIST_OK;

  if (list == NULL || out == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (list->tail == NULL)
    {
      g_last_error = LIST_OUT_OF_RANGE;
      return false;
    }

  memcpy (out, list->tail->data, list->elem_len);
  return true;
}

list_result_t
list_get_error (void)
{
  return g_last_error;
}

size_t
list_length (const list_t *list)
{
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return 0;
    }
  return list->len;
}

size_t
list_element_size (const list_t *list)
{
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return 0;
    }
  return list->elem_len;
}

size_t
list_memory_usage (const list_t *list)
{
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return 0;
    }
  return sizeof (list_t)
         + (list->len * (sizeof (list_node_t) + list->elem_len));
}

bool
list_can_perform_operation (const list_t *list,
                            [[maybe_unused]] size_t required_capacity)
{
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  size_t required_memory = sizeof (list_node_t) + list->elem_len;
  return cutils_can_allocate (list->allocator, required_memory,
                              CUTILS_ALIGNMENT);
}

// Iterator implementation
list_iterator_t
list_begin (list_t *list)
{
  list_iterator_t it = { list, list ? list->head : NULL, 0 };
  return it;
}

list_iterator_t
list_end (list_t *list)
{
  list_iterator_t it
      = { list, list ? list->tail : NULL, list ? list->len - 1 : 0 };
  return it;
}

bool
list_iterator_next (list_iterator_t *it)
{
  if (it == NULL || it->list == NULL || it->current == NULL)
    {
      return false;
    }
  it->current = it->current->next;
  it->index++;
  return it->current != NULL;
}

bool
list_iterator_prev (list_iterator_t *it)
{
  if (it == NULL || it->list == NULL || it->current == NULL)
    {
      return false;
    }
  it->current = it->current->prev;
  it->index--;
  return it->current != NULL;
}

bool
list_iterator_get (list_iterator_t *it, void *out)
{
  if (it == NULL || it->list == NULL || it->current == NULL || out == NULL)
    {
      return false;
    }
  memcpy (out, it->current->data, it->list->elem_len);
  return true;
}

bool
list_iterator_set (list_iterator_t *it, const void *elem)
{
  if (it == NULL || it->list == NULL || it->current == NULL || elem == NULL)
    {
      return false;
    }
  memcpy (it->current->data, elem, it->list->elem_len);
  return true;
}

bool
list_iterator_is_valid (const list_iterator_t *it)
{
  return it != NULL && it->list != NULL && it->current != NULL;
}
