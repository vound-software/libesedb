/*
 * Service provider identifier functions
 *
 * Copyright (c) 2009-2012, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined( _LIBFMAPI_SERVICE_PROVIDER_IDENTIFIER_H )
#define _LIBFMAPI_SERVICE_PROVIDER_IDENTIFIER_H

#include <common.h>
#include <types.h>

#include <liberror.h>

#if defined( __cplusplus )
extern "C" {
#endif

extern uint8_t libfmapi_service_provider_identifier_one_off_entry_identifier[ 16 ];
extern uint8_t libfmapi_service_provider_identifier_x500_address[ 16 ];

#if defined( HAVE_DEBUG_OUTPUT )

const char *libfmapi_service_provider_identifier_get_name(
             uint8_t *service_provider_identifier );

#endif

#if defined( __cplusplus )
}
#endif

#endif

