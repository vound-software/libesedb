#ifdef LIBESEDB_PERFORMANCE_PATCH

#include "libesedb_definitions.h"
#include "libesedb_stack_PATCH.h"
#include "memory.h"

#include <stdlib.h>

int libesedb_stack_create(
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

	*stack = memory_allocate_structure ( libesedb_stack_t );
	
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

	(*stack)->top_item = NULL;

	return 1;
}

int libesedb_stack_push(
	libesedb_stack_t *stack,
	void *node,
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

	stack_item->next = stack->top_item;
	stack_item->value = node;
	stack->top_item = stack_item;

	return 1;
}

int libesedb_stack_pop(
	libesedb_stack_t *stack,
	void **node,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_pop";
	libesedb_stack_item_t *item_to_remove = NULL;

	if (node == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: cannot retrieve node: result pointer is null.",
			function );
		
		return -1;
	}

	if (*node != NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: pointer to node is already set.",
			function );

		return -1;
	}

	if (stack->top_item == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: stack is empty.",
			function );

		return -1;
	}

	item_to_remove = stack->top_item;

	*node = item_to_remove->value;

	// unlink and deallocate the removed stack item
	stack->top_item = item_to_remove->next;
	item_to_remove->next = NULL;
	item_to_remove->value = NULL;
	memory_free(item_to_remove);

	return 1;
}

int libesedb_stack_top(
	libesedb_stack_t *stack,
	void **node,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_stack_top";

	if (node == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: cannot retrieve node: result pointer is null.",
			function );
		
		return -1;
	}
	if (*node != NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: pointer to node is already set.",
			function );

		return -1;
	}

	if (stack->top_item == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: stack is empty.",
			function );

		return -1;
	}

	*node = stack->top_item->value;
	return 1;
}

int libesedb_stack_is_empty(
	libesedb_stack_t *stack,
	int *is_empty,
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
	*is_empty = (stack->top_item == NULL);
	return 1;
}

int libesedb_stack_free(
	libesedb_stack_t **stack,
	int delete_data,
	libcerror_error_t **error)
{
	static char *function = "libesedb_stack_free";
	int result = 1;

	if (stack == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: stack is null.",
			function);
		return -1;
	}

	if (*stack != NULL) 
	{
		if (delete_data == 1) 
		{
			int is_empty = 0;

			if (libesedb_stack_is_empty(
				*stack, 
				&is_empty, 
				error) != 1) 
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: unable to pop a value from the stack.",
					function);

				result = -1;
				goto on_end;
			}

			while (!is_empty)
			{
				void *value = NULL;
				if (libesedb_stack_pop(
					*stack,
					&value,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: unable to pop a value from the stack.",
						function);

					result = -1;
					goto on_end;
				}

				memory_free(value);

				if (libesedb_stack_is_empty(
					*stack,
					&is_empty,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: unable to checke whether a stack is empty.",
						function);

					result = -1;
					goto on_end;
				}
			}
		} // if (delete_data) ...

	on_end:
		memory_free(*stack);
		*stack = NULL;
	}

	return result;
}

#endif
