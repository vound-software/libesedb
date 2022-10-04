#include <common.h>
#include <memory.h>
#include <types.h>

#include <liberror.h>
#include <libnotify.h>

#include "libfdata_date_time.h"
#include "libfdata_definitions.h"
#include "libfdata_libbfio.h"
#include "libfdata_libfcache.h"
#include "libfdata_tree.h"
#include "libfdata_tree_node.h"
#include "libfdata_types.h"


/* Retrieves a leaf nodes at a specific index
 * Returns 1 if successful or -1 on error
 */
/*
int libfdata_tree_get_leaf_node_by_index(
     libfdata_tree_t *tree,
     libbfio_handle_t *file_io_handle,
     libfcache_cache_t *cache,
     int leaf_node_index,
     libfdata_tree_node_t **leaf_node,
     uint8_t read_flags,
     liberror_error_t **error )
{
	libfdata_internal_tree_t *internal_tree = NULL;
	static char *function                   = "libfdata_tree_get_leaf_node_by_index";

	if( tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid tree.",
		 function );

		return( -1 );
	}
	internal_tree = (libfdata_internal_tree_t *) tree;

	if( libfdata_tree_node_get_leaf_node_by_index(
	     internal_tree->root_node,
	     file_io_handle,
	     cache,
	     leaf_node_index,
	     leaf_node,
	     read_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve leaf node: %d from root node.",
		 function,
		 leaf_node_index );

		return( -1 );
	}
	return( 1 );
}
*/

/* Retrieves a specific leaf node
 * Returns 1 if successful or -1 on error
 */
/*
int libfdata_tree_node_load_subnodes(...);
*/
/*
int libfdata_tree_node_get_leaf_node_by_index(
     libfdata_tree_node_t *node,
     libbfio_handle_t *file_io_handle,
     libfcache_cache_t *cache,
     int leaf_node_index,
     libfdata_tree_node_t **leaf_node,
     uint8_t read_flags,
     liberror_error_t **error )
{
	libfdata_internal_tree_node_t *internal_tree_node = NULL;
	libfdata_tree_node_t *sub_node                    = NULL;
	static char *function                             = "libfdata_tree_node_get_leaf_node_by_index";
	int first_deleted_leaf_node_index                 = 0;
	int first_leaf_node_index                         = 0;
	int number_of_leaf_nodes                          = 0;
	int number_of_sub_nodes                           = 0;
	int result                                        = 0;
	int sub_node_index                                = 0;

	if( node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid node.",
		 function );

		return( -1 );
	}
	internal_tree_node = (libfdata_internal_tree_node_t *) node;

	if( leaf_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid leaf node.",
		 function );

		return( -1 );
	}
	if( ( internal_tree_node->flags & LIBFDATA_TREE_NODE_FLAG_CALCULATE_LEAF_NODE_VALUES ) != 0 )
	{
		if( libfdata_tree_node_read_leaf_node_values(
		     node,
		     file_io_handle,
		     cache,
		     read_flags,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve node value.",
			 function );

			return( -1 );
		}
		internal_tree_node->flags &= ~( LIBFDATA_TREE_NODE_FLAG_CALCULATE_LEAF_NODE_VALUES );
	}
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

		return( -1 );
	}
	for( sub_node_index = 0;
	     sub_node_index < number_of_sub_nodes;
	     sub_node_index++ )
	{
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
		if( libfdata_tree_node_get_number_of_leaf_nodes(
		     sub_node,
		     file_io_handle,
		     cache,
		     &number_of_leaf_nodes,
		     read_flags,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve number of leaf nodes from sub node: %d.",
			 function,
			 sub_node_index );

			return( -1 );
		}
		if( libfdata_tree_node_get_first_leaf_node_index(
		     sub_node,
		     &first_leaf_node_index,
		     &first_deleted_leaf_node_index,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve first leaf node index from sub node: %d.",
			 function,
			 sub_node_index );

			return( -1 );
		}
		if( number_of_leaf_nodes == 0 )
		{
			 // It is possible to have branch nodes without leaf nodes
			 
			result = libfdata_tree_node_is_leaf(
				  sub_node,
				  file_io_handle,
				  cache,
				  read_flags,
				  error );

			if( result == -1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to determine if sub node: %d is a leaf.",
				 function,
				 sub_node_index );

				return( -1 );
			}
			else if( result != 0 )
			{
				result = libfdata_tree_node_is_deleted(
					  sub_node,
					  error );

				if( result == -1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to determine if sub node: %d is deleted.",
					 function,
					 sub_node_index );

					return( -1 );
				}
				else if( result == 0 )
				{
					if( leaf_node_index == first_leaf_node_index )
					{
						*leaf_node = sub_node;

						break;
					}
				}
			}
		}
		else if( ( leaf_node_index >= first_leaf_node_index )
		      && ( leaf_node_index < ( first_leaf_node_index + number_of_leaf_nodes ) ) )
		{
			// Note that the first_leaf_node_index is relative for a branch
			 
			if( libfdata_tree_node_get_leaf_node_by_index(
			     sub_node,
			     file_io_handle,
			     cache,
			     leaf_node_index - first_leaf_node_index,
			     leaf_node,
			     read_flags,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve leaf node: %d from sub node: %d.",
				 function,
				 leaf_node_index - first_leaf_node_index,
				 sub_node_index );

				return( -1 );
			}
			break;
		}
	}
	if( sub_node_index >= number_of_sub_nodes )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid sub node index value out of bounds.",
		 function );

		return( -1 );
	}
	return( 1 );
}
*/

/* Reads the leaf node values
 * Returns 1 if the successful or -1 on error
 */
/*
int libfdata_tree_node_read_subnodes(...);
*/


/* Retrieves a leaf nodes at a specific index
 * Returns 1 if successful or -1 on error
 */
/*
int libfdata_tree_get_leaf_node_by_index(
     libfdata_tree_t *tree,
     libbfio_handle_t *file_io_handle,
     libfcache_cache_t *cache,
     int leaf_node_index,
     libfdata_tree_node_t **leaf_node,
     uint8_t read_flags,
     liberror_error_t **error )
{
	libfdata_internal_tree_t *internal_tree = NULL;
	static char *function                   = "libfdata_tree_get_leaf_node_by_index";

	if( tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid tree.",
		 function );

		return( -1 );
	}
	internal_tree = (libfdata_internal_tree_t *) tree;

	if( libfdata_tree_node_get_leaf_node_by_index(
	     internal_tree->root_node,
	     file_io_handle,
	     cache,
	     leaf_node_index,
	     leaf_node,
	     read_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve leaf node: %d from root node.",
		 function,
		 leaf_node_index );

		return( -1 );
	}
	return( 1 );
}
*/

/*
int libfdata_tree_node_is_leaf(
	libfdata_tree_node_t *node,
	libbfio_handle_t *file_io_handle,
	libfcache_cache_t *cache,
	uint8_t read_flags,
	liberror_error_t **error )
{
	//It is possible to have branch nodes without leaf nodes
	
	result = libfdata_tree_node_is_leaf(
		sub_node,
		file_io_handle,
		cache,
		read_flags,
		error );

	if( result == -1 )
	{
		liberror_error_set(
			error,
			LIBERROR_ERROR_DOMAIN_RUNTIME,
			LIBERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to determine if sub node: %d is a leaf.",
			function,
			sub_node_index );

		return( -1 );
	}
	else if (result == 0) 
	{
		printf ("Discovered NON_LEAF subnode (%p).\n", sub_node);
		return 0;
	}
	else if( result != 0 )
	{
		result = libfdata_tree_node_is_deleted(
			sub_node,
			error );

		if( result == -1 )
		{
			liberror_error_set(
				error,
				LIBERROR_ERROR_DOMAIN_RUNTIME,
				LIBERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to determine if sub node: %d is deleted.",
				function,
				sub_node_index );

			return( -1 );
		}
		else if( result == 0 )
		{
			// this is a regular leaf node!
			internal_tree_node->number_of_leaf_nodes++;
			printf ("Discovered LEAF subnode (%p).\n", sub_node);
			return 1;
		}
		else
		{
			// this is a deleted leaf node!
			internal_tree_node->number_of_deleted_leaf_nodes++;
			printf ("Discovered DELETED LEAF subnode (%p).\n", sub_node);
			return 2;
		}
	}
}
*/