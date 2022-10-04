#if !defined( _LIBESEDB_UTILS_PATCH_H ) && defined(LIBESEDB_PERFORMANCE_PATCH)
#define _LIBESEDB_UTILS_PATCH_H

#if defined( __cplusplus )
extern "C" {
#endif

#include "libesedb_table.h"

int initialize_search_stacks(
	 libesedb_internal_table_t*,
	 libcerror_error_t**);

#if defined( __cplusplus )
}
#endif

#endif