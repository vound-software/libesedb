/*
 * Byte stream functions
 *
 * Copyright (C) 2008-2017, Joachim Metz <joachim.metz@gmail.com>
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

#include <common.h>
#include <types.h>

#include "libuna_byte_stream.h"
#include "libuna_definitions.h"
#include "libuna_libcerror.h"
#include "libuna_types.h"
#include "libuna_unicode_character.h"

/* Determines the size of a byte stream from an UTF-8 string
 * Returns 1 if successful or -1 on error
 */
int libuna_byte_stream_size_from_utf8(
     const libuna_utf8_character_t *utf8_string,
     size_t utf8_string_size,
     int codepage,
     size_t *byte_stream_size,
     libcerror_error_t **error )
{
	static char *function                        = "libuna_byte_stream_size_from_utf8";
	size_t utf8_string_index                     = 0;
	libuna_unicode_character_t unicode_character = 0;

	if( utf8_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-8 string.",
		 function );

		return( -1 );
	}
	if( utf8_string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid UTF-8 string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( byte_stream_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid byte stream size.",
		 function );

		return( -1 );
	}
	*byte_stream_size = 0;

	while( utf8_string_index < utf8_string_size )
	{
		/* Convert the UTF-8 character bytes into a Unicode character
		 */
		if( libuna_unicode_character_copy_from_utf8(
		     &unicode_character,
		     utf8_string,
		     utf8_string_size,
		     &utf8_string_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
			 "%s: unable to copy Unicode character from UTF-8.",
			 function );

			return( -1 );
		}
		/* Determine how many byte stream character bytes are required
		 */
		if( libuna_unicode_character_size_to_byte_stream(
		     unicode_character,
		     codepage,
		     byte_stream_size,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_OUTPUT_FAILED,
			 "%s: unable to unable to determine size of Unicode character in byte stream.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Copies a byte stream from an UTF-8 string
 * Returns 1 if successful or -1 on error
 */
int libuna_byte_stream_copy_from_utf8(
     uint8_t *byte_stream,
     size_t byte_stream_size,
     int codepage,
     const libuna_utf8_character_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error )
{
	static char *function                        = "libuna_byte_stream_copy_from_utf8";
	size_t byte_stream_index                     = 0;
	size_t utf8_string_index                     = 0;
	libuna_unicode_character_t unicode_character = 0;

	if( byte_stream == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid byte stream.",
		 function );

		return( -1 );
	}
	if( byte_stream_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid byte stream size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( utf8_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-8 string.",
		 function );

		return( -1 );
	}
	if( utf8_string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid UTF-8 string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	while( utf8_string_index < utf8_string_size )
	{
		/* Convert the UTF-8 string bytes into a Unicode character
		 */
		if( libuna_unicode_character_copy_from_utf8(
		     &unicode_character,
		     utf8_string,
		     utf8_string_size,
		     &utf8_string_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
			 "%s: unable to copy Unicode character from UTF-8 string.",
			 function );

			return( -1 );
		}
		/* Convert the Unicode character into a byte stream
		 */
		if( libuna_unicode_character_copy_to_byte_stream(
		     unicode_character,
		     byte_stream,
		     byte_stream_size,
		     &byte_stream_index,
		     codepage,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_OUTPUT_FAILED,
			 "%s: unable to copy Unicode character to byte stream.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Determines the size of a byte stream from an UTF-16 string
 * Returns 1 if successful or -1 on error
 */
int libuna_byte_stream_size_from_utf16(
     const libuna_utf16_character_t *utf16_string,
     size_t utf16_string_size,
     int codepage,
     size_t *byte_stream_size,
     libcerror_error_t **error )
{
	static char *function                        = "libuna_byte_stream_size_from_utf16";
	size_t utf16_string_index                    = 0;
	libuna_unicode_character_t unicode_character = 0;

	if( utf16_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-16 string.",
		 function );

		return( -1 );
	}
	if( utf16_string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid UTF-16 string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( byte_stream_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid byte stream size.",
		 function );

		return( -1 );
	}
	*byte_stream_size = 0;

	while( utf16_string_index < utf16_string_size )
	{
		/* Convert the UTF-16 character bytes into a Unicode character
		 */
		if( libuna_unicode_character_copy_from_utf16(
		     &unicode_character,
		     utf16_string,
		     utf16_string_size,
		     &utf16_string_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
			 "%s: unable to copy Unicode character from UTF-16.",
			 function );

			return( -1 );
		}
		/* Determine how many byte stream character bytes are required
		 */
		if( libuna_unicode_character_size_to_byte_stream(
		     unicode_character,
		     codepage,
		     byte_stream_size,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_OUTPUT_FAILED,
			 "%s: unable to unable to determine size of Unicode character in byte stream.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Copies a byte stream from an UTF-16 string
 * Returns 1 if successful or -1 on error
 */
int libuna_byte_stream_copy_from_utf16(
     uint8_t *byte_stream,
     size_t byte_stream_size,
     int codepage,
     const libuna_utf16_character_t *utf16_string,
     size_t utf16_string_size,
     libcerror_error_t **error )
{
	static char *function                        = "libuna_byte_stream_copy_from_utf16";
	size_t byte_stream_index                     = 0;
	size_t utf16_string_index                    = 0;
	libuna_unicode_character_t unicode_character = 0;

	if( byte_stream == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid byte stream.",
		 function );

		return( -1 );
	}
	if( byte_stream_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid byte stream size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( utf16_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-16 string.",
		 function );

		return( -1 );
	}
	if( utf16_string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid UTF-16 string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	while( utf16_string_index < utf16_string_size )
	{
		/* Convert the UTF-16 string bytes into a Unicode character
		 */
		if( libuna_unicode_character_copy_from_utf16(
		     &unicode_character,
		     utf16_string,
		     utf16_string_size,
		     &utf16_string_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
			 "%s: unable to copy Unicode character from UTF-16 string.",
			 function );

			return( -1 );
		}
		/* Convert the Unicode character into a byte stream
		 */
		if( libuna_unicode_character_copy_to_byte_stream(
		     unicode_character,
		     byte_stream,
		     byte_stream_size,
		     &byte_stream_index,
		     codepage,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_OUTPUT_FAILED,
			 "%s: unable to copy Unicode character to byte stream.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Determines the size of a byte stream from an UTF-32 string
 * Returns 1 if successful or -1 on error
 */
int libuna_byte_stream_size_from_utf32(
     const libuna_utf32_character_t *utf32_string,
     size_t utf32_string_size,
     int codepage,
     size_t *byte_stream_size,
     libcerror_error_t **error )
{
	static char *function                        = "libuna_byte_stream_size_from_utf32";
	size_t utf32_string_index                    = 0;
	libuna_unicode_character_t unicode_character = 0;

	if( utf32_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-32 string.",
		 function );

		return( -1 );
	}
	if( utf32_string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid UTF-32 string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( byte_stream_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid byte stream size.",
		 function );

		return( -1 );
	}
	*byte_stream_size = 0;

	while( utf32_string_index < utf32_string_size )
	{
		/* Convert the UTF-32 character bytes into a Unicode character
		 */
		if( libuna_unicode_character_copy_from_utf32(
		     &unicode_character,
		     utf32_string,
		     utf32_string_size,
		     &utf32_string_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
			 "%s: unable to copy Unicode character from UTF-32.",
			 function );

			return( -1 );
		}
		/* Determine how many byte stream character bytes are required
		 */
		if( libuna_unicode_character_size_to_byte_stream(
		     unicode_character,
		     codepage,
		     byte_stream_size,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_OUTPUT_FAILED,
			 "%s: unable to unable to determine size of Unicode character in byte stream.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Copies a byte stream from an UTF-32 string
 * Returns 1 if successful or -1 on error
 */
int libuna_byte_stream_copy_from_utf32(
     uint8_t *byte_stream,
     size_t byte_stream_size,
     int codepage,
     const libuna_utf32_character_t *utf32_string,
     size_t utf32_string_size,
     libcerror_error_t **error )
{
	static char *function                        = "libuna_byte_stream_copy_from_utf32";
	size_t byte_stream_index                     = 0;
	size_t utf32_string_index                    = 0;
	libuna_unicode_character_t unicode_character = 0;

	if( byte_stream == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid byte stream.",
		 function );

		return( -1 );
	}
	if( byte_stream_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid byte stream size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( utf32_string == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid UTF-32 string.",
		 function );

		return( -1 );
	}
	if( utf32_string_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid UTF-32 string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	while( utf32_string_index < utf32_string_size )
	{
		/* Convert the UTF-32 string bytes into a Unicode character
		 */
		if( libuna_unicode_character_copy_from_utf32(
		     &unicode_character,
		     utf32_string,
		     utf32_string_size,
		     &utf32_string_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
			 "%s: unable to copy Unicode character from UTF-32 string.",
			 function );

			return( -1 );
		}
		/* Convert the Unicode character into a byte stream
		 */
		if( libuna_unicode_character_copy_to_byte_stream(
		     unicode_character,
		     byte_stream,
		     byte_stream_size,
		     &byte_stream_index,
		     codepage,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_CONVERSION,
			 LIBCERROR_CONVERSION_ERROR_OUTPUT_FAILED,
			 "%s: unable to copy Unicode character to byte stream.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

