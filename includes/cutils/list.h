#ifndef CUTILS_LIST_H
#define CUTILS_LIST_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct list_node
{
  void *data;
  struct list_node *next;
  struct list_node *prev;
} list_node_t;

typedef struct
{
  list_node_t *head;
  list_node_t *tail;
  size_t len;
  size_t elem_len;
  cutils_allocator_t *allocator;
} list_t;

typedef enum
{
  LIST_OK = 0,
  LIST_NULL_PTR = 1,
  LIST_NO_MEMORY = 2,
  LIST_INVALID_ARG = 3,
  LIST_OUT_OF_RANGE = 4,
  LIST_OVERFLOW = 5,
  LIST_TIMEOUT = 6,
  LIST_PRIORITY_ERROR = 7
} list_result_t;

/**
 * Gets the last list operation error.
 *
 * @return Last error code
 */
[[nodiscard]] list_result_t list_get_error (void);

/**
 * Creates a new empty list with the specified allocator.
 *
 * @param elem_len Element length
 * @param allocator Allocator to use
 * @return Newly allocated list or NULL on error
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
[[nodiscard]] list_t *
list_create_with_allocator (size_t elem_len, cutils_allocator_t *allocator);

/**
 * Creates a new empty list using the default allocator.
 *
 * @param elem_len Element length
 * @return Newly allocated list or NULL on error
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
[[nodiscard]] list_t *list_create (size_t elem_len);

/**
 * @brief Create a deep copy of a list
 *
 * This function creates a new list and copies all elements from the source
 * list. The new list will have the same size and element order as the source
 * list.
 *
 * @param list Pointer to the source list
 * @return list_t* Pointer to the new list, or NULL if allocation failed
 */
[[nodiscard]] list_t *list_copy (const list_t *list);

/**
 * Destroys list and frees all allocated memory.
 *
 * @param list List to destroy
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
void list_destroy (list_t *list);

/**
 * Inserts a new value at the front of the list with timeout.
 *
 * @param list List to insert into
 * @param elem Element to insert
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
bool list_push_front_timeout (list_t *list, const void *elem,
                              uint32_t timeout_ms);

/**
 * Inserts a new value at the front of the list.
 *
 * @param list List to insert into
 * @param elem Element to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
bool list_push_front (list_t *list, const void *elem);

/**
 * Inserts a new value at the back of the list with timeout.
 *
 * @param list List to insert into
 * @param elem Element to insert
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
bool list_push_back_timeout (list_t *list, const void *elem,
                             uint32_t timeout_ms);

/**
 * Inserts a new value at the back of the list.
 *
 * @param list List to insert into
 * @param elem Element to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 */
bool list_push_back (list_t *list, const void *elem);

/**
 * Removes and returns the first value in the list.
 *
 * @param list List to remove from
 * @param out Output parameter to store the removed value
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
bool list_pop_front (list_t *list, void *out);

/**
 * Removes and returns the last value in the list.
 *
 * @param list List to remove from
 * @param out Output parameter to store the removed value
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
bool list_pop_back (list_t *list, void *out);

/**
 * Gets the first value in the list without removing it.
 *
 * @param list List to peek from
 * @param out Output parameter to store the peeked value
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
bool list_front (const list_t *list, void *out);

/**
 * Gets the last value in the list without removing it.
 *
 * @param list List to peek from
 * @param out Output parameter to store the peeked value
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_VALUE if list is empty
 */
bool list_back (const list_t *list, void *out);

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
 * Gets the element at the specified index.
 *
 * @param list List to get from
 * @param index Index to get
 * @param out Output parameter to store the retrieved value
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_ARG if index > size
 */
bool list_get (const list_t *list, size_t index, void *out);

/**
 * Sets the element at the specified index.
 *
 * @param list List to set into
 * @param index Index to set
 * @param elem Element to set
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_ARG if index > size
 */
bool list_set (list_t *list, size_t index, const void *elem);

/**
 * Inserts a value at the specified index with timeout.
 *
 * @param list List to insert into
 * @param index Position to insert at (0 = front)
 * @param elem Element to insert
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 * @note Sets error to LIST_INVALID_ARG if index > size
 */
bool list_insert_timeout (list_t *list, size_t index, const void *elem,
                          uint32_t timeout_ms);

/**
 * Inserts a value at the specified index.
 *
 * @param list List to insert into
 * @param index Position to insert at (0 = front)
 * @param elem Element to insert
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_NO_MEMORY if allocation fails
 * @note Sets error to LIST_INVALID_ARG if index > size
 */
bool list_insert (list_t *list, size_t index, const void *elem);

/**
 * Removes the specified node from the list.
 *
 * @param list List containing the node
 * @param index Index of the node to remove
 * @param out Output parameter to store the removed value
 * @return true if successful, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 * @note Sets error to LIST_INVALID_ARG if index > size
 */
bool list_remove (list_t *list, size_t index, void *out);

/**
 * Gets the current length of the list.
 *
 * @param list List to get length from
 * @return Current length of the list
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
size_t list_length (const list_t *list);

/**
 * Gets the element size of the list.
 *
 * @param list List to get element size from
 * @return Element size of the list
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
size_t list_element_size (const list_t *list);

/**
 * Gets the memory usage statistics of the list.
 *
 * @param list List to get memory usage from
 * @return Memory usage statistics of the list
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
size_t list_memory_usage (const list_t *list);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param list List to check
 * @param required_capacity Required capacity
 * @return true if operation would succeed, false otherwise
 * @note Sets error to LIST_NULL_PTR if list is NULL
 */
bool list_can_perform_operation (const list_t *list, size_t required_capacity);

/**
 * Iterator functions
 */
typedef struct
{
  list_t *list;
  list_node_t *current;
  size_t index;
} list_iterator_t;

/**
 * Creates an iterator starting from the beginning.
 *
 * @param list List to create iterator for
 * @return Iterator starting from the beginning
 */
list_iterator_t list_begin (list_t *list);

/**
 * Creates an iterator starting from the end.
 *
 * @param list List to create iterator for
 * @return Iterator starting from the end
 */
list_iterator_t list_end (list_t *list);

/**
 * Moves the iterator to the next element.
 *
 * @param it Iterator to move
 * @return true if successful, false if end of list
 */
bool list_iterator_next (list_iterator_t *it);

/**
 * Moves the iterator to the previous element.
 *
 * @param it Iterator to move
 * @return true if successful, false if beginning of list
 */
bool list_iterator_prev (list_iterator_t *it);

/**
 * Gets the current element.
 *
 * @param it Iterator to get element from
 * @param out Output parameter to store the retrieved element
 * @return true if successful, false if end of list
 */
bool list_iterator_get (list_iterator_t *it, void *out);

/**
 * Sets the current element.
 *
 * @param it Iterator to set element for
 * @param elem Element to set
 * @return true if successful, false if end of list
 */
bool list_iterator_set (list_iterator_t *it, const void *elem);

/**
 * Checks if the iterator is valid.
 *
 * @param it Iterator to check
 * @return true if valid, false if end of list
 */
bool list_iterator_is_valid (const list_iterator_t *it);

#endif // CUTILS_LIST_H
