#include <stdlib.h>

#include "libesedb_libbfio.h"
#include "libesedb_page.h"
#include "libesedb_page_tree.h"
#include "libesedb_definitions.h"
#include "libesedb_record.h"
#include "libesedb_table.h"
#include "memory.h"


#ifdef LIBESEDB_PERFORMANCE_PATCH

/* Reads a page tree leaf value
 * Callback function for the page tree
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_load_data_definition(
     libesedb_page_tree_t *page_tree,
	 libesedb_data_definition_t **data_def_result,
	 libfdata_btree_range_t *leaf_value_data_range,
     libcerror_error_t **error )
{
	libesedb_data_definition_t *data_definition = NULL;
	static char *function                       = "libesedb_page_tree_read_leaf_value";
	off64_t page_offset                         = 0;
	uint64_t page_number                        = 0;
	int leaf_value_data_file_index;
    off64_t leaf_value_data_offset;
    size64_t leaf_value_data_size;
	intptr_t *key_value							= NULL;
	uint32_t leaf_value_data_flags				= 0;

	if( page_tree == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	if( data_def_result == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid data definition pointer: nul.",
		 function );

		return( -1 );
	}
	if( *data_def_result != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid data definition pointer: already set.",
		 function );

		return( -1 );
	}
	if( page_tree->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid page tree - missing IO handle.",
		 function );

		return( -1 );
	}
	if( libfdata_btree_range_get(
	     leaf_value_data_range,
	     &leaf_value_data_file_index,
	     &leaf_value_data_offset,
	     &leaf_value_data_size,
	     &leaf_value_data_flags,
	     &key_value,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve leaf value data range values.",
		 function );

		return( -1 );
	}
	if( page_tree->io_handle->page_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid page tree - invalid IO handle - missing page size.",
		 function );

		return( -1 );
	}
	if( leaf_value_data_file_index < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid leaf value data file index value less than zero.",
		 function );

		return( -1 );
	}
	if( leaf_value_data_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid leaf value data offset value less than zero.",
		 function );

		return( -1 );
	}
	if( leaf_value_data_size > (off64_t) UINT16_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid leaf value data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	
	page_offset  = leaf_value_data_offset / page_tree->io_handle->page_size;
	page_number  = (uint64_t) ( page_offset + 1 );
	page_offset *= page_tree->io_handle->page_size;

	if( page_number > (uint64_t) UINT32_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid page number value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( libesedb_data_definition_initialize(
	     &data_definition,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create data definition.",
		 function );

		goto on_error;
	}
	if( data_definition == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing data definition.",
		 function );

		goto on_error;
	}
	/* leaf_value_data_file_index contains the page value index
	 */
	data_definition->page_value_index = (uint16_t) leaf_value_data_file_index;
	data_definition->page_offset      = page_offset;
	data_definition->page_number      = (uint32_t) page_number;
	data_definition->data_offset      = (uint16_t) ( leaf_value_data_offset - page_offset );
	data_definition->data_size        = (uint16_t) leaf_value_data_size;

	// Return the loaded data definition.
	*data_def_result = data_definition;

	return( 1 );

on_error:
	if( data_definition != NULL )
	{
		libesedb_data_definition_free(
		 &data_definition,
		 NULL );
	}
	return( -1 );
}


int initialize_search_stacks(
	libesedb_internal_table_t* internal_table,
	libcerror_error_t **error) 
{
	static char *function = "initialize_search_stacks";
	// Initialize tree node search stack.
	if (libesedb_tree_node_stack_create(
		&internal_table->tree_node_search_stack,
		error) != 1 )
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to create tree node search stack.",
			function );

		return -1;
	}

	// Initialize table record search stack.
	if (libesedb_tree_node_stack_create(
		&internal_table->table_record_search_stack,
		error) != 1 )
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			"%s: unable to record search stack.",
			function );

		return -1;
	}

	{
		libfdata_internal_btree_t *internal_tree = (libfdata_internal_btree_t *) 
			internal_table->table_values_tree;
		libfdata_btree_node_t *root_tree_node = NULL;

		// Push root tree node onto the tree node search stack.
		if( libfdata_btree_read_node(
			(libfdata_internal_btree_t*)(internal_table->table_values_tree),
			internal_table->file_io_handle,
			internal_table->table_values_cache,
			internal_tree->root_node_data_range,
			0,
			&root_tree_node,
			(0 | LIBFDATA_READ_FLAG_NO_CACHE),
			error ) != 1 )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_IO,
				LIBCERROR_IO_ERROR_READ_FAILED,
				"%s: unable to read node.",
				function );

			return -1;
		}

		if (root_tree_node == NULL )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: missing node.",
				function );

			return -1;
		}

		// Push value tree node. 
		if (libesedb_tree_node_stack_push(
			internal_table->tree_node_search_stack, 
			root_tree_node,
			error) != 1) 
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to push ROOT NODE onto the search stack.",
				function );

			return -1;
		}
	}
	return 1;
}

int libesedb_tree_node_mark_for_deletion(
	libfdata_tree_node_t *node,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_mark_for_deletion";
	libfdata_internal_tree_node_t *internal_node = NULL;

	if (node == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: tree node is NULL!",
			function );
			
		return -1;
	}

	internal_node = (libfdata_internal_tree_node_t*) node;
	internal_node->timestamp = -1;
	return 1;
}

int libesedb_tree_node_is_marked_for_deletion(
	libfdata_tree_node_t *node,
	libcerror_error_t **error)
{
	static char *function = "libesedb_tree_node_is_marked_for_deletion";
	libfdata_internal_tree_node_t *internal_node = NULL;

	if (node == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: tree node is NULL!",
			function );
			
		return -1;
	}
	internal_node = (libfdata_internal_tree_node_t*) node;
	return internal_node->timestamp == -1;
}


int libesedb_push_subnodes_on_search_stack(
	 libesedb_internal_table_t *table,
	 libfdata_btree_node_t *btree_node,
	 libcerror_error_t **error)
{
	int number_of_sub_nodes = 0;
	int sub_node_index;
	static char *function = "libesedb_search_next_leaf_node";
	
	if (libfdata_btree_node_get_number_of_sub_nodes(
		 btree_node, 
		 &number_of_sub_nodes, error) != 1) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve number of entries from sub nodes array.",
			function );

		return -1;
	}

	// Place the child nodes in the reverse order on the stack.
	for (sub_node_index = number_of_sub_nodes -1;
		sub_node_index >= 0;
		sub_node_index--)
	{
		// libfdata_internal_btree_node_t *internal_sub_node = NULL;
		libfdata_btree_node_t *sub_node;
		libfdata_btree_range_t *sub_node_data_range   = NULL;
		int current_node_level = 0;

		if (libfdata_btree_node_get_sub_node_data_range_by_index(
			btree_node,	
			sub_node_index,
			&sub_node_data_range,
			error ) != 1 )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve sub node: %d data range.",
				function,
				sub_node_index );

			return -1;
		}
		if( sub_node_data_range == NULL )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: missing sub node: %d data range.",
				function,
				sub_node_index );

			return -1;
		}
		if (libfdata_btree_node_get_level(
			btree_node, 
			&current_node_level, 
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_IO,
				LIBCERROR_IO_ERROR_READ_FAILED,
				"%s: unable to read node level.",
				function );
			return -1;
		}

		/* Make sure to not have the node cached here to prevent it from
		* being freed during recursing the sub tree.
		*/
		if( libfdata_btree_read_node(
			( libfdata_internal_btree_t* ) (table->table_values_tree),
			table->file_io_handle,
			table->table_values_cache,
			sub_node_data_range,
			current_node_level + 1,
			&sub_node,
			((libfdata_internal_btree_node_t*)btree_node)->flags | 
				LIBFDATA_READ_FLAG_NO_CACHE |
				LIBFDATA_READ_FLAG_IGNORE_CACHE,
			error ) != 1 )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_IO,
				LIBCERROR_IO_ERROR_READ_FAILED,
				"%s: unable to read node.",
				function );

			return -1;
		}

		if (sub_node == NULL )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: missing node.",
				function );

			return -1;
		}

		if ( libesedb_tree_node_stack_push(
			table->tree_node_search_stack, 
			sub_node,
			error) == -1 ) 
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to push node onto the stack.",
				function );

			return -1;
		}

		// printf("Pushed internal node No.%d onto the tree search stack.\n", sub_node_index);
	}
	return 1;
}

/*	Searches and retrieves next tree node, using depth-first search algorithm.
	Returns 1 on success, otherwise -1.
 */
int libesedb_search_next_leaf_tree_node (
	libesedb_internal_table_t *table,
	libfdata_btree_node_t **record_values_tree_node,
	libcerror_error_t **error )
{
	static char *function = "libesedb_search_next_leaf_node";

	if (table == NULL) 
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: table is NULL!",
			function );
		return -1;
	}
	if (record_values_tree_node == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: record pointer address is null!",
			function );

		return -1;
	}
	if (*record_values_tree_node != NULL)
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
		libfdata_btree_node_t *current_node = NULL;
		libfdata_internal_btree_node_t *internal_node = NULL;
		int result;

		// Remove the top element from the table search stack. 
		if (libesedb_tree_node_stack_pop(
			 table->tree_node_search_stack, 
			 &current_node,
			 error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to remove item from the top of the stack.",
				function );
			
			return -1;
		}

		// If current_node is NULL, 
		// it indicates the end of the search.
		if (current_node == NULL) 
		{
			break;
		}

		internal_node = (libfdata_internal_btree_node_t*) current_node;

		// Check if the current node is LEAF.
		result = libfdata_btree_node_is_leaf(current_node, error); 

		if (result == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to determine whether the node is leaf.",
				function );

			return -1;
		}

		if (result == 1) 
		{
			// A new leaf record node is found: return it immediately. 
			*record_values_tree_node = current_node;
			break;
		}
		else 
		{
			// Push current_node's child nodes onto the search stack and continue search for the next leaf node.
			if (libesedb_push_subnodes_on_search_stack(
				 table, 
				 current_node, 
				 error) != 1) 
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to push sub nodes onto the search stack.",
					function );

				return -1;
			}

			// Deallocation current node: 
			if (current_node != NULL) 
			{
				libfdata_btree_node_free(&current_node, error);
			}

		} // else
	}

	return 1;
}

int libesedb_btree_node_load_record_definitions(
	 libesedb_internal_table_t *internal_table,
	 libfdata_btree_node_t *btree_node,
	 libcerror_error_t **error)
{
	static char *function = "libesedb_btree_node_get_record_definitions";
	int number_of_leaf_values;
	int leaf_value_index;
	libfdata_internal_btree_node_t *internal_node = (libfdata_internal_btree_node_t *) btree_node;

	if (btree_node == NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid btree node: null.",
		 function );

		return( -1 );
	}

	if (internal_node->leaf_value_ranges_array == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
			"%s: invalid node - missing leaf value ranges array.",
			function );

		return( -1 );
	}

	// this is irrelevant. 
	if( ( internal_node->flags & LIBFDATA_FLAG_CALCULATE_MAPPED_RANGES ) == 0 )
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
			"%s: invalid node - unsupported flags calculate mapped ranges is set.",
			function );

		return( -1 );
	}

	if( libcdata_array_get_number_of_entries(
		internal_node->leaf_value_ranges_array,
		&number_of_leaf_values,
		error ) != 1 )
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve number of entries from leaf value ranges array.",
			function );

		return( -1 );
	}

	for(leaf_value_index = number_of_leaf_values - 1;
		leaf_value_index >= 0;
		leaf_value_index-- )
	{
		libesedb_data_definition_t *record_data_definition = NULL;
		libfdata_btree_range_t *leaf_value_data_range = NULL;

		if( libcdata_array_get_entry_by_index(
			 internal_node->leaf_value_ranges_array,
			 leaf_value_index,
			 (intptr_t **) &leaf_value_data_range,
			 error ) != 1 )
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve entry: %d from leaf value ranges array.",
				function,
				leaf_value_index );

			return( -1 );
		}

		if (leaf_value_data_range == NULL)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: missing leaf value: %d data range.",
				function,
				leaf_value_index );

			return( -1 );
		}
		leaf_value_data_range->mapped_first_leaf_value_index = leaf_value_index;
		leaf_value_data_range->mapped_number_of_leaf_values = 1;

		{ // Fetch record definition data.
			libfdata_internal_btree_t * btree = (libfdata_internal_btree_t*)internal_table->table_values_tree;
			if (libesedb_page_tree_load_data_definition(
				 (libesedb_page_tree_t*)btree->data_handle, 
				 &record_data_definition,
				 leaf_value_data_range,
				 error) != 1) 
			{
				libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: illegal leaf value range: %d data range.",
				function,
				leaf_value_index );

				return( -1 );
			}
		}	
		
		if (record_data_definition == NULL)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: illegal leaf value range: %d data range.",
				function,
				leaf_value_index );

			return( -1 );
		}
		
		// Push the record_data_definition to the record stack.
		if (libesedb_tree_node_stack_push(
			 internal_table->table_record_search_stack, 
			 (stack_value_t*)record_data_definition,
			 error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
				"%s: unagle to push record definition to the search stack.",
				function);

			return -1;
		}
		// printf("Pushed record def No.%d onto record stack.\n", leaf_value_index);
	}
	return 1;
}


/*	Searches and retrieves next tree node, using depth-first search algorithm.
	Returns 1 on success, otherwise -1.
 */
int libesedb_search_next_table_record (
	libesedb_internal_table_t *table,
	libesedb_record_t **table_record,
	libcerror_error_t **error )
{
	libesedb_internal_table_t *internal_table = NULL;
	libesedb_data_definition_t *record_data_definition = NULL;
	libfdata_btree_node_t *next_leaf_node = NULL;
	static char *function = "libesedb_search_next_table_record";
	int status;

	if( table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table.",
		 function );

		return -1;
	}
	internal_table = (libesedb_internal_table_t *) table;

	if (table_record == NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record ptr: null.",
		 function );

		return -1;
	}

	if (*table_record != NULL)
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid record value: already set.",
		 function );

		return -1;
	}

	while(1) 
	{
		record_data_definition = NULL;

		// Remove the top element from the table search stack. 
		if (libesedb_tree_node_stack_pop(
			table->table_record_search_stack, 
			(stack_value_t**)&record_data_definition,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to remove item from the top of the stack.",
				function );

			goto on_error;
		}

		// No new record => continue tree node search.
		if (record_data_definition != NULL) 
		{
			// Initialize record and return it.
			if( libesedb_record_initialize(
				table_record,
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
				error ) != 1 )
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					"%s: unable to create record.",
					function );

				goto on_error;
			}
			break;
		}
		else 
		{
			next_leaf_node = NULL;
			// Find next leaf tree node and load its record definitions onto the search stack. 
			if (libesedb_search_next_leaf_tree_node(
				internal_table, 
				&next_leaf_node,
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

			// if no new tree node found -> search is finished!
			if (next_leaf_node == NULL) 
			{
				*table_record = NULL;
				goto on_exit;
			}
			else 
			{
				// load record definitions from the leaf node and push them to the search stack.
				if (libesedb_btree_node_load_record_definitions(
					internal_table, 
					next_leaf_node, 
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
						"%s: unable to load record definitions for a leaf tree node.",
						function );

					goto on_error;
				}
			}
		}
	} // while

on_exit: 
	status = 1;
	if( record_data_definition != NULL )
	{
		libesedb_data_definition_free(&record_data_definition, NULL);
	}
	if (next_leaf_node != NULL)
	{
		libfdata_btree_node_free(&next_leaf_node, NULL);
	}

	return status;

on_error:
	status = -1;
	goto on_exit;
}


/* Retrieves a specific record
 * Returns 1 if successful or -1 on error
 */
int libesedb_table_get_next_record(
     libesedb_table_t *table,
     libesedb_record_t **record,
     libcerror_error_t **error )
{
	// libfdata_tree_node_t *tree_node = NULL;
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

	if (libesedb_search_next_table_record(
		 internal_table, 
		 record, 
		 error) != 1) 
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: unable to fetch next record",
		 function );

		// Just in case, set record ptr to NULL and exit.
		*record = NULL;
		return( -1 );
	}

	return( 1 );
}

#endif