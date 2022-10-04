#include <stdlib.h>

#include "libesedb_definitions.h"
#include "libesedb_perf_patch_stack.h"
#include "memory.h"

#ifdef LIBESEDB_PERFORMANCE_PATCH

int libesedb_tree_node_stack_create(
	libesedb_stack_t **stack,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_create";
	
	if (stack == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is NULL!",
			function );

		return -1;
	}

	if (*stack != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: invalid stack already created.",
			function );

		return -1;
	}

	*stack = memory_allocate_structure( libesedb_stack_t );
	
	if (*stack == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_MEMORY,
			LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			"%s: unable to create stack.",
			function );

		return -1;
	}

	(*stack)->first = NULL;

	return 1;
}

int libesedb_tree_node_stack_push(
	libesedb_stack_t *stack,
	stack_value_t *node,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_push";
	libesedb_stack_item_t *stack_item = NULL;

	if (stack == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is NULL!",
			function );

		return -1;
	}

	stack_item = memory_allocate_structure(libesedb_stack_item_t);

	if (stack_item == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_MEMORY,
			LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			"%s: unable to allocate memory for stack item.",
			function );

		return -1;
	}

	stack_item->next = stack->first;
	stack_item->value = node;
	stack->first = stack_item;

	return 1;
}

int libesedb_tree_node_stack_pop(
	libesedb_stack_t *stack,
	stack_value_t **data,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_pop";
	libesedb_stack_item_t *item_to_remove = NULL;

	if (data == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: cannot retrieve node: result pointer is null.",
			function );
		
		return -1;
	}

	if (*data != NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: pointer to node is already set.",
			function );

		return -1;
	}

	if (stack->first == NULL)
	{
		*data = NULL;
		return 1;
	}

	item_to_remove = stack->first;

	*data = item_to_remove->value;
	stack->first = item_to_remove->next;

	item_to_remove->next = NULL;
	memory_free(item_to_remove);

	return 1;
}

int libesedb_tree_node_stack_top(
	libesedb_stack_t *stack,
	stack_value_t **data,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_top";

	if (data == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: cannot retrieve node: result pointer is null.",
			function );
		
		return -1;
	}
	if (*data != NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: pointer to node is already set.",
			function );

		return -1;
	}

	if (stack->first == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: stack is empty.",
			function );

		return -1;
	}

	*data = stack->first->value;

	return 1;
}


int libesedb_tree_node_stack_is_empty(
	libesedb_stack_t *stack,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_is_empty";

	if (stack == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is null.",
			function );
		return -1;
	}

	return stack->first != NULL;
}

#endif
