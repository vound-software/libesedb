#include "libesedb_definitions.h"
#include "libesedb_record.h"
#include "libesedb_table.h"
#include "memory.h"

#include <stdlib.h>

#ifdef LIBESEDB_RECORD_ACCESS_PATCH

int libesedb_tree_node_mark_for_deletion(
	libfdata_tree_node_t *node,
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_mark_for_deletion";
	libfdata_internal_tree_node_t *internal_node = NULL;

	if (node == NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
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
	liberror_error_t **error)
{
	static char *function = "libesedb_tree_node_is_marked_for_deletion";
	libfdata_internal_tree_node_t *internal_node = NULL;

	if (node == NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: tree node is NULL!",
			function );
			
		return -1;
	}
	internal_node = (libfdata_internal_tree_node_t*) node;
	return internal_node->timestamp == -1;
}

/*	Searches and retrieves next tree node, using depth-first search algorithm.
	Returns 1 on success, otherwise -1.
 */
int libesedb_search_next_leaf_node (
	libesedb_internal_table_t *table,
	libfdata_tree_node_t **record_values_tree_node,
	liberror_error_t **error )
{
	static char *function = "libesedb_search_next_leaf_node";

	if (table == NULL) 
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: table is NULL!",
			function );
		return -1;
	}

	if (record_values_tree_node == NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: record pointer address is null!",
			function );

		return -1;
	}

	if (*record_values_tree_node != NULL)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: the pointer to the record is already set.",
			function );

		return -1;
	}

	do 
	{
		libfdata_tree_node_t *current_node = NULL;
		int result;

		// Remove the top element from the table search stack. 
		if ( libesedb_tree_node_stack_pop(
			table->stack, 
			&current_node,
			error) != 1 )
		{
			liberror_error_set(
				error,
				LIBERROR_ERROR_DOMAIN_RUNTIME,
				LIBERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to remove item from the top of the stack.",
				function );
			
			return -1;
		}

		// If current_node is NULL, 
		// it indicates the end of the search.
		if (current_node == NULL)
			break;

		/* REMARK:	EXPLICIT TREE NODE DELETION NOT NEEDED. 
		 *			THE TABLE IS RESPONSIBLE FOR NODES DESTRUCTION.
		 *			THE LIBRARY CRASHES IF NODES ARE DESTROYED HERE.
		 *
		 *			This code is left commented out, just as a reminder for future ehancements. 
		 *			Currently, nodes are maintained by the underlying bounded cache!
		 */
		/*
		// Check whether current node is marked for deletion.
		// This is a special node, that needs to be deleted. 
		// Its subnodes are already processed.
		result = libesedb_tree_node_is_marked_for_deletion(
					current_node, 
					error);
		if (result == -1)
		{
			liberror_error_set(
				error,
				LIBERROR_ERROR_DOMAIN_RUNTIME,
				LIBERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to check whether the current node was marked for deletion.",
				function );
			
			continue;
		}

		// Current node was marked for deletion, which means it is 
		// an internal node whose subnodes are completely processed. 
		if (result == 1)
		{
			// Deallocate the internal tree node.
			// Its subnodes are already processed.
			if (libfdata_tree_node_free_single(
				&current_node,
				error) != 1)
			{
				liberror_error_set(
					error,
					LIBERROR_ERROR_DOMAIN_RUNTIME,
					LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					"%s: unable to free tree node.",
					function );
				return -1;
			}

			// Continue search iterations. 
			continue;
		}
		*/

		/* This function reads/(pre)fetches subnodes of the current node first, 
		 *  and then checks whether the current_node is leaf or not.
		 */
		result = libfdata_tree_node_is_leaf(
					current_node, 
					table->file_io_handle,
					table->table_values_cache,
					0, 
					error ); 

		if (result == -1)
		{
			liberror_error_set(
				error,
				LIBERROR_ERROR_DOMAIN_RUNTIME,
				LIBERROR_RUNTIME_ERROR_GENERIC,
				"%s: unable to determine whether the node is leaf.",
				function );

			return -1;
		}

		// If the node is leaf, set the output parameter and break the search loop.
		if (result == 1) 
		{
			// A new leaf record node is found: return it immediately. 
			*record_values_tree_node = current_node;
			break;
		}
		else 
		{
			libfdata_internal_tree_node_t * internal_tree_node = (libfdata_internal_tree_node_t *) current_node;
			int number_of_sub_nodes = 0;
			int sub_node_index;
			libfdata_tree_node_t *sub_node                    = NULL;
			
			// REMARK:	Explicit node deletion, during the search, is not currently supported. 
			//			This may be considered as a future enhancement.
			//			Currently, nodes are maintained by the underlying bounded cache!
			/*
			// Mark current internal node for deletion
			// and return it onto the stack.
			if (libesedb_tree_node_mark_for_deletion(
				current_node, 
				error) != 1)
			{
				liberror_error_set(
					error,
					LIBERROR_ERROR_DOMAIN_RUNTIME,
					LIBERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to mark current node for deletion.",
					function );

				return -1;
			}

			// Return current node as marked for deletion to the stack. 
			if ( libesedb_tree_node_stack_push(
				table->stack, 
				current_node,
				error) == -1 ) 
			{
				liberror_error_set(
					error,
					LIBERROR_ERROR_DOMAIN_RUNTIME,
					LIBERROR_RUNTIME_ERROR_GENERIC,
					"%s: unable to push current node onto the stack.",
					function );

				return -1;
			}
			*/

			// If current node is not leaf, its children are put onto the stack.
			// The node's child nodes are already fetched by libfdata_tree_node_is_leaf operation.
			if( libfdata_array_get_number_of_entries(
				internal_tree_node->sub_nodes,
				&number_of_sub_nodes,
				error ) != 1 )
			{
				liberror_error_set(
					error,
					LIBERROR_ERROR_DOMAIN_RUNTIME,
					LIBERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve number of entries from sub nodes array.",
					function );

				return -1;
			}

			for( sub_node_index = number_of_sub_nodes -1;
				sub_node_index >= 0;
				sub_node_index--)
			{
				libfdata_internal_tree_node_t *internal_sub_node = NULL;

				// Get sub_node by index and push it onto the stack.
				if( libfdata_array_get_entry_by_index(
					internal_tree_node->sub_nodes,
					sub_node_index,
					(intptr_t **) &sub_node,
					error ) != 1 )
				{
					liberror_error_set(
						error,
						LIBERROR_ERROR_DOMAIN_RUNTIME,
						LIBERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve entry: %d from sub nodes array.",
						function,
						sub_node_index );

					return( -1 );
				}
				
				internal_sub_node = (libfdata_internal_tree_node_t*) sub_node;
				
				if ( libesedb_tree_node_stack_push(
						table->stack, 
						sub_node,
						error) == -1 ) 
				{
					liberror_error_set(
						error,
						LIBERROR_ERROR_DOMAIN_RUNTIME,
						LIBERROR_RUNTIME_ERROR_GENERIC,
						"%s: unable to push node onto the stack.",
						function );

					return -1;
				}
			}
		} // else

	} while (1);

	return 1;
}


/* Retrieves a specific record
 * Returns 1 if successful or -1 on error
 */
int libesedb_table_get_next_record(
     libesedb_table_t *table,
     libesedb_record_t **record,
     liberror_error_t **error )
{
	libfdata_tree_node_t *record_values_tree_node = NULL;
	libesedb_internal_table_t *internal_table     = NULL;
	static char *function                         = "libesedb_table_get_next_record";

	if( table == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table.",
		 function );

		return( -1 );
	}
	internal_table = (libesedb_internal_table_t *) table;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}

	if( *record != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid record value already set.",
		 function );

		return( -1 );
	}

	if ( libesedb_search_next_leaf_node(
			internal_table, 
			&record_values_tree_node,
			error) != 1)
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			"%s: unable to find next leaf node.",
			function );
		
		return -1;
	}

	if (record_values_tree_node != NULL)
	{
		if( libesedb_record_initialize(
			record,
			internal_table->file_io_handle,
			internal_table->io_handle,
			internal_table->table_definition,
			internal_table->template_table_definition,
			internal_table->pages_vector,
			internal_table->pages_cache,
			internal_table->long_values_pages_vector,
			internal_table->long_values_pages_cache,
			record_values_tree_node,
			internal_table->table_values_cache,
			internal_table->long_values_tree,
			internal_table->long_values_cache,
			LIBESEDB_ITEM_FLAGS_DEFAULT,
			error ) != 1 )
		{
			liberror_error_set(
				error,
				LIBERROR_ERROR_DOMAIN_RUNTIME,
				LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				"%s: unable to create record.",
				function );

			return( -1 );
		}
		/*
		if (libfdata_tree_node_free_single(
			&record_values_tree_node,
			error) != 1)
		{
			liberror_error_set(
				error,
				LIBERROR_ERROR_DOMAIN_RUNTIME,
				LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				"%s: unable to free tree node.",
				function );
			return -1;
		}
		*/
	}
	else 
	{
		*record = NULL;
	}

	return( 1 );
}

#endif