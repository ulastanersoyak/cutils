#include "cutils/list.h"
#include <stdlib.h>

static thread_local list_result_t g_last_error = LIST_OK;

[[nodiscard]] list_result_t
list_get_error (void)
{
  return g_last_error;
}

[[nodiscard]] list_t *
list_create (void)
{
  g_last_error = LIST_OK;
  list_t *list = (list_t *)malloc (sizeof (list_t));
  if (list == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return NULL;
    }

  list->head = (list_node_t *)malloc (sizeof (list_node_t));
  if (list->head == NULL)
    {
      free (list);
      g_last_error = LIST_NO_MEMORY;
      return NULL;
    }

  list->head->next_node = list->head;
  list->head->prev_node = list->head;

  return list;
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

  list_node_t *current = list->head->next_node;
  while (current != list->head)
    {
      list_node_t *next = current->next_node;
      free (current);
      current = next;
    }

  free (list);
}

bool
list_push_front (list_t *list, void *value)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  list_node_t *new_node = (list_node_t *)malloc (sizeof (list_node_t));
  if (new_node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  new_node->value = value;
  new_node->next_node = list->head->next_node;
  new_node->prev_node = list->head;

  list->head->next_node->prev_node = new_node;
  list->head->next_node = new_node;
  list->len++;

  return true;
}

bool
list_push_back (list_t *list, void *value)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }
  list_node_t *new_node = (list_node_t *)malloc (sizeof (list_node_t));
  if (new_node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }
  new_node->value = value;
  new_node->next_node = list->head;
  new_node->prev_node = list->head->prev_node;
  list->head->prev_node->next_node = new_node;
  list->head->prev_node = new_node;
  list->len++;
  return true;
}

void *
list_pop_front (list_t *list)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return NULL;
    }
  if (list->len == 0)
    {
      g_last_error = LIST_INVALID_VALUE;
      return NULL;
    }
  list_node_t *node = list->head->next_node;
  void *value = node->value;

  list->head->next_node = node->next_node;
  node->next_node->prev_node = list->head;

  free (node);
  list->len--;
  return value;
}

void *
list_pop_back (list_t *list)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return NULL;
    }
  if (list->len == 0)
    {
      g_last_error = LIST_INVALID_VALUE;
      return NULL;
    }
  list_node_t *node = list->head->prev_node;
  void *value = node->value;

  node->prev_node->next_node = list->head;
  list->head->prev_node = node->prev_node;

  free (node);
  list->len--;
  return value;
}

void *
list_peek_front (const list_t *list)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return NULL;
    }
  if (list->len == 0)
    {
      g_last_error = LIST_INVALID_VALUE;
      return NULL;
    }
  return list->head->next_node->value;
}

void *
list_peek_back (const list_t *list)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return NULL;
    }
  if (list->len == 0)
    {
      g_last_error = LIST_INVALID_VALUE;
      return NULL;
    }
  return list->head->prev_node->value;
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
list_clear (list_t *list)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  list_node_t *current = list->head->next_node;
  while (current != list->head)
    {
      list_node_t *next = current->next_node;
      free (current);
      current = next;
    }

  list->head->next_node = list->head;
  list->head->prev_node = list->head;
  list->len = 0;
  return true;
}

void *
list_remove_node (list_t *list, list_node_t *node)
{
  g_last_error = LIST_OK;
  if (list == NULL || node == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return NULL;
    }

  list_node_t *current = list->head->next_node;
  bool found = false;
  while (current != list->head)
    {
      if (current == node)
        {
          found = true;
          break;
        }
      current = current->next_node;
    }

  if (!found)
    {
      g_last_error = LIST_INVALID_ARG;
      return NULL;
    }

  void *value = node->value;
  node->prev_node->next_node = node->next_node;
  node->next_node->prev_node = node->prev_node;
  free (node);
  list->len--;
  return value;
}

bool
list_insert_after (list_t *list, list_node_t *node, void *value)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (node == NULL)
    {
      return list_push_front (list, value);
    }

  list_node_t *current = list->head->next_node;
  bool found = false;
  while (current != list->head)
    {
      if (current == node)
        {
          found = true;
          break;
        }
      current = current->next_node;
    }

  if (!found)
    {
      g_last_error = LIST_INVALID_ARG;
      return false;
    }

  list_node_t *new_node = (list_node_t *)malloc (sizeof (list_node_t));
  if (new_node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  new_node->value = value;
  new_node->next_node = node->next_node;
  new_node->prev_node = node;
  node->next_node->prev_node = new_node;
  node->next_node = new_node;

  list->len++;
  return true;
}

bool
list_insert_before (list_t *list, list_node_t *node, void *value)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (node == NULL)
    {
      return list_push_back (list, value);
    }

  list_node_t *current = list->head->next_node;
  bool found = false;
  while (current != list->head)
    {
      if (current == node)
        {
          found = true;
          break;
        }
      current = current->next_node;
    }

  if (!found)
    {
      g_last_error = LIST_INVALID_ARG;
      return false;
    }

  list_node_t *new_node = (list_node_t *)malloc (sizeof (list_node_t));
  if (new_node == NULL)
    {
      g_last_error = LIST_NO_MEMORY;
      return false;
    }

  new_node->value = value;
  new_node->next_node = node;
  new_node->prev_node = node->prev_node;
  node->prev_node->next_node = new_node;
  node->prev_node = new_node;

  list->len++;
  return true;
}

bool
list_insert_at (list_t *list, size_t index, void *value)
{
  g_last_error = LIST_OK;
  if (list == NULL)
    {
      g_last_error = LIST_NULL_PTR;
      return false;
    }

  if (index > list->len)
    {
      g_last_error = LIST_INVALID_ARG;
      return false;
    }

  if (index == 0)
    {
      return list_push_front (list, value);
    }

  if (index == list->len)
    {
      return list_push_back (list, value);
    }

  list_node_t *current = list->head->next_node;
  for (size_t i = 0; i < index; i++)
    {
      current = current->next_node;
    }

  return list_insert_before (list, current, value);
}
