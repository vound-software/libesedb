#ifdef LIBESEDB_PERFORMANCE_PATCH

#include "byte_stream.h"
#include "libcdata_array.h"
#include "libfdata_btree.h"
#include "libfdata_btree_node.h"
#include "libesedb_definitions.h"
#include "libesedb_data_segment.h"
#include "libesedb_long_value.h"
#include "libesedb_page_tree.h"
#include "libesedb_record.h"
#include "libesedb_table.h"
#include "libesedb_key.h"
#include "memory.h"
#include "libesedb_performance_patch.h"
#include <stdlib.h>

int libfdata_btree_read_record_definition(
	libesedb_page_tree_t *page_tree,
	libfdata_btree_range_t *leaf_value_data_range,
	libesedb_data_definition_t **record_data_definition,
	libcerror_error_t **error)
{
	static char *function = "libesedb_page_tree_read_leaf_value";
	off64_t page_offset = 0;
	uint64_t page_number = 0;
	int leaf_value_data_file_index;
	off64_t leaf_value_data_offset;
	size64_t leaf_value_data_size;
	intptr_t *key_value = NULL;
	uint32_t leaf_value_data_flags = 0;
	libesedb_data_definition_t *data_definition = NULL;

	if (page_tree == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid page tree.",
			function);

		return(-1);
	}
	if (page_tree->io_handle == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid page tree - missing IO handle.",
			function);

		return(-1);
	}
	if (page_tree->io_handle->page_size == 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid page tree - invalid IO handle - missing page size.",
			function);

		return(-1);
	}
	if (libfdata_btree_range_get(
		leaf_value_data_range,
		&leaf_value_data_file_index,
		&leaf_value_data_offset,
		&leaf_value_data_size,
		&leaf_value_data_flags,
		&key_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve leaf value data range values.",
			function);

		return -1;
	}

	if (leaf_value_data_file_index < 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid leaf value data file index value less than zero.",
			function);

		return(-1);
	}
	if (leaf_value_data_offset < 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid leaf value data offset value less than zero.",
			function);

		return(-1);
	}
	if (leaf_value_data_size > (off64_t)UINT16_MAX)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid leaf value data size value exceeds maximum.",
			function);

		return(-1);
	}
	page_offset = leaf_value_data_offset / page_tree->io_handle->page_size;
	page_number = (uint64_t)(page_offset + 1);
	page_offset *= page_tree->io_handle->page_size;

	if (page_number > (uint64_t)UINT32_MAX)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid page number value exceeds maximum.",
			function);

		return(-1);
	}

	if (libesedb_data_definition_initialize(
		&data_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create data definition.",
			function);

		goto on_error;
	}

	if (data_definition == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: missing data definition.",
			function);

		goto on_error;
	}

	/* leaf_value_data_file_index contains the page value index
	 */
	data_definition->page_value_index = (uint16_t)leaf_value_data_file_index;
	data_definition->page_offset = page_offset;
	data_definition->page_number = (uint32_t)page_number;
	data_definition->data_offset = (uint16_t)(leaf_value_data_offset - page_offset);
	data_definition->data_size = (uint16_t)leaf_value_data_size;

	*record_data_definition = data_definition;

	{
		int exit_status = 1;
		goto on_exit;

	on_error:
		exit_status = -1;
		if (libesedb_data_definition_free(
			&data_definition,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to free a data definition object.",
				function);

			exit_status = -1;
		}

	on_exit: 
		return exit_status;
	}	
}

/* Retrieve and place leaf values of current_btree_node 
 * onto the table->record_definitions_stack.
 */
int traverse_table_records_btree_leaf_node(
	libesedb_internal_table_t *table,
	libfdata_btree_node_t *current_btree_node,
	libcerror_error_t **error
)
{
	static char *function = "traverse_btree_leaf_node";
	int number_of_leaf_values;
	int leaf_value_index;
	libfdata_internal_btree_t *internal_tree = (libfdata_internal_btree_t *)table->table_values_tree;

	if (libfdata_btree_node_get_number_of_leaf_values(
		current_btree_node,
		&number_of_leaf_values,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve number of leaf values.",
			function);

		return -1;
	}

	for (leaf_value_index = number_of_leaf_values - 1;
		leaf_value_index >= 0;
		leaf_value_index--)
	{
		libfdata_btree_range_t *leaf_value_data_range = NULL;
		libesedb_data_definition_t *record_definition = NULL;

		if (libfdata_btree_node_get_leaf_value_data_range_by_index(
			current_btree_node,
			leaf_value_index,
			&leaf_value_data_range,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve leaf value: %d data range.",
				function,
				leaf_value_index);

			return -1;
		}
		if (leaf_value_data_range == NULL)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: missing leaf value: %d data range.",
				function,
				leaf_value_index);

			return -1;
		}
		if (libfdata_btree_read_record_definition(
			(libesedb_page_tree_t*)(internal_tree->data_handle),
			leaf_value_data_range,
			&record_definition,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_IO,
				LIBCERROR_IO_ERROR_READ_FAILED,
				"%s: unable to read leaf value.",
				function);

			return(-1);
		}

		// Put the new leaf value onto the search stack. 
		if (libesedb_stack_push(
			table->record_definitions_stack,
			record_definition,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to a record definitions onto the stack.",
				function);

			return -1;
		}
	} // END FOR.
	return 1;
}

int libesedb_key_clone(
	libesedb_key_t *key,
	libesedb_key_t **key_clone,
	libcerror_error_t **error)
{
	static char *function = "libesedb_key_clone";
	libesedb_key_t *tmp_key_clone = NULL;

	if (key == NULL)
	{
		*key_clone = NULL;
		return 1;
	}

	if (key_clone == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: key clone pointer is null",
			function);
		return -1;
	}
	if (*key_clone != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: key clone is already set",
			function);
		return -1;
	}

	tmp_key_clone = memory_allocate_structure(libesedb_key_t);

	if (tmp_key_clone == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: unable to allocate memory for a libesedb key.",
			function);
		return -1;
	}

	// copy scalar fields' values
	(*tmp_key_clone) = (*key);

	// deep copy dynamic data array
	tmp_key_clone->data = memory_allocate(sizeof(uint8_t) * key->data_size);

	if (tmp_key_clone->data == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: unable to allocate memory for a libesedb key data",
			function);
		goto on_error;
	}

	// copy data bytes
	for (int i = 0; i < key->data_size; i++)
	{
		tmp_key_clone->data[i] = key->data[i];
	}

	// when a clone is ready, pass it on through the output parameter key_clone
	*key_clone = tmp_key_clone;
	return 1;

on_error:
	if (tmp_key_clone != NULL)
	{
		memory_free(tmp_key_clone);
	}
	return -1;
}

int libesedb_libfdata_btree_range_clone(
	libfdata_btree_range_t *btree_range,
	libfdata_btree_range_t **btree_range_clone,
	libcerror_error_t **error)
{
	static char *function = "libesedb_libfdata_btree_range_clone";

	if (libfdata_btree_range_clone(
		btree_range_clone,
		btree_range,
		error) == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: unable to clone a btree range object.",
			function);

		return -1;
	}

	// Clone the key_value field value
	if (libesedb_key_clone(
		(libesedb_key_t*)(btree_range->key_value),
		(libesedb_key_t**)(&((*btree_range_clone)->key_value)),
		error) == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: unable to allocate a btree range key value.",
			function);

		goto on_error;
	}

	return 1;

on_error:
	if (*btree_range_clone != NULL)
	{
		if (libfdata_btree_range_free(
			btree_range_clone, 
			error) == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: unable to deallocate a btree range object.",
				function);
		}
	}
	return -1;
}

/* Retrieves ranges values of child nodes of current_bree_node and
 * places them onto tables->btree_nodes_ranges_stack.
 */
int traverse_table_records_btree_branch_node(
	libesedb_internal_table_t *table,
	libfdata_btree_node_t *current_btree_node,
	libcerror_error_t **error)
{
	static char *function = "traverse_btree_branching_node";
	int number_of_sub_nodes = 0;
	int sub_node_index;
	libfdata_btree_range_t *sub_node_data_range_clone = NULL;

	if (libfdata_btree_node_get_number_of_sub_nodes(
		current_btree_node,
		&number_of_sub_nodes,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve number of entries from sub nodes array.",
			function);

		return -1;
	}

	for (sub_node_index = number_of_sub_nodes - 1;
		sub_node_index >= 0;
		sub_node_index--)
	{
		libfdata_btree_range_t *sub_node_data_range_original = NULL;
		sub_node_data_range_clone = NULL;

		if (libfdata_btree_node_get_sub_node_data_range_by_index(
			current_btree_node,
			sub_node_index,
			&sub_node_data_range_original,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve sub node: %d data range.",
				function,
				sub_node_index);

			goto on_error;
		}

		if (sub_node_data_range_original == NULL)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: unable to clone a btree range object: index = %d.",
				function,
				sub_node_index);

			goto on_error;
		}

		/* Create an independent clone of the original btree range structure.
		 */
		if (libesedb_libfdata_btree_range_clone(
			sub_node_data_range_original,
			&sub_node_data_range_clone,
			error) == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: missing sub node: %d data range.",
				function,
				sub_node_index);

			goto on_error;
		}

		if (libesedb_stack_push(
			table->nodes_stack,
			sub_node_data_range_clone,
			error) == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to push a btree node range object onto the nodes stack.",
				function);

			goto on_error;
		}
	}
	return 1;

on_error:
	if (sub_node_data_range_clone != NULL)
	{
		if (libfdata_btree_range_free(
			&sub_node_data_range_clone, 
			error) == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to deallocate a btree node range.",
				function);
		}
	}
	return -1;
}

int load_btree_tree_node(
	libesedb_internal_table_t *table,
	libfdata_btree_range_t *sub_node_data_range,
	libfdata_btree_node_t **sub_node,
	libcerror_error_t **error)
{
	static char *function = "libesedb_search_next_leaf_node";
	off64_t node_data_offset = 0;
	size64_t node_data_size = 0;
	uint32_t node_data_flags = 0;
	int node_data_file_index = -1;
	intptr_t *key_value = NULL;
	libfdata_internal_btree_t *internal_tree;

	if (table == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: table is NULL!",
			function);

		return -1;
	}
	if (table->table_values_tree == NULL) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid tree - missing root node data range.",
			function);

		return(-1);
	}
	if (sub_node_data_range == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: subnode data range is NULL!",
			function);

		return -1;
	}
	if (sub_node == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: subnode pointer is NULL!",
			function);

		return -1;
	}
	if (*sub_node != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: subnode already set!",
			function);

		return -1;
	}

	internal_tree = (libfdata_internal_btree_t *)table->table_values_tree;

	if (libfdata_btree_range_get(
		sub_node_data_range,
		&node_data_file_index,
		&node_data_offset,
		&node_data_size,
		&node_data_flags,
		&key_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve data range from tree node.",
			function);

		goto on_error;
	}

	/* Make sure not to have the node cached here to prevent it from
	* being freed during recursing the sub tree.
	*/
	if (libfdata_btree_read_node(
		internal_tree,
		table->file_io_handle,
		table->table_values_cache,
		sub_node_data_range,
		0,
		sub_node,
		LIBFDATA_READ_FLAG_NO_CACHE | LIBFDATA_READ_FLAG_IGNORE_CACHE,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			LIBCERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read node.",
			function);

		goto on_error;
	}

	{
		int exit_status = 1;
		goto on_exit;

	on_error:
		exit_status = -1;
		if (libfdata_btree_node_free(
			sub_node,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to deallocate a record data definition",
				function);
			exit_status = -1;
		}

	on_exit:
		return exit_status;
	}
}

#if defined(DEBUG_LIBESEDB_PERFRORMANCE_PATCH)
int detach_sub_nodes(
	libfdata_btree_node_t *node)
{
	if (node != NULL) 
	{
		libfdata_internal_btree_node_t *internal_btree_node = (libfdata_internal_btree_node_t *)node;
		internal_btree_node->leaf_value_ranges_array = NULL;
		internal_btree_node->sub_node_ranges_array = NULL;
	}
	return 1;
}
#endif


/*	Performs depth-first B-tree search algorithm to find and retrieve next record data definition object.
 *  Record definition nodes are leaf nodes in the B-Tree. 
 *	Returns 1 on success, otherwise -1.
 */
int libesedb_search_next_record_definition (
	libesedb_internal_table_t *table,
	libesedb_data_definition_t **record_data_definition,
	libcerror_error_t **error)
{
	static char *function = "libesedb_search_next_leaf_node";
	int exit_status = 1;

	if (table == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: table is NULL!",
			function);

		return -1;
	}

	if (table->table_values_tree == NULL) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid tree - missing root node data range.",
			function);

		return(-1);
	}

	if (record_data_definition == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: record pointer address is null!",
			function );

		return -1;
	}

	if (*record_data_definition != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: the pointer to the record is already set.",
			function );

		return -1;
	}

	while (1)
	{
		libfdata_btree_range_t *current_btree_range = NULL;
		libfdata_btree_node_t *current_btree_node = NULL;
		int is_stack_empty;
		int is_leaf_node;

		// Check if any record definition is already available.
		if (libesedb_stack_is_empty(
			table->record_definitions_stack,
			&is_stack_empty,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to access the record definitions search stack.",
				function);

			exit_status = -1;
			goto on_resource_deallocation;
		}

		// If there are some record definitions on the search stack (stack is not empty),
		// pop and retrieve the top one.
		if (!is_stack_empty) 
		{
			if (libesedb_stack_pop(
				table->record_definitions_stack,
				record_data_definition,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to pop a record definition from the search stack.",
					function);

				exit_status = -1;
				goto on_resource_deallocation;
			}

			// A record definition is found - break the search loop.
			break;
		}

		/* When the records_data_definitions stack is EMPTY => find new record definitions.
		 * Objects of the structure record_data_definition are attached to the table's B-treee leaf nodes.
		 */
		if (libesedb_stack_is_empty(
			table->nodes_stack,
			&is_stack_empty,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to access the record definitions search stack.",
				function);

			exit_status = -1;
			goto on_resource_deallocation;
		}

		if (is_stack_empty == 1) 
		{
			break; // END OF SEARCH
		}

		if (libesedb_stack_pop(
			table->nodes_stack,
			&current_btree_range,
			error) != 1 )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to pop a B-tree node from the search stack.",
				function );
			
			exit_status = -1;
			goto on_resource_deallocation;
		}

		if (load_btree_tree_node(
			table,
			current_btree_range,
			&current_btree_node,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to load B-tree node.",
				function);

			exit_status = -1;
			goto on_resource_deallocation;
		}

		is_leaf_node = libfdata_btree_node_is_leaf(
						   current_btree_node,
						   error);

		if (is_leaf_node == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to determine whether the node is leaf.",
				function);

			exit_status = -1;
			goto on_resource_deallocation;
		}
		else if (is_leaf_node == 1)
		{
			if (traverse_table_records_btree_leaf_node(
				table,
				current_btree_node,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to process btree leaf nodes.",
					function);

				exit_status = -1;
				goto on_resource_deallocation;
			}
		}
		else
		{
			if (traverse_table_records_btree_branch_node(
				table,
				current_btree_node,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to process a branching btree node.",
					function);
				
				exit_status = -1;
				goto on_resource_deallocation;
			}
		} // else
	
	on_resource_deallocation:
		if (libfdata_btree_range_free(
			&current_btree_range,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to deallocate B-tree value range object.",
				function);

			exit_status = -1;
		}

		if (libfdata_btree_node_free(
			&current_btree_node,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to deallocate a btree node.",
				function);

			exit_status = -1;
		}

		if (exit_status == -1)
		{
			return exit_status;
		}
	}
	return 1;
}


int libfdata_blob_btree_traverse_node(
	libesedb_table_t *table,
	libfdata_btree_range_t *node_data_range,
	int level,
	uint8_t read_flags,
	libcerror_error_t **error);

/*	Performs depth-first B-tree search algorithm to find and retrieve next record data definition object.
*  Record definition nodes are leaf nodes in the B-Tree.
*	Returns 1 on success, otherwise -1.
*/
int libesedb_search_next_leaf_data_range(
	libesedb_internal_table_t *table,
	libfdata_range_t **node_data_range,
	libcerror_error_t **error)
{
	static char *function = "libesedb_search_next_leaf_node";
	int result = 1;

	if (table == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: table is NULL!",
			function);
		return -1;
	}

	if (table->long_values_tree == NULL) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid tree - missing root node data range.",
			function);

		return(-1);
	}

	if (node_data_range == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: record pointer address is null!",
			function);

		return -1;
	}

	if (*node_data_range != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: the pointer to the record is already set.",
			function);

		return -1;
	}

	while (1)
	{
		btree_node_info_t *current_btree_node_info = NULL;
		int is_stack_empty;

		// Check if any record definition is already available.
		if (libesedb_stack_is_empty(
			table->blob_btree_nodes_stack,
			&is_stack_empty,
			error) == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to access the record definitions search stack.",
				function);

			goto on_error;
		}

		// If there are some record definitions on the search stack (stack is not empty),
		// pop and retrieve the top one.
		if (is_stack_empty)
		{
			*node_data_range = NULL;
			break;
		}

		// When the records data definitions stack is EMPTY => find new record definitions.
		// The record definitions are values attached to leaf nodes in the B-tree.
		// Pop next range from the search stack.
		if (libesedb_stack_pop(
			table->blob_btree_nodes_stack,
			&current_btree_node_info,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to pop a B-tree node from the search stack.",
				function);

			goto on_error;
		}

		// If current_btree_node is NULL, the search is completed.
		if (current_btree_node_info == NULL)
		{
			break;
		}

		if (current_btree_node_info->is_leaf_node) 
		{
			*node_data_range = current_btree_node_info->node_data;
			current_btree_node_info->node_data = NULL;
			memory_free(current_btree_node_info);
			return 1;
		}
		else 
		{
			libfdata_btree_range_t *branch_node_data_range = (libfdata_btree_range_t*)(current_btree_node_info->node_data);

			if (libfdata_blob_btree_traverse_node(
				(libesedb_table_t *)table,
				branch_node_data_range,
				0, // level
				0, // read_flags
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to pop a B-tree node from the search stack.",
					function);

				goto on_error;
			}
		}

		{ /* Resource Deallocation */
			goto on_end;
		on_error:
			result = -1;
		on_end:
			if (current_btree_node_info->node_data != NULL) 
			{
				if (libfdata_btree_range_free(
					(libfdata_btree_range_t**)(&(current_btree_node_info->node_data)),
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GENERIC,
						"%s: unable to deallocate a btree range node.",
						function);

					result = -1;
				}
			}

			memory_free(current_btree_node_info);
		}
	}
	return 0;
}

/* Retrieves next record from the given table.
 * Returns 1 on success, or -1 in case of an error.
 */
int libesedb_table_get_next_record(
     libesedb_table_t *table,
     libesedb_record_t **record,
     libcerror_error_t **error)
{
	libesedb_data_definition_t *record_data_definition = NULL;
	libesedb_internal_table_t *internal_table     = NULL;
	static char *function                         = "libesedb_table_get_next_record";

	if( table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table.",
		 function );

		return( -1 );
	}
	internal_table = (libesedb_internal_table_t *) table;

	if( record == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}

	if( *record != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid record value already set.",
		 function );

		return( -1 );
	}

	if ( libesedb_search_next_record_definition(
			internal_table, 
			&record_data_definition,
			error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: unable to find next leaf node.",
			function );
		
		goto on_error;
	}

	if (record_data_definition == NULL)
	{
		// Reached the end of the b-tree search - return NULL
		*record = NULL;
		goto on_exit;
	}
	
	if (libesedb_record_initialize(
		record,
		internal_table->file_io_handle,
		internal_table->io_handle,
		internal_table->table_definition,
		internal_table->template_table_definition,
		internal_table->pages_vector,
		internal_table->pages_cache,
		internal_table->long_values_pages_vector,
		internal_table->long_values_pages_cache,
		record_data_definition,
		internal_table->long_values_tree,
		internal_table->long_values_cache,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create record.",
			function);

		goto on_error;
	}

	{
		int exit_status;
		goto on_exit;

	on_error: 
		exit_status = -1;

		if (libesedb_record_free(
			record,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				"%s: unable to deallocate a record",
				function);
		}
		goto on_resource_deallocation;

	on_exit:
		exit_status = 1;

	on_resource_deallocation: 
		if (libesedb_data_definition_free(
			&record_data_definition,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to deallocate a record data definition",
				function);
			exit_status = -1;
		}

		return exit_status;
	}
}

#if defined(DEBUG_LIBESEDB_PERFRORMANCE_PATCH)
static void print_long_value_key(libesedb_key_btree_t* key, long *count, const char* suffix)
{
	printf("%d) key (type: %c, size: %dB, value: ", ++(*count), key->type, key->data_size);
	for (int i = 0; i < key->data_size; i++) {
		printf("%02X", key->data[i]);
	}
	printf(") - %s\n", suffix);
}
#endif

#if defined(DEBUG_LIBESEDB_PERFRORMANCE_PATCH)
int libesedb_traverse_blob_tree(
	libesedb_table_t *table,
	uint8_t read_flags,
	libcerror_error_t **error)
{
	static char *function = "libfdata_btree_read_sub_tree";
	
	libesedb_internal_table_t *internal_table = NULL;
	libfdata_internal_btree_t *internal_btree = NULL;
	btree_node_info_t *btree_node_info = NULL;
	libfdata_btree_node_t *sub_node = NULL;
	long counter = 0;

	if (table == NULL) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid argument, table is null",
			function);
	}

	internal_table = (libesedb_internal_table_t*)table;

	if (internal_table->long_values_tree == NULL) 
	{
		return 0;
	}

	internal_btree = (libfdata_internal_btree_t*)internal_table->long_values_tree;

	if (internal_btree->root_node_data_range == NULL) 
	{
		return 0;
	}

#if defined(DEBUG_LIBESEDB_PERFRORMANCE_PATCH)

	btree_node_info = memory_allocate_structure(btree_node_info_t);
	btree_node_info->node_data = NULL;
	btree_node_info->is_leaf_node = 0;

	libesedb_libfdata_btree_range_clone(
		internal_btree->root_node_data_range,
		(libfdata_btree_range_t**) &(btree_node_info->node_data),
		error);

	// Push value tree node. 
	if (libesedb_stack_push(
		internal_table->blob_btree_nodes_stack,
		btree_node_info,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GENERIC,
			"%s: unable to push ROOT NODE onto the search stack.",
			function);

		return(-1);
	}

	{
		while (1)
		{
			size_t blob_key_size = 0, blob_data_size = 0;
			uint8_t *blob_key_bytes = NULL, *blob_data_bytes = NULL;
			int result = libesedb_table_get_next_blob_segment(
							table,
							&blob_key_size,
							&blob_key_bytes,
							&blob_data_size,
							&blob_data_bytes,
							error);

			if (result == 0) 
			{
				break;
			}

			if (result != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
					"%s: unable to fetch next blob segment",
					function);
				return(-1);
			}

			// Print extracted bytes
			int i = 0;
			printf("key bytes [%zd] = ", blob_key_size);
			for (i = 0; i < blob_key_size; i++) {
				printf("%02x", blob_key_bytes[i]);
			}
			printf("\n");

			printf("data bytes [%zd] = ", blob_data_size);
			for (i = 0; i < blob_data_size; i++) {
				printf("%02x", blob_data_bytes[i]);
			}
			printf("\n");

			// deallocate extracted blob data
			memory_free(blob_key_bytes);
			memory_free(blob_data_bytes);
		}
	}
#else

	return libfdata_btree_traverse_sub_tree(
		table,
		// internal_table->long_values_tree,
		// internal_table->file_io_handle,
		// internal_table->io_handle,
		// internal_table->long_values_cache,
		// internal_table->long_values_pages_cache,
		// internal_table->long_values_pages_vector,
		internal_btree->root_node_data_range,
		0,
		internal_btree->root_node_data_range->mapped_first_leaf_value_index,
		&sub_node,
		LIBFDATA_EXPORT_LEAF_NODES,
		&counter,
		error);	

#endif
}
#endif

#if defined (DEBUG_LIBESEDB_PERFRORMANCE_PATCH)
int extract_long_value_segment_bytes(
	libesedb_internal_table_t *table,
	libfdata_internal_btree_t *internal_tree,
	libfdata_btree_range_t *leaf_value_data_range,
	long* key_counter,
	libcerror_error_t **error) 
{
	static char *function = "extract_long_value_segment_bytes";

	uint32_t long_value_segment_offset = 0;
	libfdata_list_t *data_segments_list = NULL;
	libesedb_data_definition_t *data_definition = NULL;
	off64_t leaf_value_data_offset = 0;
	size64_t leaf_value_data_size = 0;
	uint32_t leaf_value_data_flags = 0;
	int leaf_value_data_file_index = -1;
	intptr_t *leaf_value_key_value;
	libesedb_long_value_t *long_value = NULL;
	int result;

	if (libfdata_btree_range_get(
		leaf_value_data_range,
		&leaf_value_data_file_index,
		&leaf_value_data_offset,
		&leaf_value_data_size,
		&leaf_value_data_flags,
		&leaf_value_key_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve leaf value data range values.",
			function);

		goto on_error;
	}

#ifdef HAVE_DEBUG_OUTPUT
	libesedb_key_btree_t* key = (libesedb_key_btree_t*)leaf_value_key_value;
	print_long_value_key(key, key_counter, "leaf value");
#endif

	if (libesedb_page_tree_read_data_definition(
		(libesedb_page_tree_t *)internal_tree->data_handle,
		leaf_value_data_range,
		&data_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to read data_definition from a leaf node.",
			function);
		
		goto on_error;
	}

#ifdef HAVE_DEBUG_OUTPUT
	printf("data definition: offset=%d, size=%d, pgNo=%d, pgOff=%d, pgValueIdx=%d\n",
		data_definition->data_offset,
		data_definition->data_size,
		data_definition->page_number,
		data_definition->page_offset,
		data_definition->page_value_index
	);
#endif

	if (libfdata_list_initialize(
		&data_segments_list,
		NULL,
		NULL,
		NULL,
		(int(*)(intptr_t *, intptr_t *, libfdata_list_element_t *, libfcache_cache_t *, int, off64_t, size64_t, uint32_t, uint8_t, libcerror_error_t **)) &libesedb_data_segment_read_element_data,
		NULL,
		0,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create data segments list.",
			function);

		goto on_error;
	}

	if (libesedb_data_definition_read_long_value_segment(
		data_definition,
		table->file_io_handle,
		table->io_handle,
		table->long_values_pages_vector,
		table->long_values_pages_cache,
		long_value_segment_offset,
		data_segments_list,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			LIBCERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read data definition long value segment.",
			function);

		goto on_error;
	}

	if (libesedb_create_long_value_from_data_segments(
		&long_value,
		table->file_io_handle,
		table->io_handle,
		data_segments_list,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create long value.",
			function);

		goto on_error;
	}

	if (long_value == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to read long value.",
			function);
		
		goto on_error;
	}

	{
		uint8_t *value_data = NULL;
		size64_t value_data_size = 0;
		
		if (libesedb_read_long_value_bytes(
			long_value,
			&value_data,
			&value_data_size,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve long value data of record entry: %d.",
				function,
				0);

			goto on_error;
		}
#ifdef HAVE_DEBUG_OUTPUT
		{
			// Print extracted bytes
			int i = 0;
			printf("bytes [%d] = ", value_data_size);
			for (i = 0; i < value_data_size; i++) {
				printf("%02x", value_data[i]);
			}
			printf("\n");
		}
#endif
	}

	result = 1;

on_end:
	if (data_segments_list != NULL)
	{
		if (libfdata_list_free(&data_segments_list, NULL) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to deallocate data segments list.",
				function);
		}
	}

	if (libesedb_data_definition_free(
		&data_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to deallocate data definition.",
			function);		
	}

	return result;

on_error:
	result = -1;
	goto on_end;
}
#endif


#if defined(DEBUG_LIBESEDB_PERFRORMANCE_PATCH)
/* Reads the sub tree
 * Returns 1 if successful or -1 on error
 * TODO: This method is NOT used - DELETE IT.
 */
int libfdata_btree_traverse_sub_tree(
	libesedb_table_t *table,
	libfdata_btree_range_t *node_data_range,
	int level,
	int mapped_first_leaf_value_index,
	libfdata_btree_node_t **node,
	uint8_t read_flags,
	long* key_counter,
	libcerror_error_t **error)
{
	libfdata_btree_range_t *leaf_value_data_range = NULL;
	libfdata_btree_range_t *sub_node_data_range = NULL;
	libfdata_btree_node_t *sub_node = NULL;
	intptr_t *key_value = NULL;
	static char *function = "libfdata_btree_read_sub_tree";
	off64_t node_data_offset = 0;
	size64_t node_data_size = 0;
	uint32_t node_data_flags = 0;
	int branch_number_of_leaf_values = 0;
	int leaf_value_index = 0;
	int node_data_file_index = -1;
	int number_of_leaf_values = 0;
	int number_of_sub_nodes = 0;
	int result = 0;
	int sub_node_index = 0;
	libfdata_internal_btree_t *internal_tree = NULL;
	libesedb_internal_table_t *internal_table = NULL;

	if (table == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid table (NULL).",
			function);
	}

	internal_table = (libesedb_internal_table_t*)table;

	if (internal_table->long_values_tree == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid tree.",
			function);

		return(-1);
	}

	internal_tree = (libfdata_internal_btree_t*)internal_table->long_values_tree;

	if (level < 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
			"%s: invalid level value out of bounds.",
			function);

		return(-1);
	}
	if (node == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: missing node.",
			function);

		return(-1);
	}

	/* Note that the node data range can be cached out later in the function.
	*/
	if (libfdata_btree_range_get(
		node_data_range,
		&node_data_file_index,
		&node_data_offset,
		&node_data_size,
		&node_data_flags,
		&key_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve data range from tree node.",
			function);

		goto on_error;
	}
	/* Make sure to not have the node cached here to prevent it from
	* being freed during recursing the sub tree.
	*/
	if (libfdata_btree_read_node(
		internal_tree,
		internal_table->file_io_handle,
		internal_table->long_values_cache,
		node_data_range,
		level,
		node,
		read_flags | LIBFDATA_READ_FLAG_NO_CACHE,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			LIBCERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read node.",
			function);

		goto on_error;
	}
	if (*node == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: missing node.",
			function);

		goto on_error;
	}
	if ((((libfdata_internal_btree_node_t *)*node)->flags & LIBFDATA_FLAG_CALCULATE_MAPPED_RANGES) != 0)
	{
		result = libfdata_btree_node_is_leaf(
			*node,
			error);

		if (result == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to determine if node is a leaf node.",
				function,
				sub_node_index);

			goto on_error;
		}
		else if (result != 0)
		{
			if (libfdata_btree_node_get_number_of_leaf_values(
				*node,
				&number_of_leaf_values,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve number of leaf values.",
					function);

				goto on_error;
			}
			for (leaf_value_index = 0;
				leaf_value_index < number_of_leaf_values;
				leaf_value_index++)
			{
				if (libfdata_btree_node_get_leaf_value_data_range_by_index(
					*node,
					leaf_value_index,
					&leaf_value_data_range,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve leaf value: %d data range.",
						function,
						leaf_value_index);

					goto on_error;
				}
				if (leaf_value_data_range == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
						"%s: missing leaf value: %d data range.",
						function,
						leaf_value_index);

					goto on_error;
				}
				leaf_value_data_range->mapped_first_leaf_value_index = mapped_first_leaf_value_index;
				leaf_value_data_range->mapped_last_leaf_value_index = -1;
				leaf_value_data_range->mapped_number_of_leaf_values = 1;

				mapped_first_leaf_value_index++;

				if ((read_flags & LIBFDATA_EXPORT_LEAF_NODES) != 0)
				{
					extract_long_value_segment_bytes(
						internal_table, 
						internal_tree, 
						leaf_value_data_range, 
						key_counter, 
						error);
				}
			}
			branch_number_of_leaf_values = number_of_leaf_values;

			/* TODO*/
			level++;
		}
		else
		{
			if (libfdata_btree_node_get_number_of_sub_nodes(
				*node,
				&number_of_sub_nodes,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve number of sub nodes.",
					function);

				goto on_error;
			}
			level++;

			for (sub_node_index = 0;
				sub_node_index < number_of_sub_nodes;
				sub_node_index++)
			{
				if (libfdata_btree_node_get_sub_node_data_range_by_index(
					*node,
					sub_node_index,
					&sub_node_data_range,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve sub node: %d data range.",
						function,
						sub_node_index);

					goto on_error;
				}
				if (sub_node_data_range == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
						"%s: missing sub node: %d data range.",
						function,
						sub_node_index);

					goto on_error;
				}

				if ((read_flags & LIBFDATA_EXPORT_LEAF_NODES) != 0)
				{
					off64_t leaf_value_data_offset = 0;
					size64_t leaf_value_data_size = 0;
					uint32_t leaf_value_data_flags = 0;
					int leaf_value_data_file_index = -1;
					intptr_t *leaf_value_key_value;

					if (libfdata_btree_range_get(
						sub_node_data_range,
						&leaf_value_data_file_index,
						&leaf_value_data_offset,
						&leaf_value_data_size,
						&leaf_value_data_flags,
						&leaf_value_key_value,
						error) != 1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GET_FAILED,
							"%s: unable to retrieve leaf value data range values.",
							function);

						return(-1);
					}
					libesedb_key_btree_t* key = (libesedb_key_btree_t*)leaf_value_key_value;
#if defined(HAVE_DEBUG_OUTPUT)
					print_long_value_key(key, key_counter, "branch node ============================== ");
#endif
				}

				// Recursive call.
				if (libfdata_btree_traverse_sub_tree(
					table,
					// internal_tree,
					// file_io_handle,
					// io_handle,
					// long_values_cache,
					// long_values_pages_cache,
					// long_values_pages_vector,
					sub_node_data_range,
					level,
					mapped_first_leaf_value_index,
					&sub_node,
					read_flags,
					key_counter,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_READ_FAILED,
						"%s: unable to read sub node: %d sub tree.",
						function,
						sub_node_index);

					goto on_error;
				}
				if (libfdata_btree_node_get_number_of_leaf_values_in_branch(
					sub_node,
					&number_of_leaf_values,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve number of leaf values in branch.",
						function);

					goto on_error;
				}
				sub_node_data_range->mapped_first_leaf_value_index = mapped_first_leaf_value_index;
				mapped_first_leaf_value_index += number_of_leaf_values;
				sub_node_data_range->mapped_last_leaf_value_index = mapped_first_leaf_value_index - 1;
				sub_node_data_range->mapped_number_of_leaf_values = number_of_leaf_values;
				branch_number_of_leaf_values += number_of_leaf_values;
			}
		}
		((libfdata_internal_btree_node_t *)*node)->branch_number_of_leaf_values = branch_number_of_leaf_values;
		((libfdata_internal_btree_node_t *)*node)->flags &= ~(LIBFDATA_FLAG_CALCULATE_MAPPED_RANGES);
	}
	
	return(1);

on_error:
	if (*node != NULL)
	{
		libfdata_btree_node_free(
			node,
			NULL);
	}
	return(-1);
}
#endif


/* Reads the sub tree
 * Returns 1 if successful or -1 on error
 */
int libfdata_blob_btree_traverse_node(
	libesedb_table_t *table,
	libfdata_btree_range_t *node_data_range,
	int level,
	uint8_t read_flags,
	libcerror_error_t **error)
{
	libfdata_btree_range_t *leaf_value_data_range = NULL;
	libfdata_btree_range_t *sub_node_data_range = NULL;
	libfdata_btree_node_t *node = NULL;

	intptr_t *key_value = NULL;
	static char *function = "libfdata_btree_read_sub_tree";
	off64_t node_data_offset = 0;
	size64_t node_data_size = 0;
	uint32_t node_data_flags = 0;
	int branch_number_of_leaf_values = 0;
	int node_data_file_index = -1;
	int number_of_leaf_values = 0;
	int number_of_sub_nodes = 0;
	int leaf_value_index;
	int result = 0;
	int sub_node_index = 0;
	libfdata_internal_btree_t *internal_tree = NULL;
	libesedb_internal_table_t *internal_table = NULL;

	if (table == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid table (NULL).",
			function);
	}

	internal_table = (libesedb_internal_table_t*)table;

	if (internal_table->long_values_tree == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid tree.",
			function);

		return(-1);
	}

	internal_tree = (libfdata_internal_btree_t*)internal_table->long_values_tree;

	if (level < 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
			"%s: invalid level value out of bounds.",
			function);

		return(-1);
	}
	
	/* Note that the node data range can be cached out later in the function.
	*/
	if (libfdata_btree_range_get(
		node_data_range,
		&node_data_file_index,
		&node_data_offset,
		&node_data_size,
		&node_data_flags,
		&key_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve data range from tree node.",
			function);

		goto on_error;
	}
	/* Make sure to not have the node cached here to prevent it from
	 * being freed during recursing the sub tree.
	 */
	if (libfdata_btree_read_node(
		internal_tree,
		internal_table->file_io_handle,
		internal_table->long_values_cache,
		node_data_range,
		level,
		&node,
		read_flags | LIBFDATA_READ_FLAG_NO_CACHE,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			LIBCERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read node.",
			function);

		goto on_error;
	}
	if (node == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: missing node.",
			function);

		goto on_error;
	}
	if ((((libfdata_internal_btree_node_t *)node)->flags & LIBFDATA_FLAG_CALCULATE_MAPPED_RANGES) != 0)
	{
		result = libfdata_btree_node_is_leaf(node, error);

		if (result == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to determine if node is a leaf node.",
				function,
				sub_node_index);

			goto on_error;
		}
		else if (result != 0)
		{
			if (libfdata_btree_node_get_number_of_leaf_values(
				node,
				&number_of_leaf_values,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve number of leaf values.",
					function);

				goto on_error;
			}

			for (leaf_value_index = number_of_leaf_values - 1;
				leaf_value_index >= 0;
				leaf_value_index--)
			{
				if (libfdata_btree_node_get_leaf_value_data_range_by_index(
					node,
					leaf_value_index,
					&leaf_value_data_range,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve leaf value: %d data range.",
						function,
						leaf_value_index);

					goto on_error;
				}
				
				if (leaf_value_data_range == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
						"%s: missing leaf value: %d data range.",
						function,
						leaf_value_index);

					goto on_error;
				}
				
				leaf_value_data_range->mapped_first_leaf_value_index = internal_table->mapped_leaf_value_first_index + leaf_value_index;
				leaf_value_data_range->mapped_last_leaf_value_index = -1;
				leaf_value_data_range->mapped_number_of_leaf_values = 1;

				{
					btree_node_info_t *btree_node_info = memory_allocate_structure(btree_node_info_t);
					if (btree_node_info == NULL)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GENERIC,
							"%s: unable to allocate a btree_node_info.",
							function);

						return -1;
					}

					btree_node_info->is_leaf_node = 1;
					btree_node_info->node_data = NULL; // leaf_value_data_range;

					// clone current leaf_value_data_range and put it onto the temp_stack
					if (libesedb_libfdata_btree_range_clone(
						leaf_value_data_range,
						(libfdata_btree_range_t **) &(btree_node_info->node_data),
						error) == -1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GENERIC,
							"%s: unable to clone a btree node data range.",
							function);

						memory_free(btree_node_info);
						return -1;
					}

					// Put the new leaf value onto the search stack. 
					if (libesedb_stack_push(
						internal_table->blob_btree_nodes_stack,
						btree_node_info,
						error) == -1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GENERIC,
							"%s: unable to a record definitions onto the stack.",
							function);

						memory_free(btree_node_info->node_data);
						memory_free(btree_node_info);
						return -1;
					}
				}
			}

			internal_table->mapped_leaf_value_first_index += number_of_leaf_values;
			branch_number_of_leaf_values = number_of_leaf_values;

			/* TODO*/
			level++;
		}
		else
		{
			if (libfdata_btree_node_get_number_of_sub_nodes(
				node,
				&number_of_sub_nodes,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve number of sub nodes.",
					function);

				goto on_error;
			}
			level++;

			for (sub_node_index = number_of_sub_nodes - 1;
				sub_node_index >= 0;
				sub_node_index--)
			{
				btree_node_info_t *btree_node_info = NULL;

				if (libfdata_btree_node_get_sub_node_data_range_by_index(
					node,
					sub_node_index,
					&sub_node_data_range,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve sub node: %d data range.",
						function,
						sub_node_index);

					goto on_error;
				}
				
				if (sub_node_data_range == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
						"%s: missing sub node: %d data range.",
						function,
						sub_node_index);

					goto on_error;
				}
#ifdef DEBUG_LIBESEDB_PERFRORMANCE_PATCH
				if ((read_flags & LIBFDATA_EXPORT_LEAF_NODES) != 0)
				{
					off64_t leaf_value_data_offset = 0;
					size64_t leaf_value_data_size = 0;
					uint32_t leaf_value_data_flags = 0;
					int leaf_value_data_file_index = -1;
					intptr_t *leaf_value_key_value;

					if (libfdata_btree_range_get(
						sub_node_data_range,
						&leaf_value_data_file_index,
						&leaf_value_data_offset,
						&leaf_value_data_size,
						&leaf_value_data_flags,
						&leaf_value_key_value,
						error) != 1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GET_FAILED,
							"%s: unable to retrieve leaf value data range values.",
							function);

						return(-1);
					}
					libesedb_key_btree_t* key = (libesedb_key_btree_t*)leaf_value_key_value;
					// print_long_value_key(key, key_counter, "branch node ============================== ");
				}
#endif // DEBUG_LIBESEDB_PERFRORMANCE_PATCH
				
				btree_node_info = memory_allocate_structure(btree_node_info_t);

				if (btree_node_info == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GENERIC,
						"%s: unable to allocate a btree_node_info.",
						function);

					return -1;
				}

				btree_node_info->is_leaf_node = 0;
				btree_node_info->node_data = NULL; // leaf_value_data_range;

				// clone current leaf_value_data_range and put it onto the temp_stack
				if (libesedb_libfdata_btree_range_clone(
					sub_node_data_range,
					(libfdata_btree_range_t**) &(btree_node_info->node_data),
					error) == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GENERIC,
						"%s: unable to clone a btree node data range.",
						function);

					memory_free(btree_node_info);
					return -1;
				}

				if (libesedb_stack_push(
					internal_table->blob_btree_nodes_stack,
					btree_node_info,
					error) != 1) 
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to push a branch node to temp stack.",
						function);

					memory_free(btree_node_info->node_data);
					memory_free(btree_node_info);
					return -1;
				}
			}
		}
		((libfdata_internal_btree_node_t *)node)->branch_number_of_leaf_values = branch_number_of_leaf_values;
		((libfdata_internal_btree_node_t *)node)->flags &= ~(LIBFDATA_FLAG_CALCULATE_MAPPED_RANGES);
	}

	return(1);

on_error:
	if (node != NULL)
	{
		if (libfdata_btree_node_free(
			&node, 
			error) != 1) 
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to deallocate a btree node.",
				function);
		}
	}
	return(-1);
}

/* Reads a page tree leaf value
 * Callback function for the page tree
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_read_data_definition(
	libesedb_page_tree_t *page_tree,
	libfdata_btree_range_t *leaf_value_data_range,
	libesedb_data_definition_t **data_definition,
	libcerror_error_t **error)
{
	static char *function = "libesedb_page_tree_read_leaf_value";
	off64_t page_offset = 0;
	uint64_t page_number = 0;

	int leaf_value_data_file_index = leaf_value_data_range->file_index;
	off64_t leaf_value_data_offset = leaf_value_data_range->offset;
	size64_t leaf_value_data_size = leaf_value_data_range->size;
	// uint32_t leaf_value_data_flags = leaf_value_data_range->flags;
	// intptr_t *key_value = leaf_value_data_range->key_value;

	if (page_tree == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid page tree.",
			function);

		return(-1);
	}
	if (data_definition == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid data definition pointer.",
			function);

		return(-1);
	}
	if (*data_definition != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: data definition must be NULL.",
			function);

		return(-1);
	}
	if (page_tree->io_handle == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid page tree - missing IO handle.",
			function);

		return(-1);
	}
	if (page_tree->io_handle->page_size == 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid page tree - invalid IO handle - missing page size.",
			function);

		return(-1);
	}
	if (leaf_value_data_file_index < 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid leaf value data file index value less than zero.",
			function);

		return(-1);
	}
	if (leaf_value_data_offset < 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid leaf value data offset value less than zero.",
			function);

		return(-1);
	}
	if (leaf_value_data_size > (off64_t)UINT16_MAX)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid leaf value data size value exceeds maximum.",
			function);

		return(-1);
	}
	page_offset = leaf_value_data_offset / page_tree->io_handle->page_size;
	page_number = (uint64_t)(page_offset + 1);
	page_offset *= page_tree->io_handle->page_size;

	if (page_number > (uint64_t)UINT32_MAX)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
			"%s: invalid page number value exceeds maximum.",
			function);

		return(-1);
	}
	if (libesedb_data_definition_initialize(
		data_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create data definition.",
			function);

		goto on_error;
	}
	if (*data_definition == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: missing data definition.",
			function);

		return(-1);
	}

	/* leaf_value_data_file_index contains the page value index
	 */
	(*data_definition)->page_value_index = (uint16_t)leaf_value_data_file_index;
	(*data_definition)->page_offset = page_offset;
	(*data_definition)->page_number = (uint32_t)page_number;
	(*data_definition)->data_offset = (uint16_t)(leaf_value_data_offset - page_offset);
	(*data_definition)->data_size = (uint16_t)leaf_value_data_size;

	return(1);

on_error:
	if (*data_definition != NULL)
	{
		libesedb_data_definition_free(
			data_definition,
			NULL);
	}
	return(-1);
}

int libfdata_data_definition_read_blob_segment(
	libfdata_list_t **data_segments_list,
	libesedb_internal_record_t *internal_record,
	libesedb_data_definition_t *data_definition,
	libcerror_error_t **error)
{
	static char *function = "libesedb_record_get_long_value_data_segments_list";
	uint32_t long_value_segment_offset = 0;

	if (data_segments_list == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid data segments list.",
			function);

		return(-1);
	}

	if (libfdata_list_initialize(
		data_segments_list,
		NULL,
		NULL,
		NULL,
		(int(*)(intptr_t *, intptr_t *, libfdata_list_element_t *, libfcache_cache_t *, int, off64_t, size64_t, uint32_t, uint8_t, libcerror_error_t **)) &libesedb_data_segment_read_element_data,
		NULL,
		0,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create data segments list.",
			function);

		goto on_error;
	}

	if (libesedb_data_definition_read_long_value_segment(
		data_definition,
		internal_record->file_io_handle,
		internal_record->io_handle,
		internal_record->long_values_pages_vector,
		internal_record->long_values_pages_cache,
		long_value_segment_offset,
		*data_segments_list,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			LIBCERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read data definition long value segment.",
			function);

		goto on_error;
	}

on_error:
	if (*data_segments_list != NULL)
	{
		libfdata_list_free(
			data_segments_list,
			NULL);
	}
	return(-1);
}


/* Creates a long value
 * Make sure the value long_value is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libesedb_create_long_value_from_data_segments(
	libesedb_long_value_t **long_value,
	libbfio_handle_t *file_io_handle,
	libesedb_io_handle_t *io_handle,
	libfdata_list_t *data_segments_list,
	libcerror_error_t **error)
{
	libesedb_internal_long_value_t *internal_long_value = NULL;
	static char *function = "libesedb_long_value_initialize";

	if (long_value == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid long value.",
			function);

		return(-1);
	}
	if (*long_value != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: invalid long value value already set.",
			function);

		return(-1);
	}
	if (data_segments_list == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid data segments list.",
			function);

		return(-1);
	}
	internal_long_value = memory_allocate_structure(libesedb_internal_long_value_t);

	if (internal_long_value == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_MEMORY,
			LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			"%s: unable to create long value.",
			function);

		goto on_error;
	}

	if (memory_set(
		internal_long_value,
		0,
		sizeof(libesedb_internal_long_value_t)) == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_MEMORY,
			LIBCERROR_MEMORY_ERROR_SET_FAILED,
			"%s: unable to clear long value.",
			function);

		goto on_error;
	}

	if (libfcache_cache_initialize(
		&(internal_long_value->data_segments_cache),
		LIBESEDB_MAXIMUM_CACHE_ENTRIES_LONG_VALUES_DATA,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create data segments cache.",
			function);

		goto on_error;
	}

	internal_long_value->file_io_handle = file_io_handle;
	internal_long_value->io_handle = io_handle;
	internal_long_value->data_segments_list = data_segments_list;

	// return the intialized long value object through the out ref argument long_value
	*long_value = (libesedb_long_value_t *)internal_long_value;

	internal_long_value = NULL;
	return(1);

on_error:
	if (libesedb_long_value_free(
		(libesedb_long_value_t **) &internal_long_value,
		NULL) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to deallocate a long_value object.",
			function);
	}

	return(-1);
}

/* Retrieves the long value data from a long value
 * Returns 1 if successful or -1 on error
 */
int libesedb_read_long_value_bytes(
	libesedb_long_value_t *long_value,
	uint8_t **long_value_data,
	size_t *long_value_data_size,
	libcerror_error_t **error)
{
	static char *function = "export_get_long_value_data";
	size64_t safe_long_value_data_size = 0;
	uint8_t *long_value_bytes = NULL;

	if (long_value == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid long value.",
			function);

		return(-1);
	}
	if (long_value_data == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid long value data.",
			function);

		return(-1);
	}
	if (*long_value_data != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: invalid long value data value already set.",
			function);

		return(-1);
	}
	if (long_value_data_size == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid long value data size.",
			function);

		return(-1);
	}
	if (libesedb_long_value_get_data_size(
		long_value,
		&safe_long_value_data_size,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve long value data size.",
			function);

		goto on_error;
	}
	if (safe_long_value_data_size > (size64_t)SSIZE_MAX)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
			"%s: invalid long value data size value exceeds maximum.",
			function);

		goto on_error;
	}
	if (safe_long_value_data_size == 0)
	{
		*long_value_data = NULL;
		*long_value_data_size = 0;
		return 1;
	}

	long_value_bytes = (uint8_t *)memory_allocate(sizeof(uint8_t) * (size_t)safe_long_value_data_size);

	if (long_value_bytes == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_MEMORY,
			LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			"%s: unable to create a long value data array.",
			function);

		goto on_error;
	}
	if (libesedb_long_value_get_data(
		long_value,
		long_value_bytes,
		(size_t)safe_long_value_data_size,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve long value data.",
			function);

		goto on_error;
	}

	// return long value bytes & size 
	*long_value_data_size = (size_t)safe_long_value_data_size;
	*long_value_data = long_value_bytes;

	long_value_bytes = NULL;
	return(1);

on_error:
	if (long_value_bytes != NULL)
	{
		memory_free(long_value_bytes);
	}
	return(-1);
}

/* Retrieves next blob segment from the table blob btree.
 * Returns: 
 *    1  - if next segment is retrieved successfully, 
 *    0  - if no more blob segments is available in the blob tree,
 *   -1  - if an error occurred during reading next blob segment.
 */
int libesedb_table_get_next_blob_segment(
	libesedb_table_t *table,
	size_t *blobSegmentKeySize,
	uint8_t **blobSegmentKeyBytes,
	size_t *blobSegmentDataSize,
	uint8_t **blobSegmentDataBytes,
	libcerror_error_t **error)
{
	static char *function = "libesedb_table_get_next_blob_segment";

	libfdata_list_t *data_segments_list = NULL;
	libesedb_data_definition_t *data_definition = NULL;
	off64_t leaf_value_data_offset = 0;
	size64_t leaf_value_data_size = 0;
	uint32_t leaf_value_data_flags = 0;
	int leaf_value_data_file_index = -1;
	intptr_t *leaf_value_key_value;
	libesedb_long_value_t *long_value = NULL;
	libfdata_range_t *leaf_node_data_range = NULL;
	libesedb_internal_table_t *internal_table = NULL;
	libfdata_internal_btree_t *internal_tree = NULL;
	uint8_t *blob_data_bytes = NULL;
	uint8_t *blob_key_bytes = NULL;
	size_t blob_key_size, blob_data_size;
	libesedb_key_btree_t *long_value_key = NULL;
	int result;

	if (table == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: table is null",
			function);
		
		goto on_error;
	}

	internal_table = (libesedb_internal_table_t*)table;

	if (internal_table->long_values_tree == NULL) 
	{
		// No long values tree
		return 0;
	}

	result = libesedb_search_next_leaf_data_range(
				internal_table,
				&leaf_node_data_range,
				error);

	if (result == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: unable to get long value data range.",
			function);

		goto on_error;
	}

	if (result == 0) 
	{
		return 0;
	}

	if (leaf_node_data_range == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: missing leaf value: %d data range.",
			function,
			0);

		goto on_error;
	}

	if (libfdata_btree_range_get(
		(libfdata_btree_range_t *)leaf_node_data_range,
		&leaf_value_data_file_index,
		&leaf_value_data_offset,
		&leaf_value_data_size,
		&leaf_value_data_flags,
		&leaf_value_key_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve leaf value data range values.",
			function);

		goto on_error;
	}

	if (internal_table->long_values_tree == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid tree.",
			function);

		goto on_error;
	}

	long_value_key = (libesedb_key_btree_t *)leaf_value_key_value;
	blob_key_size = long_value_key->data_size;
	blob_key_bytes = (uint8_t *)memory_allocate(sizeof(uint8_t) * blob_key_size);

	if (blob_key_bytes == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: unable to allocate blob key bytes: size = %d",
			function,
			blob_key_size);

		goto on_error;
	}

	for (int i = 0; i < blob_key_size; i++)
	{
		blob_key_bytes[i] = long_value_key->data[i];
	}

	internal_tree = (libfdata_internal_btree_t*)internal_table->long_values_tree;

	if (libesedb_page_tree_read_data_definition(
		(libesedb_page_tree_t *)internal_tree->data_handle,
		(libfdata_btree_range_t *)leaf_node_data_range,
		&data_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to read data_definition from a leaf node.",
			function);

		goto on_error;
	}

	if (libfdata_list_initialize(
		&data_segments_list,
		NULL,
		NULL,
		NULL,
		(int(*)(intptr_t *, intptr_t *, libfdata_list_element_t *, libfcache_cache_t *, int, off64_t, size64_t, uint32_t, uint8_t, libcerror_error_t **)) &libesedb_data_segment_read_element_data,
		NULL,
		0,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create data segments list.",
			function);

		goto on_error;
	}

	if (libesedb_data_definition_read_long_value_segment(
		data_definition,
		internal_table->file_io_handle,
		internal_table->io_handle,
		internal_table->long_values_pages_vector,
		internal_table->long_values_pages_cache,
		0, // long_value_segment_offset,
		data_segments_list,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			LIBCERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read data definition long value segment.",
			function);

		goto on_error;
	}

	if (libesedb_create_long_value_from_data_segments(
		&long_value,
		internal_table->file_io_handle,
		internal_table->io_handle,
		data_segments_list,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create long value.",
			function);

		goto on_error;
	}

	if (long_value == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to read long value.",
			function);

		goto on_error;
	}

	// data_segments_list is linked to the long_value
	// set to null to prevent it from being deallocated twice
	data_segments_list = NULL;

	if (libesedb_read_long_value_bytes(
		long_value,
		&blob_data_bytes,
		&blob_data_size,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve long value data of record entry: %d.",
			function,
			0);

		goto on_error;
	}

	// Return extracted blob segment data
	*blobSegmentKeySize = blob_key_size;
	*blobSegmentKeyBytes = blob_key_bytes;
	*blobSegmentDataSize = blob_data_size;
	*blobSegmentDataBytes = blob_data_bytes;
	
	// set local pointers to NULL to prevent their values from being deallocated
	blob_data_bytes = NULL;
	blob_key_bytes = NULL;

	result = 1;

on_end:
	if (libesedb_long_value_free(
		&long_value, 
		error) != 1) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to deallocate data segments list.",
			function);

		result = -1;
	}
	if (libfdata_list_free(
		&data_segments_list,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to deallocate data segments list.",
			function);

		result = -1;
	}
	if (libesedb_data_definition_free(
		&data_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to deallocate data definition.",
			function);

		result = -1;
	}
	if (libfdata_btree_range_free(
		(libfdata_btree_range_t**) &leaf_node_data_range,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to deallocate data definition.",
			function);
		result = -1;
	}
	if (blob_key_bytes != NULL) 
	{
		memory_free(blob_key_bytes);
	}
	if (blob_data_bytes != NULL) 
	{
		memory_free(blob_data_bytes);
	}

	return result;

on_error:
	result = -1;
	goto on_end;
}


/* Retrieves the long value key of a specific entry
 * Creates a new long value
 * Returns 1 if successful, 0 if the item does not contain such value or -1 on error
 */
int libesedb_record_get_long_value_key(
	libesedb_record_t *record,
	int value_entry,
	uint8_t **long_value_key_bytes,
	size_t *long_value_key_size,
	libcerror_error_t **error)
{
	libesedb_catalog_definition_t *column_catalog_definition = NULL;
	libesedb_internal_record_t *internal_record = NULL;
	libfvalue_value_t *record_value = NULL;
	static char *function = "libesedb_record_get_long_value_key";
	uint32_t data_flags = 0;
	int encoding = 0;
	int result = 0;

	if (record == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid record.",
			function);

		return(-1);
	}
	internal_record = (libesedb_internal_record_t *)record;

	if (long_value_key_bytes == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid long value.",
			function);

		return(-1);
	}
	if (*long_value_key_bytes != NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: long value already set.",
			function);

		return(-1);
	}
	if (libesedb_record_get_column_catalog_definition(
		internal_record,
		value_entry,
		&column_catalog_definition,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve column catalog definition.",
			function);

		goto on_error;
	}
	if (libcdata_array_get_entry_by_index(
		internal_record->values_array,
		value_entry,
		(intptr_t **)&record_value,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve value: %d from values array.",
			function,
			value_entry);

		goto on_error;
	}
	if (libfvalue_value_get_data_flags(
		record_value,
		&data_flags,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve value: %d data flags.",
			function,
			value_entry);

		goto on_error;
	}
	if ((data_flags & LIBESEDB_VALUE_FLAG_LONG_VALUE) == 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			"%s: unsupported data flags: 0x%02" PRIx32 ".",
			function,
			data_flags);

		goto on_error;
	}
	if ((data_flags & LIBESEDB_VALUE_FLAG_MULTI_VALUE) != 0)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			"%s: unsupported data flags: 0x%02" PRIx32 ".",
			function,
			data_flags);

		goto on_error;
	}
	result = libfvalue_value_has_data(
				record_value,
				error);

	if (result == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to determine if value: %d has data.",
			function,
			value_entry);

		goto on_error;
	}
	else if (result == 0)
	{
		return(0);
	}

	if (libfvalue_value_get_data(
		record_value,
		long_value_key_bytes,
		long_value_key_size,
		&encoding,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable retrieve value data.",
			function);

		goto on_error;
	}

	return 1;

on_error:
	return -1;
}


int libesedb_memory_free_byte_array(
	uint8_t **byte_array,
	libcerror_error_t **error)
{
	if (*byte_array != NULL) 
	{
		memory_free(*byte_array);
	}

	*byte_array = NULL;
	return 1;
}

#endif