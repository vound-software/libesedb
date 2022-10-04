#ifdef LIBESEDB_PERFORMANCE_PATCH

#ifndef _LIBESEDB_STACK_H
#define _LIBESEDB_STACK_H

#include "libesedb_libfdata.h"
#include "libfdata_btree_node.h"

/* Stack implemented as Linked List. 
 */
typedef struct libesedb_stack_item libesedb_stack_item_t;

struct libesedb_stack_item
{
	void *value;
	libesedb_stack_item_t *next;
};

/* B-tree node element descriptor (aimed for storing on the search stack)
 */
typedef struct btree_node_info btree_node_info_t;

struct btree_node_info {
	void *node_data;
	uint8_t is_leaf_node;
};


/* Definition of stack used for implementation of B-tree search algorithms. 
 * Contains the top_item pointer to the top stack element.
 */
typedef struct libesedb_stack libesedb_stack_t;

struct libesedb_stack
{
	libesedb_stack_item_t *top_item;
};

/* Creates an empty stack. 
 */
int libesedb_stack_create(
	libesedb_stack_t **stack,
	libcerror_error_t **error);

/* Pushes data value onto the stack.
 * Returns 1 on success, otherwise -1.  
 */
int libesedb_stack_push(
	libesedb_stack_t *stack,
	void *data,
	libcerror_error_t **error);

/* Removes the top stack element and retrieves it via the "removed_item" pointer.
 * Returns 1 on success, otherwise -1. 
 */
int libesedb_stack_pop(
	libesedb_stack_t *stack,
	void **removed_item,
	libcerror_error_t **error);

/* Retrieves a pointer to the top stack element, but does not remove that element.
 * Returns 1 on success, otherwise -1 is returned. 
 */
int libesedb_stack_top(
	libesedb_stack_t *stack,
	void **top_item,
	libcerror_error_t **error);

/* Checks whether a stack is empty. 
 * Returns 1 if empty, otherwise 0. 
 * If an error occurs, the function returns -1. 
 */
int libesedb_stack_is_empty(
	libesedb_stack_t *stack,
	int *is_empty,
	libcerror_error_t **error);

/* Deallocates a stack object. 
 * Returns 1 on success, 0 on error.
 */
int libesedb_stack_free(
	libesedb_stack_t **stack,
	int delete_data,
	libcerror_error_t **error);

#endif

#endif