#if !defined(_LIBESEDB_STACK_H) && defined(LIBESEDB_PERFORMANCE_PATCH)
#define _LIBESEDB_STACK_H

#if defined( __cplusplus )
extern "C" {
#endif

typedef intptr_t stack_value_t;
typedef struct libesedb_stack_item libesedb_stack_item_t;

/*	The linked stack implementation.
	Every item contains its accompanying data (node) and 
	the pointer (next) to the next item in the list.
 */
struct libesedb_stack_item
{
	stack_value_t *value;
	libesedb_stack_item_t *next;
};

/*	Definition of the stack. 
	Contains a pointer (first) to the top of the stack.
	This stack of tree nodes is used to support the depth-first search algorithm,
	which searches and retrieves table records from the tree.
	The records of an edb table are organized into the tree structure and 
	are stored on the leaf level.
 */
typedef struct libesedb_stack libesedb_stack_t;
struct libesedb_stack
{
	libesedb_stack_item_t *first;
};

/*	Creates the empty stack. 
 */
int libesedb_tree_node_stack_create(
	libesedb_stack_t **stack,
	libcerror_error_t **error);

/*	Pushes the node "data", onto the stack denoted by the parameter "stack". 
	Returns 1 on success, otherwise -1 is returned. 
	If an error occurs, it is passed through the parameter "error". 
 */
int libesedb_tree_node_stack_push(
	libesedb_stack_t *stack,
	stack_value_t *data,
	libcerror_error_t **error);

/*	Removes a node from the top of the stack.  
	The removed node is retrieved through the pointer "removed_item".
	Returns 1 on success, otherwise -1. 
	If an error occurs, it is passed through the parameter "error".
 */
int libesedb_tree_node_stack_pop(
	libesedb_stack_t *stack,
	stack_value_t **removed_item,
	libcerror_error_t **error);

/*	Retrieves a node on the top of the stack through the parameter "top_item". 
	The node is not removed from the stack. 
	Returns 1 on success, otherwise -1 is returned. 
	If an error occurs, it is passed through the parameter "error".
 */
int libesedb_tree_node_stack_top(
	libesedb_stack_t *stack,
	stack_value_t **top_item,
	libcerror_error_t **error);

/*	Checks whether the stack "stack" is empty. 
	Returns 1 if empty, othwerwise 0. 
	If an error occurs, the function returns -1 and 
	retrieves the error message through the parameter "error". 
 */
int libesedb_tree_node_stack_is_empty(
	libesedb_stack_t *stack,
	libcerror_error_t **error);

#if defined( __cplusplus )
}
#endif

#endif