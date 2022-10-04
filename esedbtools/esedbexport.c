/*
 * Extracts tables from an Extensible Storage Engine (ESE) Database (EDB) file
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
#include <windows.h>

#include <common.h>
#include <file_stream.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

#include <libsystem.h>

#include "esedboutput.h"
#include "esedbtools_libesedb.h"
#include "export_handle.h"
#include "log_handle.h"

// #include "../intella_edb/intella_edb_concepts.h"
// #include "../intella_edb/intella_edb_hashtable.h"
// #include "../intella_edb/intella_edb_filesystem_utils.h"
// #include "../intella_edb/intella_edb_print.h"
// #include "../intella_edb/intella_edb_import_export.h"
// #include "edb_utils.h"

#include <crtdbg.h>
#include <signal.h>

#include <time.h>

export_handle_t *esedbexport_export_handle = NULL;
int esedbexport_abort                      = 0;

/* Prints the executable usage information
 */
void usage_fprint(FILE *stream )
{
	if(stream == NULL )
	{
		return;
	}
	fprintf( stream, "Use esedbexport to export items stored in an Extensible Storage Engine (ESE)\n"
	                 "Database (EDB) file\n\n" );

	fprintf( stream, "Usage: esedbexport [ -c codepage ] [ -l logfile ] [ -m mode ] [ -t target ]\n"
	                 "                   [ -T table_name ] [ -hvV ] source\n\n" );

	fprintf( stream, "\tsource: the source file\n\n" );

	fprintf( stream, "\t-c:     codepage of ASCII strings, options: ascii, windows-874,\n"
	                 "\t        windows-932, windows-936, windows-1250, windows-1251,\n"
	                 "\t        windows-1252 (default), windows-1253, windows-1254\n"
	                 "\t        windows-1255, windows-1256, windows-1257 or windows-1258\n" );
	fprintf( stream, "\t-h:     shows this help\n" );
	fprintf( stream, "\t-l:     logs information about the exported items\n" );
	fprintf( stream, "\t-m:     export mode, option: all, tables (default)\n"
	                 "\t        'all' exports all the tables or a single specified table with indexes,\n"
	                 "\t        'tables' exports all the tables or a single specified table\n" );
	fprintf( stream, "\t-t:     specify the basename of the target directory to export to\n"
	                 "\t        (default is the source filename) esedbexport will add the suffix\n"
	                 "\t        .export to the basename\n" );
	fprintf( stream, "\t-T:     exports only a specific table\n" );
	fprintf( stream, "\t-v:     verbose output to stderr\n" );
	fprintf( stream, "\t-V:     print version\n" );
}

void edb_usage_fprint(FILE *stream )
{
	if(stream == NULL )
	{
		return;
	}
	fprintf( stream, "\nUse edbxtract to export items stored in an Extensible Storage Engine (ESE)\n"
	                 "Database (EDB) file.\n\n" );

	fprintf( stream, "Usage: edbxtract -f <edb file> [-l <log file>] [-v]\n\n" );
	fprintf( stream, "\t-f <edb file>:	\tThe source edb file\n\n" );
	fprintf( stream, "\t-l <log file>:	\tLog file (optional).\n" );
	fprintf( stream, "\t-v:			\tEnables verbose output (optional).\n" );
	fprintf( stream, "\nThe extractor creates output file \"edb-output.txt\".\n");
}

/* Signal handler for esedbexport
 */
void esedbexport_signal_handler(
      libsystem_signal_t signal LIBSYSTEM_ATTRIBUTE_UNUSED )
{
	liberror_error_t *error = NULL;
	static char *function   = "esedbexport_signal_handler";

	LIBSYSTEM_UNREFERENCED_PARAMETER( signal )

	esedbexport_abort = 1;

	if( esedbexport_export_handle != NULL )
	{
		if( export_handle_signal_abort(
		     esedbexport_export_handle,
		     &error ) != 1 )
		{
			libsystem_notify_printf(
			 "%s: unable to signal export handle to abort.\n",
			 function );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
		}
	}
	/* Force stdin to close otherwise any function reading it will remain blocked
	 */
	if( libsystem_file_io_close(
	     0 ) != 0 )
	{
		libsystem_notify_printf(
		 "%s: unable to close stdin.\n",
		 function );
		
	}
}

/* The main program
 */
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	boolean enabledFeature = 1;
	libcstring_system_character_t *log_filename          = NULL;
	libcstring_system_character_t *option_ascii_codepage = NULL;
	libcstring_system_character_t *option_export_mode    = NULL;
	libcstring_system_character_t *option_table_name     = NULL;
	libcstring_system_character_t *option_target_path    = NULL;
	libcstring_system_character_t *path_separator        = NULL;
	libcstring_system_character_t *source                = NULL;
	liberror_error_t *error                              = NULL;
	log_handle_t *log_handle                             = NULL;
	char *program                                        = "esedbexport";
	size_t source_length                                 = 0;
	size_t option_table_name_length                      = 0;
	libcstring_system_integer_t option                   = 0;
	int result                                           = 0;
	int verbose                                          = 0;
	int tmpDbgFlag;

	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
	/*
	* Set the debug-heap flag to keep freed blocks in the
	* heap's linked list - This will allow us to catch any
	* inadvertent use of freed memory
	*/
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	tmpDbgFlag |= _CRTDBG_CHECK_CRT_DF;
	_CrtSetDbgFlag(tmpDbgFlag);

	libsystem_notify_set_stream( stderr, NULL );
	libsystem_notify_set_verbose( 1 );
	
	// print_data_pages ("test2007.edb");
	
	if( libsystem_initialize("esedbtools",_IONBF, &error ) != 1 )
	{
		fprintf(stderr, "Unable to initialize system values.\n" );
		libsystem_notify_print_error_backtrace( error );
		liberror_error_free( &error );
		return( EXIT_FAILURE );
	}
	esedboutput_version_fprint( stdout, program );

	while(( option = libsystem_getopt(
	                   argc,
	                   argv,
	                   _LIBCSTRING_SYSTEM_STRING( "f:c:hl:m:t:T:vV" ) ) ) != (libcstring_system_integer_t) -1 )
	{
		switch( option )
		{
			case (libcstring_system_integer_t) '?':
			default:
				fprintf(
				 stderr,
				 "Invalid argument: %" PRIs_LIBCSTRING_SYSTEM "\n",
				 argv[ optind - 1 ] );

				usage_fprint(
				 stdout );

				return( EXIT_FAILURE );

			case (libcstring_system_integer_t) 'c':
				option_ascii_codepage = optarg;

				break;

			case (libcstring_system_integer_t) 'h':
				usage_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (libcstring_system_integer_t) 'l':
				log_filename = optarg;

				break;

			case (libcstring_system_integer_t) 'm':
				option_export_mode = optarg;

				break;

			case (libcstring_system_integer_t) 't':
				option_target_path = optarg;

				break;

			case (libcstring_system_integer_t) 'T':
				option_table_name = optarg;

				break;

			case (libcstring_system_integer_t) 'v':
				verbose = 1;
				// verbose_output = 1;
				break;

			case (libcstring_system_integer_t) 'f':
				source = optarg;

				break;

			case (libcstring_system_integer_t) 'V':
				esedboutput_copyright_fprint(
				 stdout );

				return( EXIT_SUCCESS );

		}
	}

	if (argc < 2) 
	{
		fprintf( stderr, "Wrong command. See usage instructions.\n" );
		edb_usage_fprint( stdout );
		return( EXIT_FAILURE );
	}
	//if( optind == argc )
	//{
	//	fprintf( stderr, "Missing source file.\n" );
	//	edb_usage_fprint( stdout );
	//	return( EXIT_FAILURE );
	//}
	// source = argv[ 1 ];
	/*
	if (argc == 3) 
	{
		if (libcstring_system_string_compare(argv[2], _LIBCSTRING_SYSTEM_STRING( "-v" ), 2 ) == 0)
			verbose_output = true;
		else
		{
			fprintf( stderr, "Wrong switch detected. Expected \"-v\". See usage instructions.\n" );
			edb_usage_fprint( stdout );
			return( EXIT_FAILURE );
		}
	}
	*/
	/*
	if (libcstring_system_string_compare(argv[2], _LIBCSTRING_SYSTEM_STRING( "2003" ), 4 ) == 0) 
		edb_version = EDB_2003;
	else if (libcstring_system_string_compare(argv[2], _LIBCSTRING_SYSTEM_STRING( "2007" ), 4 ) == 0) 
		edb_version = EDB_2007;
	else if (libcstring_system_string_compare(argv[2], _LIBCSTRING_SYSTEM_STRING( "2010" ), 4 ) == 0) 
		edb_version = EDB_2010;
	else
	{
		fprintf( stderr, "Wrong edb version value. See usage instructions.\n" );
		edb_usage_fprint( stdout );
		return( EXIT_FAILURE );
	}
	*/

	if(enabledFeature && option_target_path == NULL )
	{
		source_length = libcstring_system_string_length( source );

		path_separator = libcstring_system_string_search_character_reverse(
		                  source,
		                  (libcstring_system_character_t) LIBSYSTEM_PATH_SEPARATOR,
		                  source_length );

		if( path_separator == NULL )
		{
			path_separator = source;
		}
		else
		{
			path_separator++;
		}
		option_target_path = path_separator;
	}
	if(enabledFeature && option_table_name != NULL )
	{
		option_table_name_length = libcstring_system_string_length(
		                            option_table_name );
	}
	libsystem_notify_set_verbose( verbose );
	libesedb_notify_set_stream( stderr, NULL );
	libesedb_notify_set_verbose( verbose );

	if (log_filename != NULL)
	{
		if(log_handle_initialize( &log_handle, &error ) != 1 )
		{
			fprintf( stderr, "Unable to initialize log handle.\n" );
			goto on_error;
		}
	}

	if( export_handle_initialize(&esedbexport_export_handle, &error ) != 1 )
	{
		fprintf(stderr, "Unable to initialize export handle.\n" );
		goto on_error;
	}
	
/* TODO
	esedbexport_export_handle->print_status_information = print_status_information;
*/
	if(enabledFeature && option_export_mode != NULL )
	{
		result = export_handle_set_export_mode(
			  esedbexport_export_handle,
			  option_export_mode,
			  &error );

		if( result == -1 )
		{
			fprintf( stderr, "Unable to set export mode.\n" );
			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf( stderr, "Unsupported export mode defaulting to: tables.\n" );
		}
	}
	if(enabledFeature && option_ascii_codepage != NULL )
	{
		result = export_handle_set_ascii_codepage(
		          esedbexport_export_handle,
		          option_ascii_codepage,
		          &error );

		if( result == -1 )
		{
			fprintf( stderr, "Unable to set ASCII codepage in export handle.\n" );
			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf( stderr, "Unsupported ASCII codepage defaulting to: windows-1252.\n" );
		}
	}
	if(enabledFeature && export_handle_set_target_path(
	     esedbexport_export_handle,
	     option_target_path,
	     &error ) != 1 )
	{
		fprintf( stderr, "Unable to set target path.\n" );
		goto on_error;
	}
	if (enabledFeature) {
		result = export_handle_create_items_export_path(
			esedbexport_export_handle,
			&error );

		if( result == -1 )
		{
			fprintf( stderr, "Unable to create items export path.\n" );
			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf( stderr, "%" PRIs_LIBCSTRING_SYSTEM " already exists.\n",
				esedbexport_export_handle->items_export_path );
			goto on_error;
		}
	}

	if (log_filename != NULL)
	{
		if(log_handle_open( log_handle, log_filename, &error ) != 1 )
		{
			fprintf(
				stderr,
				"Unable to open log file: %" PRIs_LIBCSTRING_SYSTEM ".\n",
				log_filename );

			goto on_error;
		}
	}
	fprintf(stdout, "Opening file.\n" );

#ifdef TODO_SIGNAL_ABORT
	if( libsystem_signal_attach(
	     esedbexport_signal_handler,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to attach signal handler.\n" );

		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
#endif
	if( export_handle_open(
	     esedbexport_export_handle,
	     source,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to open: %" PRIs_LIBCSTRING_SYSTEM ".\n",
		 source );

		goto on_error;
	}
	
/* TODO
	fprintf(
	 stdout,
	 "Exporting aliases.\n" );
*/
	if (0) 
	{
#ifdef _EDB_DEBUG_
		MailboxArrayDesc *mailbox_collection = NULL;
		FolderArrayDesc *folder_collection = NULL;
		HashMap *att_ref_map = hashmap_create (1000000);
		HashMap *msg_ref_map = hashmap_create (1000000);
		SignalHandlerPointer previousHandler = signal (SIGABRT, abortHandler);
		edb_format_version_t edb_detected_version;
		time_t time_start, time_end; 
		
		libesedb_table_t *table = NULL;
		libesedb_record_t *item_record = NULL;
		EDBColumnDescriptor *column = NULL;
		uint8_t *value_data = NULL;
		size_t value_data_size = 0;

		/*
		init_edb_tables_map(esedbexport_export_handle, &error);
		
		fetch_edb_record (421, 0x7d, 2, &item_record, &column, &table);

		print_edb_tables_schema();
		close_edb_tables_map ();

		if (1) return 1;
		*/
		// Data tables. 
		clear_file("msg.txt");					reset_counter_file("msg.txt.cnt", 0);
		clear_file("msg-body.txt");				reset_counter_file("msg-body.txt.cnt", 0);
		clear_file("atts.txt");					reset_counter_file("atts.txt.cnt", 0);
		clear_file("att-blobs.txt");			reset_counter_file("att-blobs.txt.cnt", 0);
		clear_file("mailboxes.txt");			reset_counter_file("mailboxes.txt.cnt", 0);
		clear_file("folders.txt");				reset_counter_file("folders.txt.cnt", 0);
		clear_file("msg-metadata-merged.txt");	reset_counter_file("msg-metadata-merged.txt.cnt", 0);
		clear_file("msg-merged.txt");			reset_counter_file("msg-merged.txt.cnt", 0);
		// Association tables. 
		clear_file("msg-att-xref.txt");		reset_counter_file("msg-att-xref.txt.cnt", 0);
		clear_file("folder-msg-xref.txt");	reset_counter_file("folder-msg-xref.txt.cnt", 0);
		
		if (detect_edb_version(esedbexport_export_handle, &error, &edb_detected_version) != 1)
		{
			printf("Unable to detect EDB format version.\n");
			return ( -1 );
		}

		result = export_edb_database(
	          esedbexport_export_handle,
	          option_table_name,
	          option_table_name_length,
	          log_handle,
	          &error );

		if( result == -1 )
		{
			fprintf(stderr, "Unable to export file.\n" );
			goto on_error;
		}
 
		printf("Loading mailboxes...\n");
		load_mailboxes(&mailbox_collection, "mailboxes.txt");
		
		printf("Loading folders...\n");
		load_folders(&folder_collection, "folders.txt");

		printf("Linking folders with mailboxes...\n");
		link_folders_with_mailboxes(mailbox_collection, folder_collection);

		printf("Linking folders and subfolders...\n");
		link_folders_and_subfolders(folder_collection);

		printf("Calculating attachment reference count...\n");
		calculate_attachment_reference_count ("msg-att-xref.txt", att_ref_map);
		hashmap_filter_values (att_ref_map, _is_shared_object);

		printf("Merging attachment data:\n");
		time_start = time(NULL);
		merge_attachment_data("msg-att-xref.txt", "atts.txt", "msg-att-xref-merged.txt", att_ref_map);
		time_end = time(NULL);
		printf ("Done in %ds.\n", (time_end-time_start));

		hashmap_destroy (&att_ref_map, TRUE, NULL);

		printf("Merging message metadata:\n");
		time_start = time(NULL);
		merge_message_headers_and_bodies ("msg.txt", "msg-body.txt", "msg-metadata-merged.txt");
		time_end = time(NULL);
		printf ("Done in %ds.\n", (time_end-time_start));

		printf("Merging messages and attachments:\n");
		time_start = time(NULL);
		merge_messages_with_attachments ("msg-metadata-merged.txt", "msg-att-xref-merged.txt", "msg-merged.txt");
		time_end = time(NULL);
		printf ("Done in %ds.\n", (time_end-time_start));

		printf("Scanning messages (searching for duplicates):\n");
		time_start = time(NULL);
		calculate_message_reference_count ("folder-msg-xref.txt", msg_ref_map);
		hashmap_filter_values (msg_ref_map, _is_shared_object);
		time_end = time(NULL);
		printf ("Done in %ds.\n", (time_end-time_start));
		
		init_attachment_blob_file ();
		printf("Exporting complete messages:\n");
		time_start = time(NULL);
		export_complete_messages_to_folders (folder_collection, "msg-merged.txt", "folder-msg-xref.txt", "edb-output.txt", msg_ref_map, edb_detected_version);
		time_end = time(NULL);
		printf ("Done in %ds.\n", (time_end-time_start));
		close_attachment_blob_file();

		hashmap_destroy (&msg_ref_map, TRUE, NULL);
		free (folder_collection->arrPtr);
		free (folder_collection);
		free (mailbox_collection->arrPtr);
		free (mailbox_collection);

		// restore signal handler.
		signal (SIGABRT, previousHandler);

		remove ("mailboxes.txt");			remove ("mailboxes.txt.cnt");
		remove ("folders.txt");				remove ("folders.txt.cnt");
		remove ("msg.txt");					remove ("msg.txt.cnt");
		remove ("msg-body.txt");			remove ("msg-body.txt.cnt");
		remove ("atts.txt");				remove ("atts.txt.cnt");
		remove ("att-blobs.txt");			remove ("att-blobs.txt.cnt");
		remove ("folder-msg-xref.txt");		remove ("folder-msg-xref.txt.cnt");
		remove ("msg-att-xref-merged.txt");	remove ("msg-att-xref-merged.txt.cnt");
		remove ("msg-att-xref.txt");		remove ("msg-att-xref.txt.cnt");
		remove ("msg-metadata-merged.txt");	remove ("msg-metadata-merged.txt.cnt");
		remove ("msg-merged.txt");			remove ("msg-merged.txt.cnt");
		remove ("folder-msg-xref.txt");		remove ("folder-msg-xref.txt.cnt");
#endif

	}
	else
	result = export_handle_export_file(
	          esedbexport_export_handle,
	          option_table_name,
	          option_table_name_length,
	          log_handle,
	          &error );
	
	if( result == -1 )
	{
		fprintf(
		 stderr,
		 "Unable to export file.\n" );

		goto on_error;
	}
#ifdef TODO_SIGNAL_ABORT
	if( libsystem_signal_detach(
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to detach signal handler.\n" );

		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
#endif
	

	if( export_handle_close(
	     esedbexport_export_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close export handle.\n" );

		goto on_error;
	}
	if( export_handle_free(
	     &esedbexport_export_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free export handle.\n" );

		goto on_error;
	}
	if (log_handle != NULL)
	{
		if( log_handle_close(
			log_handle,
			&error ) != 0 )
		{
			fprintf(
				stderr,
				"Unable to close log handle.\n" );

			goto on_error;
		}
		if( log_handle_free(
			&log_handle,
			&error ) != 1 )
		{
			fprintf(
				stderr,
				"Unable to free log handle.\n" );

			goto on_error;
		}
	}

	if( esedbexport_abort != 0 )
	{
		fprintf( stdout, "Export aborted.\n" );

		return( EXIT_FAILURE );
	}
/* TODO export FAILED ? */
	fprintf( stdout, "Export completed.\n" );

	return ( EXIT_SUCCESS );

on_error:
	if( error != NULL )
	{
		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
	if( esedbexport_export_handle != NULL )
	{
		export_handle_free(
		 &esedbexport_export_handle,
		 NULL );
	}
	if( log_handle != NULL )
	{
		log_handle_free(
		 &log_handle,
		 NULL );
	}
	return( EXIT_FAILURE );
}

