#ifndef CUTILS_LIST_H
#define CUTILS_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  LIST_OK,
  LIST_NULL_PTR,
  LIST_NO_MEMORY,
  LIST_INVALID_VALUE,
  LIST_INVALID_ARG,
  LIST_OUT_OF_MEMORY
} list_result_t;

typedef struct list_node
{
  struct list_node *next_node;
  struct list_node *prev_node;
  void *value;
} list_node_t;

typedef struct
{
  list_node_t *head;
  size_t len;
} list_t;

/**
 * Gets the last list operation error.
 *
 * @return Last error code
 */
[[nodiscard]] list_result_t list_get_error (void);

/**
 * Creates a new empty list.
 *
 * @return Newly allocated list or NULL on error
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
[[nodiscard]] list_t *list_create (void);

/**
 * Destroys list and frees all allocated memory.
 *
 * @param list List to destroy
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
void list_destroy (list_t *list);

/**
 * Inserts a new value at the front of the list.
 *
 * @param list List to insert into
 * @param value Value to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
bool list_push_front (list_t *list, void *value);

/**
 * Inserts a new value at the back of the list.
 *
 * @param list List to insert into
 * @param value Value to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
bool list_push_back (list_t *list, void *value);

/**
 * Removes and returns the first value in the list.
 *
 * @param list List to remove from
 * @return First value in the list or NULL if empty/error
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
[[nodiscard]] void *list_pop_front (list_t *list);

/**
 * Removes and returns the last value in the list.
 *
 * @param list List to remove from
 * @return Last value in the list or NULL if empty/error
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
[[nodiscard]] void *list_pop_back (list_t *list);

/**
 * Gets the first value in the list without removing it.
 *
 * @param list List to peek from
 * @return First value in the list or NULL if empty/error
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
[[nodiscard]] void *list_peek_front (const list_t *list);

/**
 * Gets the last value in the list without removing it.
 *
 * @param list List to peek from
 * @return Last value in the list or NULL if empty/error
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
[[nodiscard]] void *list_peek_back (const list_t *list);

/**
 * Checks if the list is empty.
 *
 * @param list List to check
 * @return true if empty, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
bool list_is_empty (const list_t *list);

/**
 * Removes all elements from the list.
 *
 * @param list List to clear
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
bool list_clear (list_t *list);

/**
 * Removes the specified node from the list.
 *
 * @param list List containing the node
 * @param node Node to remove
 * @return Value from the removed node or NULL on error
 * @note Sets error to LIST_NULL_PTR if list or node is NULL
 * @note Sets error to LIST_INVALID_ARG if node is not in list
 */
[[nodiscard]] void *list_remove_node (list_t *list, list_node_t *node);

/**
 * Inserts a value after the specified node.
 *
 * @param list List to insert into
 * @param node Node to insert after, if NULL inserts at beginning
 * @param value Value to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 * @note Sets error to LIST_INVALID_ARG if node is not in list
 */
bool list_insert_after (list_t *list, list_node_t *node, void *value);

/**
 * Inserts a value before the specified node.
 *
 * @param list List to insert into
 * @param node Node to insert before, if NULL inserts at end
 * @param value Value to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 * @note Sets error to LIST_INVALID_ARG if node is not in list
 */
bool list_insert_before (list_t *list, list_node_t *node, void *value);

/**
 * Inserts a value at the specified index.
 *
 * @param list List to insert into
 * @param index Position to insert at (0 = front)
 * @param value Value to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 * @note Sets error to LIST_INVALID_ARG if index > size
 */
bool list_insert_at (list_t *list, size_t index, void *value);

#endif // CUTILS_LIST_H
