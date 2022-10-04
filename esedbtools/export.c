/* 
 * Export functions
 *
 * Copyright (c) 2010, Joachim Metz <jbmetz@users.sourceforge.net>
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
#include <file_stream.h>
#include <types.h>

#include <libcstring.h>

#include "export.h"

/* Exports binary data
 */
void export_binary_data(
      const uint8_t *data,
      size_t data_size,
      FILE *stream )
{
	if (data == NULL || data_size == 0)
	{
		// fprintf (stream, "00");
		return;
	}

	while( data_size > 0 )
	{
		fprintf(
		 stream,
		 "%02" PRIx8 "",
		 *data );
		data      += 1;
		data_size -= 1;
	}
}

/* Exports narrow character text
 * Converts \\, \t, \n, \r, \0 into their string representation
 */
void export_narrow_text(
      const char *string,
      size_t string_size,
      FILE *stream )
{
	if (string == NULL || string_size == 0)
	{
		// fprintf (stream, "0000");
		return;
	}

	while( string_size > 0 )
	{
		if( *string == 0 )
		{
			if( string_size > 1 )
			{
				fprintf(
					stream,
					"\\0" );
			}
		}
		else if( *string == '\\' )
		{
			fprintf(
				stream,
				"\\\\" );
		}
		else if( *string == '\n' )
		{
			fprintf(
				stream,
				"\\n" );
		}
		else if( *string == '\r' )
		{
			fprintf(
				stream,
				"\\r" );
		}
		else if( *string == '\t' )
		{
			fprintf(
				stream,
				"\\t" );
		}
		else
		{
			fprintf(
				stream,
				"%c",
				*string );
		}
		string      += 1;
		string_size -= 1;
	}
	
}

// #define __USE__BUILTIN__EXPORTER
#ifdef __USE__BUILTIN__EXPORTER

/* Exports system character text
 * Converts \\, \t, \n, \r, \0 into their string representation
 */
void export_text(
      const libcstring_system_character_t *string,
      size_t string_size,
      FILE *stream )
{
	if( string != NULL )
	{
		while( string_size > 0 )
		{
			if( *string == 0 )
			{
				if( string_size > 1 )
				{
					// fprintf(stream, "\\0" );
					// Do nothing with zero bytes...
				}
			}
			else if( *string == (libcstring_system_character_t) '\\' )
			{
				fprintf(
				 stream,
				 "\\\\" );
			}
			else if( *string == (libcstring_system_character_t) '\n' )
			{
				fprintf(
				 stream,
				 "\\n" );
			}
			else if( *string == (libcstring_system_character_t) '\r' )
			{
				fprintf(
				 stream,
				 "\\r" );
			}
			else if( *string == (libcstring_system_character_t) '\t' )
			{
				fprintf(
				 stream,
				 "\\t" );
			}
			else
			{
				fprintf(
				 stream,
				 "%" PRIc_LIBCSTRING_SYSTEM "",
				 *string );
			}
			string      += 1;
			string_size -= 1;
		}
	}
}

#else

/* Exports system character text
 * Converts \\, \t, \n, \r, \0 into their string representation
 */
void export_text(
      const libcstring_system_character_t *string,
      size_t string_size,
      FILE *stream )
{

	if (string == NULL || string_size == 0)
	{
		// fprintf (stream, "0000");
		return;
	}
	else
	{
		unsigned char wc_lower_byte, wc_higher_byte;
		int zero_wchar_count = 0, wc_cnt = 0;
		// Indicator that tells whether two wchars or one wchar is used to encode a UTF16 code point. 
		// String is UTF16LE
		int is_two_wchar_utf16_encoding = 0;
		while (wc_cnt < string_size && zero_wchar_count <= 1)
		{
			if (string[wc_cnt] == 0x00 || string[wc_cnt]== 0x04 || string[wc_cnt]== 0x05 || string[wc_cnt]== 0x06 || string[wc_cnt]==0x3000) 
				zero_wchar_count++;
			wc_cnt++;
		}
		is_two_wchar_utf16_encoding = zero_wchar_count > 1;

		if (string_size % 2 && is_two_wchar_utf16_encoding)
			string_size--;

		while( string_size > 1/* && *string != 0*/)
		{
			// Extract lower byte.
			wc_lower_byte = *string & (wchar_t)0x00FF;

			if (is_two_wchar_utf16_encoding)
				wc_higher_byte = *(string + 1) & (wchar_t)0x00FF;
			else
				wc_higher_byte = *string >> 8; 

			fprintf (stream, "%02x", wc_lower_byte);
			fprintf (stream, "%02x", wc_higher_byte);

			/*
			if( *string == 0 )
			{
			if( string_size > 1 )
			{
			// FIXME: Uncommented because we don't care about Unicode right now. 
			fprintf(stream, "\\0" );
			}
			}
			else if( *string == (libcstring_system_character_t) '\\' )
			{
			fprintf( stream, "\\\\" );
			}
			else if( *string == (libcstring_system_character_t) '\n' )
			{
			fprintf( stream, "\\n" );
			}
			else if( *string == (libcstring_system_character_t) '\r' )
			{
			fprintf( stream, "\\r" );
			}
			else if( *string == (libcstring_system_character_t) '\t' )
			{
			fprintf( stream, "\\t" );
			}
			else if (*string == L'\3')
			{
			// skip control characters...
			printf ("** ** **  FOUND CONTROL CHARACTER ** ** **\n");
			fprintf( stream, "%" PRIc_LIBCSTRING_SYSTEM "" , *string+1 );
			}
			else
			{
			// fwprintf(stream, "%" PRIc_LIBCSTRING_SYSTEM) "", *string );
			fwprintf(stream, L"%lc", *string );
			}
			*/
			string      += (1 + is_two_wchar_utf16_encoding);
			string_size -= (1 + is_two_wchar_utf16_encoding);
		}
	}
	// fprintf (stream, "%02x%02x", '\0', '\0');
}

#endif