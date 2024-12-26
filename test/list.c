#include "cutils/list.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_list_create (void);
static void test_list_push_front (void);
static void test_list_push_back (void);
static void test_list_pop_front (void);
static void test_list_pop_back (void);
static void test_list_peek (void);
static void test_list_insert (void);
static void test_list_remove (void);

void
run_list_tests (void)
{
  test_list_create ();
  test_list_push_front ();
  test_list_push_back ();
  test_list_pop_front ();
  test_list_pop_back ();
  test_list_peek ();
  test_list_insert ();
  test_list_remove ();
}

static void
test_list_create (void)
{
  // Test normal creation
  list_t *list = list_create ();
  assert (list != NULL);
  assert (list->head != NULL);
  assert (list->len == 0);
  assert (list->head->next_node == list->head);
  assert (list->head->prev_node == list->head);
  list_destroy (list);

  TEST_PASS ("list_create");
}

static void
test_list_push_front (void)
{
  list_t *list = list_create ();

  // Test basic push front
  int value1 = 42;
  assert (list_push_front (list, &value1));
  assert (list->len == 1);
  assert (*(int *)list->head->next_node->value == 42);

  // Test multiple pushes
  int value2 = 43;
  assert (list_push_front (list, &value2));
  assert (list->len == 2);
  assert (*(int *)list->head->next_node->value == 43);

  // Test null list
  assert (!list_push_front (NULL, &value1));
  assert (list_get_error () == LIST_NULL_PTR);

  list_destroy (list);
  TEST_PASS ("list_push_front");
}

static void
test_list_push_back (void)
{
  list_t *list = list_create ();

  // Test basic push back
  int value1 = 42;
  assert (list_push_back (list, &value1));
  assert (list->len == 1);
  assert (*(int *)list->head->prev_node->value == 42);

  // Test multiple pushes
  int value2 = 43;
  assert (list_push_back (list, &value2));
  assert (list->len == 2);
  assert (*(int *)list->head->prev_node->value == 43);

  // Test null list
  assert (!list_push_back (NULL, &value1));
  assert (list_get_error () == LIST_NULL_PTR);

  list_destroy (list);
  TEST_PASS ("list_push_back");
}

static void
test_list_pop_front (void)
{
  list_t *list = list_create ();

  // Test pop from empty list
  assert (list_pop_front (list) == NULL);
  assert (list_get_error () == LIST_INVALID_VALUE);

  // Test basic pop
  int value1 = 42;
  list_push_front (list, &value1);
  void *popped = list_pop_front (list);
  assert (popped != NULL);
  assert (*(int *)popped == 42);
  assert (list->len == 0);

  // Test multiple pops
  int value2 = 43;
  list_push_front (list, &value1);
  list_push_front (list, &value2);
  popped = list_pop_front (list);
  assert (*(int *)popped == 43);
  assert (list->len == 1);

  // Test null list
  assert (list_pop_front (NULL) == NULL);
  assert (list_get_error () == LIST_NULL_PTR);

  list_destroy (list);
  TEST_PASS ("list_pop_front");
}

static void
test_list_pop_back (void)
{
  list_t *list = list_create ();

  // Test pop from empty list
  assert (list_pop_back (list) == NULL);
  assert (list_get_error () == LIST_INVALID_VALUE);

  // Test basic pop
  int value1 = 42;
  list_push_back (list, &value1);
  void *popped = list_pop_back (list);
  assert (popped != NULL);
  assert (*(int *)popped == 42);
  assert (list->len == 0);

  // Test multiple pops
  int value2 = 43;
  list_push_back (list, &value1);
  list_push_back (list, &value2);
  popped = list_pop_back (list);
  assert (*(int *)popped == 43);
  assert (list->len == 1);

  // Test null list
  assert (list_pop_back (NULL) == NULL);
  assert (list_get_error () == LIST_NULL_PTR);

  list_destroy (list);
  TEST_PASS ("list_pop_back");
}

static void
test_list_peek (void)
{
  list_t *list = list_create ();

  // Test peek on empty list
  assert (list_peek_front (list) == NULL);
  assert (list_get_error () == LIST_INVALID_VALUE);
  assert (list_peek_back (list) == NULL);
  assert (list_get_error () == LIST_INVALID_VALUE);

  // Test basic peek
  int value1 = 42;
  list_push_front (list, &value1);
  assert (*(int *)list_peek_front (list) == 42);
  assert (*(int *)list_peek_back (list) == 42);

  // Test peek with multiple elements
  int value2 = 43;
  list_push_back (list, &value2);
  assert (*(int *)list_peek_front (list) == 42);
  assert (*(int *)list_peek_back (list) == 43);

  // Test null list
  assert (list_peek_front (NULL) == NULL);
  assert (list_get_error () == LIST_NULL_PTR);
  assert (list_peek_back (NULL) == NULL);
  assert (list_get_error () == LIST_NULL_PTR);

  list_destroy (list);
  TEST_PASS ("list_peek");
}

static void
test_list_insert (void)
{
  list_t *list = list_create ();

  // Test insert at index
  int value1 = 42;
  assert (list_insert_at (list, 0, &value1));
  assert (list->len == 1);
  assert (*(int *)list_peek_front (list) == 42);

  // Test insert after
  int value2 = 43;
  assert (list_insert_after (list, list->head->next_node, &value2));
  assert (list->len == 2);
  assert (*(int *)list_peek_back (list) == 43);

  // Test insert before
  int value3 = 44;
  assert (list_insert_before (list, list->head->prev_node, &value3));
  assert (list->len == 3);
  assert (*(int *)list->head->prev_node->prev_node->value == 44);

  // Test invalid index
  assert (!list_insert_at (list, 10, &value1));
  assert (list_get_error () == LIST_INVALID_ARG);

  // Test null list
  assert (!list_insert_at (NULL, 0, &value1));
  assert (list_get_error () == LIST_NULL_PTR);

  list_destroy (list);
  TEST_PASS ("list_insert");
}

static void
test_list_remove (void)
{
  list_t *list = list_create ();

  // Setup list
  int value1 = 42, value2 = 43, value3 = 44;
  list_push_back (list, &value1);
  list_push_back (list, &value2);
  list_push_back (list, &value3);

  // Test remove node
  list_node_t *node = list->head->next_node;
  void *removed = list_remove_node (list, node);
  assert (*(int *)removed == 42);
  assert (list->len == 2);

  // Test remove non-existent node
  list_node_t fake_node;
  assert (list_remove_node (list, &fake_node) == NULL);
  assert (list_get_error () == LIST_INVALID_ARG);

  // Test null cases
  assert (list_remove_node (NULL, node) == NULL);
  assert (list_get_error () == LIST_NULL_PTR);
  assert (list_remove_node (list, NULL) == NULL);
  assert (list_get_error () == LIST_NULL_PTR);

  // Test clear
  assert (list_clear (list));
  assert (list->len == 0);
  assert (list->head->next_node == list->head);
  assert (list->head->prev_node == list->head);

  list_destroy (list);
  TEST_PASS ("list_remove");
}
