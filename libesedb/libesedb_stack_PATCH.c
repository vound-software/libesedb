
#include "libesedb_definitions.h"
#include "libesedb_stack_PATCH.h"
#include "memory.h"

#include <stdlib.h>

int libesedb_tree_node_stack_create(
	libesedb_tree_node_stack_t **stack,
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_create";
	
	if (stack == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is NULL!",
			function );

		return -1;
	}

	if (*stack != NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: invalid stack already created.",
			function );

		return -1;
	}

	*stack = memory_allocate_structure(
		libesedb_tree_node_stack_t );
	
	if (*stack == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_MEMORY,
			LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			"%s: unable to create stack.",
			function );

		return -1;
	}

	(*stack)->first = NULL;

	return 1;
}

int libesedb_tree_node_stack_push(
	libesedb_tree_node_stack_t *stack,
	libfdata_tree_node_t *node,
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_push";
	libesedb_stack_item_t *stack_item = NULL;

	if (stack == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is NULL!",
			function );

		return -1;
	}

	stack_item = memory_allocate_structure(libesedb_stack_item_t);

	if (stack_item == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_MEMORY,
			LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			"%s: unable to allocate memory for stack item.",
			function );

		return -1;
	}

	stack_item->next = stack->first;
	stack_item->node = node;
	stack->first = stack_item;

	return 1;
}

int libesedb_tree_node_stack_pop(
	libesedb_tree_node_stack_t *stack,
	libfdata_tree_node_t **node,
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_pop";
	libesedb_stack_item_t *item_to_remove = NULL;

	if (node == NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: cannot retrieve node: result pointer is null.",
			function );
		
		return -1;
	}

	if (*node != NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: pointer to node is already set.",
			function );

		return -1;
	}

	if (stack->first == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: stack is empty.",
			function );

		return -1;
	}

	item_to_remove = stack->first;

	*node = item_to_remove->node;
	stack->first = item_to_remove->next;

	item_to_remove->next = NULL;
	memory_free(item_to_remove);

	return 1;

}

int libesedb_tree_node_stack_top(
	libesedb_tree_node_stack_t *stack,
	libfdata_tree_node_t **node,
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_top";

	if (node == NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: cannot retrieve node: result pointer is null.",
			function );
		
		return -1;
	}
	if (*node != NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: pointer to node is already set.",
			function );

		return -1;
	}

	if (stack->first == NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: stack is empty.",
			function );

		return -1;
	}

	*node = stack->first->node;

	return 1;
}
/*
int libesedb_tree_node_stack_has_next(
	libesedb_tree_node_stack_t *stack)
{
	return !(libesedb_tree_node_stack_is_empty(stack));
}
*/
int libesedb_tree_node_stack_is_empty(
	libesedb_tree_node_stack_t *stack,
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_is_empty";

	if (stack == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is null.",
			function );
		return -1;
	}

	return stack->first != NULL;
}
