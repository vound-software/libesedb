/**
 * This header file contains declarations of operations that comprise the libesedb performance patch. 
 * The libesedb performance patch addresses the existing inefficient and resource-consuming breadth-first btree traversal algorithm
 * and replaces it with a more efficient depth-first traversal algorithm.
 * 
 * The patch speeds up retrieveal of both table records and BLOBs. 
 * 
 * Author: Nemanja Kojic
 * Last Modified: 21-Apr-2020
 */
#ifdef LIBESEDB_PERFORMANCE_PATCH

#if !defined( _LIBESEDB_UTILS_PATCH_H )
#define _LIBESEDB_UTILS_PATCH_H

#if defined( __cplusplus )
extern "C" {
#endif

#include "libesedb_table.h"
#include "libfdata_tree_node.h"
#include "libesedb_page_tree.h"
#include "libesedb_data_definition.h"


int load_btree_tree_node(
	libesedb_internal_table_t *table,
	libfdata_btree_range_t *sub_node_data_range,
	libfdata_btree_node_t **sub_node,
	libcerror_error_t **error);

int libfdata_btree_traverse_sub_tree(
	libesedb_table_t *table,
	libfdata_btree_range_t *node_data_range,
	int level,
	int mapped_first_leaf_value_index,
	libfdata_btree_node_t **node,
	uint8_t read_flags,
	long* key_counter,
	libcerror_error_t **error);

/* Creates a long value
 * Make sure the value long_value is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libesedb_create_long_value_from_data_segments(
	libesedb_long_value_t **long_value,
	libbfio_handle_t *file_io_handle,
	libesedb_io_handle_t *io_handle,
	libfdata_list_t *data_segments_list,
	libcerror_error_t **error);

/* Retrieves the long value data from a long value
* Returns 1 if successful or -1 on error
*/
int libesedb_read_long_value_bytes(
	libesedb_long_value_t *long_value,
	uint8_t **long_value_data,
	size_t *long_value_data_size,
	libcerror_error_t **error);

int libesedb_table_get_next_blob_segment(
	libesedb_table_t *table,
	size_t *blobSegmentKeySize,
	uint8_t **blobSegmentKeyBytes,
	size_t *blobSegmentDataSize,
	uint8_t **blobSegmentDataBytes,
	libcerror_error_t **error);

/* Reads a page tree leaf value
 * Callback function for the page tree
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_read_data_definition(
	libesedb_page_tree_t *page_tree,
	libfdata_btree_range_t *leaf_value_data_range,
	libesedb_data_definition_t **data_definition,
	libcerror_error_t **error);

/* Retrieves the long value key of a specific entry
 * Creates a new long value
 * Returns 1 if successful, 0 if the item does not contain such value or -1 on error
 */
int libesedb_record_get_long_value_key(
	libesedb_record_t *record,
	int value_entry,
	uint8_t **long_value_key_bytes,
	size_t *long_value_key_size,
	libcerror_error_t **error);

int libesedb_memory_free_byte_array(
	uint8_t **byte_array,
	libcerror_error_t **error);

int libesedb_libfdata_btree_range_clone(
	libfdata_btree_range_t *btree_range,
	libfdata_btree_range_t **btree_range_clone,
	libcerror_error_t **error);

int libesedb_traverse_blob_tree(
	libesedb_table_t *table,
	uint8_t read_flags,
	libcerror_error_t **error);

#if defined( __cplusplus )
}
#endif

#endif

#endif