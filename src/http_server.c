//
// Copyright 2025 voidtools / David Carpenter
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// TODO:
// add support for the date created column.
// a simple path rewrite to hide roots, for example: c:\private\media => /media
// add support for date created column (mostly for json support)
// keep case and other search options in result URLs.
// announce self -ideally over SSDP
// json should return version info.
// an option to limit the number of results for a 'index of' page.
// an ini option to set which properties to supported. (multiple users want date created and sha256) -or just make all indexed properties available. -will need an option to specify which columns are shown by default

#define HTTP_SERVER_WM_LISTEN			(WM_USER)
#define HTTP_SERVER_WM_CLIENT			(WM_USER+1)

#define HTTP_SERVER_RECV_CHUNK_SIZE		65536
#define HTTP_SERVER_SEND_CHUNK_SIZE		65536
#define HTTP_SERVER_DATA_CHUNK_SIZE		65536

#define HTTP_SERVER_SORT_NAME			0
#define HTTP_SERVER_SORT_PATH			1
#define HTTP_SERVER_SORT_SIZE			2
#define HTTP_SERVER_SORT_DATE_MODIFIED	3

#define HTTP_SERVER_SORT_ITEM_FILENAME(sort_item)	((everything_plugin_utf8_t *)(((http_server_sort_item_t *)(sort_item)) + 1))
#define HTTP_SERVER_RECV_CHUNK_DATA(recv_chunk)		((everything_plugin_utf8_t *)(((http_server_recv_chunk_t *)(recv_chunk)) + 1))

#define HTTP_SERVER_DEFAULT_PORT				80
#define HTTP_SERVER_DEFAULT_ITEMS_PER_PAGE		32

// plugin ids
enum
{
	HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX,
	HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX,
	HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX,
	HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,
	HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,
	HTTP_SERVER_PLUGIN_ID_HOME_STATIC,
	HTTP_SERVER_PLUGIN_ID_HOME_EDIT,
	HTTP_SERVER_PLUGIN_ID_HOME_BROWSE_BUTTON,
	HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_STATIC,
	HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,
	HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_BROWSE_BUTTON,
	HTTP_SERVER_PLUGIN_ID_PORT_STATIC,
	HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX,
	HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX,
	HTTP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,
	HTTP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,
	HTTP_SERVER_PLUGIN_ID_KB_STATIC,
	HTTP_SERVER_PLUGIN_ID_USERNAME_STATIC,
	HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX,
	HTTP_SERVER_PLUGIN_ID_PASSWORD_STATIC,
	HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX,
	HTTP_SERVER_PLUGIN_ID_BINDINGS_STATIC,
	HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX,
	HTTP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS,
};

#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "..\..\include\everything_plugin.h"
#include "version.h"

#define HTTP_SERVER_STRING_MACRO(name,lowercase_ascii_name,string) HTTP_SERVER_STRING_##name,
enum
{
	#include "http_server_string.h"
};
#undef HTTP_SERVER_STRING_MACRO

#define HTTP_SERVER_STRING_MACRO(name,lowercase_ascii_name,string) string,
static const everything_plugin_utf8_t *http_server_strings[] =
{
	#include "http_server_string.h"
};
#undef HTTP_SERVER_STRING_MACRO

#define HTTP_SERVER_STRING_MACRO(name,lowercase_ascii_name,string) lowercase_ascii_name,
static const everything_plugin_utf8_t *http_server_string_names[] =
{
	#include "http_server_string.h"
};
#undef HTTP_SERVER_STRING_MACRO

#define HTTP_SERVER_STRING_COUNT (sizeof(http_server_strings) / sizeof(const everything_plugin_utf8_t *))

// a 64k struct recv buffer chunk		
typedef struct http_server_recv_chunk_s
{
	struct http_server_recv_chunk_s *next;

#pragma pack (push,1)

	// data follows.	
	
}http_server_recv_chunk_t;

#pragma pack (pop)

// a 64k struct recv buffer chunk		
typedef struct http_server_send_chunk_s
{
	struct http_server_send_chunk_s *next;
	uintptr_t size;
	everything_plugin_utf8_t *data;
	
}http_server_send_chunk_t;

typedef struct http_server_client_s
{
	// client data
	EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle;

	struct http_server_client_s *next;
	struct http_server_client_s *prev;
	struct http_server_client_s *query_next;
	struct http_server_client_s *query_prev;
	int got_authentication;
	EVERYTHING_PLUGIN_QWORD content_range_start;
	EVERYTHING_PLUGIN_QWORD content_range_size;
	
	// recv packet buffers
	http_server_recv_chunk_t *recv_chunk_start;
	http_server_recv_chunk_t *recv_chunk_last;
	everything_plugin_utf8_t *recv_front;
	everything_plugin_utf8_t *recv_end;
	uintptr_t recv_chunk_count;
	
	// full send packets.
	http_server_send_chunk_t *send_chunk_start;
	http_server_send_chunk_t *send_chunk_last;
	
	// send packets
	everything_plugin_utf8_t *send_buffer;
	
	// send buffer positions.
	everything_plugin_utf8_t *send_front;
	everything_plugin_utf8_t *send_end;
	
	// remaining data send_chunk_start to send.
	uintptr_t send_remaining;
	int send_shutdown;
	
	// get request
	everything_plugin_utf8_t *get;

	// query info.
	// since this doesnt occur straight away
	// we get a completion event once the query has completed, then we 
	// send the results.
	// this is a copy of the search parameters.
	int json;
	DWORD offset;
	DWORD show_max;

	// file
	everything_plugin_utf8_t *data_buffer;
	uintptr_t data_size; // number of bytes saved into buffer.
	HANDLE data_file;
	uintptr_t data_remaining;
	everything_plugin_os_thread_t *data_thread;  // NULL if we got EOF.
	HANDLE data_hevent;
	CRITICAL_SECTION data_cs;
	int data_abort;
	int data_state; // 0 = ok, 1 = EOF, 2 read error.
	int data_complete;
	
	// current query state.
	everything_plugin_utf8_t *search_string;
	int sort;
	int sort_ascending;
	int match_case;
	int match_whole_word;
	int match_path;
	int match_diacritics;
	int match_prefix;
	int match_suffix;
	int ignore_punctuation;
	int ignore_whitespace;
	int match_regex;
	const everything_plugin_property_t *db_sort_column_type;
	int db_sort_ascending;
	int is_query; // are we in the query list?
	int got_header;
	
	int path_column;
	int size_column;
	int date_modified_column;
		
}http_server_client_t;

// types
typedef struct http_server_listen_s
{
	struct http_server_listen_s *next;
	EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET listen_socket;

}http_server_listen_t;

// types
typedef struct http_server_s
{
	http_server_client_t *client_start;
	http_server_client_t *client_last;

	HWND hwnd;

	http_server_listen_t *listen_start;
	http_server_listen_t *listen_last;
	
	// the port we are listening on
	// we can check if this is changed with http_server_port;
	int port;
	
	// list of interfaces to bind to.
	everything_plugin_utf8_t *bindings;

	everything_plugin_db_t *db;
	everything_plugin_db_query_t *q;
	
	// current query state.
	everything_plugin_utf8_t *current_search_string;
	const everything_plugin_property_t *current_db_sort_column_type;
	int current_db_sort_ascending;
	int current_match_case;
	int current_match_whole_word;
	int current_match_path;
	int current_match_diacritics;
	int current_match_prefix;
	int current_match_suffix;
	int current_ignore_punctuation;
	int current_ignore_whitespace;
	int current_match_regex;
	int is_current_query;
	
	http_server_client_t *client_query_active;
	http_server_client_t *client_query_start;
	http_server_client_t *client_query_last;
	const everything_plugin_utf8_t *strings[HTTP_SERVER_STRING_COUNT];
	
	everything_plugin_output_stream_t *log_file;
	
}http_server_t;

// an internal http server resource.
typedef struct http_server_resource_s
{
	const everything_plugin_utf8_t *filename;
	const everything_plugin_utf8_t *content_type;
	const void *data;
	uintptr_t size;
	EVERYTHING_PLUGIN_QWORD date_modified;
	
}http_server_resource_t;

typedef struct http_server_sort_item_s
{
	everything_plugin_fileinfo_fd_t fd;
	uintptr_t len;

#pragma pack (push,1)

	BYTE is_folder;
	
	// filename follows.
	
}http_server_sort_item_t;

#pragma pack (pop)

// statics.
static int http_server_is_valid_url_char(everything_plugin_utf8_t c,int escape_forward_slashes);
static everything_plugin_utf8_t *http_server_get_escape_url(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *str,int escape_forward_slashes);
static DWORD WINAPI _send_db_file_thread_proc(void *lpParameter);
static void http_server_escape_url(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *str,int escape_forward_slashes);
static void http_server_escape_url_filename(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *filename);
static void http_server_send_escape_html(http_server_client_t *c,const everything_plugin_utf8_t *str);
static void http_server_send_size(http_server_client_t *c,EVERYTHING_PLUGIN_QWORD size);
static void http_server_send_date(http_server_client_t *c,EVERYTHING_PLUGIN_QWORD date);
static http_server_client_t *http_server_client_create(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET client_socket);
static void http_server_client_destroy(http_server_client_t *c);
static int http_server_check_authorization(const everything_plugin_utf8_t *base64);
static void http_server_search(http_server_client_t *c);
static int http_server_send_resource(http_server_client_t *c,const everything_plugin_utf8_t *filename);
static int http_server_send_file2(http_server_client_t *c,const everything_plugin_utf8_t *filename,int iscache);
static int http_server_send_file(http_server_client_t *c,const everything_plugin_utf8_t *filename);
static int http_server_send_path(http_server_client_t *c,const everything_plugin_utf8_t *filename);
static void http_server_error(http_server_client_t *c,int code,const everything_plugin_utf8_t *desc);
static void http_server_client_send_head(http_server_client_t *c,const everything_plugin_utf8_t *title);
static void http_server_html_td(http_server_client_t *c,int alternate,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *name,int is_folder,everything_plugin_fileinfo_fd_t *fd,int show_path);
static void http_server_html_foot(http_server_client_t *c);
static uintptr_t http_server_add_page(uintptr_t *pagelist,uintptr_t numpages,uintptr_t pagei);
static void http_server_log(http_server_client_t *c,const everything_plugin_utf8_t *format,...);
static void http_server_file_header(http_server_client_t *c,const everything_plugin_utf8_t *content_type,EVERYTHING_PLUGIN_QWORD size,EVERYTHING_PLUGIN_QWORD date_modified,int cache,int allow_range,EVERYTHING_PLUGIN_QWORD content_range_start,EVERYTHING_PLUGIN_QWORD content_range_count);
static void http_server_client_send_header(http_server_client_t *c);
static void http_server_select(void);
static void http_server_unescapeurl(everything_plugin_utf8_buf_t *cbuf);
static void EVERYTHING_PLUGIN_API http_server_db_query_event_proc(void *user_data,int type);
static int http_server_get_bind_addrinfo(const everything_plugin_utf8_t *nodename,struct everything_plugin_os_winsock_addrinfo **ai);
static LRESULT WINAPI http_server_window_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static void http_server_recv_chunk_add(http_server_client_t *c);
static void http_server_client_process_command(http_server_client_t *c,everything_plugin_utf8_t *command);
static void http_server_recv_chunk_destroy(http_server_recv_chunk_t *recv_chunk);
static void http_server_send_chunk_destroy(http_server_send_chunk_t *send_chunk);
static void http_server_client_table_begin(http_server_client_t *c,const everything_plugin_utf8_t *search_string);
static void http_server_client_table_end(http_server_client_t *c);
static void http_server_client_send_table_header(http_server_client_t *c,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *search,int is_current_sort,int ascending,const everything_plugin_utf8_t *classname,const everything_plugin_utf8_t *title,const everything_plugin_utf8_t *sortname,int default_ascending);
static void http_server_client_send_table_headers(http_server_client_t *c,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *search,int sort,int ascending);
static void http_server_client_send_search_link(http_server_client_t *c,const everything_plugin_utf8_t *search_string,int sort,int ascending,uintptr_t offset,DWORD show_max);
static everything_plugin_utf8_t *http_server_escape_json(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *s);
static void http_server_send_escaped_json(http_server_client_t *c,const everything_plugin_utf8_t *text);
static void http_server_format_time(everything_plugin_utf8_buf_t *cbuf,SYSTEMTIME *st);
static http_server_client_t *http_server_client_find(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static int http_server_client_update_recv(http_server_client_t *c);
static int http_server_client_update_send(http_server_client_t *c);
static void http_server_client_flush(http_server_client_t *c);
static void http_server_client_send_shutdown(http_server_client_t *c);
static void http_server_client_send_add(http_server_client_t *c,const void *data,uintptr_t size);
static void http_server_client_printf(http_server_client_t *c,const everything_plugin_utf8_t *format,...);
static void http_server_client_send_string_id(http_server_client_t *c,DWORD string_id);
static void http_server_client_send_utf8(http_server_client_t *c,const everything_plugin_utf8_t *text);
static int http_server_compare_folder(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int WINAPI http_server_compare_name_ascending(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int WINAPI http_server_compare_name_descending(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int WINAPI http_server_compare_size_ascending(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int WINAPI http_server_compare_size_descending(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int WINAPI http_server_compare_date_ascending(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int WINAPI http_server_compare_date_descending(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
static int http_server_add_binding(everything_plugin_utf8_buf_t *error_cbuf,const everything_plugin_utf8_t *nodename);
static DWORD WINAPI http_server_send_file_thread_proc(http_server_client_t *c);
static int http_server_is_config_change(void);
static void http_server_send_query_results(http_server_client_t *c);
static void http_server_remove_query(http_server_client_t *c);
static void http_server_insert_query(http_server_client_t *c);
static void http_server_start_next_query(void);
static const everything_plugin_utf8_t *http_server_get_sort_name(int sort);
static void http_server_shutdown(void);
static void http_server_start(void);
static int http_server_apply_settings(void);
static void http_server_update_options_page(HWND page_hwnd);
static void http_server_create_checkbox(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,int checked);
static void http_server_create_static(everything_plugin_load_options_page_t *load_options_page,int id,int text_localization_id);
static void http_server_create_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,const everything_plugin_utf8_t *text);
static void http_server_create_number_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,int value);
static void http_server_create_password_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,const everything_plugin_utf8_t *text);
static void http_server_create_button(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id);
static void http_server_enable_options_apply(everything_plugin_options_page_proc_t *options_page_proc);
static int http_server_expand_min_wide(HWND page_hwnd,int text_localization_id,int current_wide);
static everything_plugin_utf8_t *http_server_get_options_text(HWND page_hwnd,int id,everything_plugin_utf8_t *old_value);
static int http_server_is_valid_header(const everything_plugin_utf8_t *s);
static void http_server_client_send_header_list(http_server_client_t *c,const everything_plugin_utf8_t *list);
static void http_server_send_sort_url_param(http_server_client_t *c);
static const everything_plugin_property_t *http_server_get_property_from_sort(int sort);

// static vars
static http_server_t *_http_server = 0;

// HTTP server
static int http_server_enabled = 0;
static everything_plugin_utf8_t *http_server_title_format = NULL;
static int http_server_port = HTTP_SERVER_DEFAULT_PORT;
static everything_plugin_utf8_t *http_server_username = NULL;
static everything_plugin_utf8_t *http_server_password = NULL;
static everything_plugin_utf8_t *http_server_home = NULL;
static everything_plugin_utf8_t *http_server_default_page = NULL;
static everything_plugin_utf8_t *http_server_log_file_name = NULL;
static int http_server_logging_enabled = 1;
static int http_server_log_max_size = 4 * 1024 * 1024;
static int http_server_log_delta_size = 512 * 1024;
static int http_server_allow_file_download = 1;
static everything_plugin_utf8_t *http_server_bindings = NULL;
static int http_server_items_per_page = HTTP_SERVER_DEFAULT_ITEMS_PER_PAGE;
static int http_server_show_drive_labels;
static everything_plugin_utf8_t *http_server_strings_filename = NULL;
static everything_plugin_utf8_t *http_server_header = NULL;
static int http_server_allow_query_access = 0; // is-open: online: runcount:
static int http_server_allow_disk_access = 0; // content: and include-filelist:
static int http_server_default_sort = HTTP_SERVER_SORT_DATE_MODIFIED;
static int http_server_default_sort_ascending = 0;

static const char http_server_main_css[] = 
	"* {\r\n"
	"	padding: 0px;\r\n"
	"	margin: 0px;\r\n"
	"}\r\n"
	"\r\n"
	"body {\r\n"
	"	padding: 4px;\r\n"
	"}\r\n"
	"\r\n"
	"body, td, div {\r\n"
	"	font-family: Arial, sans-serif;\r\n"
	"	font-size: 12px;\r\n"
	"}\r\n"
	"\r\n"
	"td, tr, img {\r\n"
	"	border-width: 0px;\r\n"
	"	vertical-align:middle;\r\n"
	"}\r\n"
	"\r\n"
	"a {\r\n"
	"	text-decoration: none;\r\n"
	"}\r\n"
	"\r\n"
	".indexof {\r\n"
	"	text-align: left;\r\n"
	"	font-weight: bold;\r\n"
	"	font-size: 125%;\r\n"
	"	margin-top: 32px;\r\n"
	"}\r\n"
	"\r\n"
	".numresults {\r\n"
	"	text-align: left;\r\n"
	"	font-weight: bold;\r\n"
	"	font-size: 125%;\r\n"
	"	margin-top: 32px;\r\n"
	"}\r\n"
	"\r\n"
	".updir {\r\n"
	"	text-align: left;\r\n"
	"	padding-top: 4px;\r\n"
	"}\r\n"
	"\r\n"
	".nameheader, .folder, .file {\r\n"
	"	text-align: left;\r\n"
	"	padding: 2px 15px 2px 2px;\r\n"
	"	width: 100%;\r\n"
	"}\r\n"
	"\r\n"
	".pathheader, .pathdata {\r\n"
	"	text-align: left;\r\n"
	"	padding: 2px 15px 2px 2px;\r\n"
	"	width: 0px;\r\n"
	"}\r\n"
	"\r\n"
	".sizeheader, .sizedata {\r\n"
	"	padding: 2px 15px 2px 2px;\r\n"
	"	text-align: right;\r\n"
	"	width: 0px;\r\n"
	"}\r\n"
	"\r\n"
	".modifiedheader, .modifieddata {\r\n"
	"	text-align: left;\r\n"
	"	padding: 2px;\r\n"
	"	width: 0px;\r\n"
	"}\r\n"
	"\r\n"
	".sizeheader, .modifiedheader, .nameheader, .pathheader {\r\n"
	"	font-weight: bold;\r\n"
	"	padding-top: 32px;\r\n"
	"	padding-bottom: 2px;\r\n"
	"}\r\n"
	"\r\n"
	".sizeheader a, .modifiedheader a, .nameheader a , .pathheader a {\r\n"
	"	color:#000;\r\n"
	"}\r\n"
	"\r\n"
	"tr.trdata1:hover, tr.trdata2:hover {\r\n"
	"	background-color: #eee;\r\n"
	"}\r\n"
	"\r\n"
	".icon {\r\n"
	"	vertical-align:middle;\r\n"
	"	margin-right: 4px;\r\n"
	"}\r\n"
	"\r\n"
	".updown {\r\n"
	"	margin-left: 4px;\r\n"
	"	vertical-align: middle;\r\n"
	"}\r\n"
	"\r\n"
	".lineshadow {\r\n"
	"	background-color: #ccc;\r\n"
	"}\r\n"
	"\r\n"
	".logo {\r\n"
	"	color:#666;\r\n"
	"	font-size:64px;\r\n"
	"}\r\n"
	"\r\n"
	".searchbox {\r\n"
	"	font-size: 150%;\r\n"
	"	padding:2px;\r\n"
	"}\r\n"
	"\r\n"
	".nobr {\r\n"
	"	white-space: nowrap;\r\n"
	"}\r\n"
	"\r\n"
	".nav {\r\n"
	"	padding-left: 4px;\r\n"
	"	padding-right: 4px;\r\n"
	"}\r\n"
	"\r\n"
	".prevnext {\r\n"
	"	font-weight: bold;\r\n"
	"	padding-left: 8px;\r\n"
	"	padding-right: 8px;\r\n"
	"}\r\n";

static const BYTE http_server_favicon_ico[] = 
{
	// size: 1150
	0x00,0x00,0x01,0x00,0x01,0x00,0x10,0x10,0x00,0x00,0x01,0x00,0x20,0x00,0x68,0x04,
	0x00,0x00,0x16,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x20,0x00,
	0x00,0x00,0x01,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x40,0x04,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x80,0x4c,0x00,0x00,0x8f,0xff,0x00,0x00,0x85,0xf3,0x00,0x00,0x80,0x4e,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x01,
	0x8e,0xfe,0x00,0x0b,0xfc,0xff,0x00,0x0a,0xe9,0xff,0x00,0x01,0x8b,0xf9,0x00,0x00,
	0x80,0x4f,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x85,0xf3,0x00,0x0f,0xe8,0xff,0x00,0x13,0xff,0xff,0x00,0x10,0xe9,0xff,0x00,0x01,
	0x8a,0xf9,0x00,0x00,0x80,0x4f,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x4c,0x00,0x02,0x8a,0xf8,0x00,0x16,0xe8,0xff,0x00,0x1b,0xff,0xff,0x00,0x17,
	0xe9,0xff,0x00,0x02,0x8a,0xf8,0x00,0x00,0x80,0x6a,0x00,0x00,0x80,0x97,0x00,0x00,
	0x80,0xd8,0x00,0x00,0x80,0xef,0x00,0x00,0x80,0xe0,0x00,0x00,0x80,0xa8,0x00,0x00,
	0x80,0x43,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x4c,0x00,0x02,0x8a,0xf8,0x00,0x1d,0xe8,0xff,0x00,0x23,
	0xff,0xff,0x00,0x1e,0xe9,0xff,0x00,0x0a,0xa0,0xff,0x00,0x14,0xc6,0xff,0x00,0x1f,
	0xed,0xff,0x00,0x23,0xfb,0xff,0x00,0x20,0xf1,0xff,0x00,0x17,0xd0,0xff,0x00,0x06,
	0x96,0xff,0x00,0x00,0x80,0xa5,0x00,0x00,0x80,0x08,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x4c,0x00,0x03,0x8a,0xf8,0x00,0x23,
	0xe8,0xff,0x00,0x2b,0xff,0xff,0x00,0x2b,0xff,0xff,0x00,0x2b,0xff,0xff,0x00,0x2b,
	0xff,0xff,0x00,0x2b,0xff,0xff,0x00,0x2b,0xff,0xff,0x00,0x2b,0xff,0xff,0x00,0x2b,
	0xfc,0xff,0x00,0x11,0xb0,0xff,0x00,0x00,0x80,0xaf,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x68,0x00,0x0d,
	0xa0,0xff,0x00,0x33,0xff,0xff,0x00,0x33,0xff,0xff,0x00,0x2b,0xea,0xff,0x00,0x11,
	0xac,0xff,0x00,0x08,0x95,0xff,0x00,0x0e,0xa4,0xff,0x00,0x25,0xde,0xff,0x00,0x33,
	0xff,0xff,0x00,0x33,0xfd,0xff,0x00,0x0c,0x9d,0xff,0x00,0x00,0x80,0x5b,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x97,0x00,0x21,
	0xc6,0xff,0x00,0x3b,0xff,0xff,0x00,0x32,0xea,0xff,0x00,0x03,0x87,0xf3,0x00,0x00,
	0x80,0x66,0x00,0x00,0x80,0x23,0x00,0x00,0x80,0x4e,0x00,0x00,0x81,0xe0,0x00,0x28,
	0xd7,0xff,0x00,0x3b,0xff,0xff,0x00,0x2d,0xe0,0xff,0x00,0x00,0x80,0xca,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0xd7,0x00,0x3a,
	0xec,0xff,0x00,0x43,0xff,0xff,0x00,0x17,0xac,0xff,0x00,0x00,0x80,0x66,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x32,0x00,0x0a,
	0x93,0xfe,0x00,0x43,0xff,0xff,0x00,0x43,0xfe,0xff,0x00,0x04,0x88,0xfd,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0xee,0x00,0x49,
	0xfb,0xff,0x00,0x4b,0xff,0xff,0x00,0x0d,0x95,0xff,0x00,0x00,0x80,0x23,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0xef,0x00,0x49,0xfb,0xff,0x00,0x4b,0xff,0xff,0x00,0x0d,0x95,0xff,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0xdf,0x00,0x4b,
	0xf1,0xff,0x00,0x53,0xff,0xff,0x00,0x18,0xa5,0xff,0x00,0x00,0x80,0x4f,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x1d,0x00,0x07,
	0x8b,0xfd,0x00,0x53,0xfe,0xff,0x00,0x53,0xff,0xff,0x00,0x07,0x8c,0xff,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0xa7,0x00,0x39,
	0xd0,0xff,0x00,0x5b,0xff,0xff,0x00,0x44,0xde,0xff,0x00,0x00,0x81,0xe0,0x00,0x00,
	0x80,0x33,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x1d,0x00,0x00,0x80,0xc3,0x00,0x33,
	0xc6,0xff,0x00,0x5b,0xff,0xff,0x00,0x4c,0xe9,0xff,0x00,0x00,0x80,0xdb,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x42,0x00,0x10,
	0x95,0xfe,0x00,0x61,0xfc,0xff,0x00,0x63,0xff,0xff,0x00,0x45,0xd7,0xff,0x00,0x0f,
	0x93,0xfe,0x00,0x00,0x80,0xef,0x00,0x09,0x8b,0xfd,0x00,0x38,0xc6,0xff,0x00,0x63,
	0xfe,0xff,0x00,0x63,0xff,0xff,0x00,0x22,0xac,0xff,0x00,0x00,0x80,0x75,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0xa4,0x00,0x28,0xaf,0xff,0x00,0x6a,0xfd,0xff,0x00,0x6b,0xff,0xff,0x00,0x6b,
	0xff,0xff,0x00,0x69,0xfb,0xff,0x00,0x6b,0xfe,0xff,0x00,0x6b,0xff,0xff,0x00,0x6b,
	0xff,0xff,0x00,0x3c,0xc7,0xff,0x00,0x00,0x80,0xd0,0x00,0x00,0x80,0x07,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x08,0x00,0x00,0x80,0xae,0x00,0x1a,0x9d,0xff,0x00,0x56,0xdf,0xff,0x00,0x73,
	0xfe,0xff,0x00,0x73,0xff,0xff,0x00,0x73,0xff,0xff,0x00,0x60,0xe9,0xff,0x00,0x27,
	0xac,0xff,0x00,0x00,0x80,0xd0,0x00,0x00,0x80,0x1a,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x5a,0x00,0x00,0x80,0xca,0x00,0x07,
	0x88,0xfd,0x00,0x14,0x95,0xff,0x00,0x0b,0x8c,0xfe,0x00,0x00,0x80,0xda,0x00,0x00,
	0x80,0x75,0x00,0x00,0x80,0x07,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x0f,0xff,
	0x00,0x00,0x07,0xff,0x00,0x00,0x03,0xff,0x00,0x00,0x00,0x07,0x00,0x00,0x80,0x01,
	0x00,0x00,0xc0,0x01,0x00,0x00,0xe0,0x00,0x00,0x00,0xe0,0x00,0x00,0x00,0xe0,0xe0,
	0x00,0x00,0xe0,0xf0,0x00,0x00,0xe0,0xe0,0x00,0x00,0xe0,0x40,0x00,0x00,0xe0,0x00,
	0x00,0x00,0xf0,0x00,0x00,0x00,0xf0,0x01,0x00,0x00,0xfc,0x03,0x00,0x00,
};

static const BYTE http_server_folder_gif[] = 
{
	// size: 125
	0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xb3,0x00,0x00,0x91,0x7b,0x09,
	0xff,0x00,0xff,0xfc,0xce,0x2d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0xf9,0x04,
	0x01,0x00,0x00,0x01,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x04,
	0x2a,0x30,0xc8,0x49,0xab,0x04,0x38,0xdb,0x09,0x84,0xff,0xc0,0x16,0x74,0x1f,0x98,
	0x69,0x64,0x99,0xaa,0x67,0x56,0xb2,0x6f,0x6c,0xca,0x32,0x46,0xc7,0xf6,0x0d,0xeb,
	0x33,0xef,0xe5,0x3e,0x20,0xaf,0x45,0x24,0x8a,0x44,0x11,0x00,0x3b,
};

static const BYTE http_server_file_gif[] = 
{
	// size: 131
	0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xb3,0x00,0x00,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x80,0x00,
	0x80,0x80,0xc0,0xc0,0xc0,0x80,0x80,0x80,0xff,0x00,0x00,0x00,0xff,0x00,0xff,0xff,
	0x00,0x00,0x00,0xff,0xff,0x00,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0x21,0xf9,0x04,
	0x01,0x00,0x00,0x0d,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x04,
	0x30,0xb0,0x35,0x44,0xab,0x95,0x78,0xbe,0xcd,0x37,0xcd,0x5a,0xc7,0x21,0x0f,0x92,
	0x91,0xa2,0x57,0x61,0x68,0x3a,0xb2,0x6e,0x67,0x4a,0x6d,0x3c,0x87,0x71,0x09,0xe7,
	0x3a,0xcd,0xf7,0x38,0xdb,0x2e,0x77,0xab,0xb9,0x8a,0x3f,0x24,0xaf,0x68,0x69,0xae,
	0x22,0x00,0x3b,
};

static const BYTE http_server_everything_gif[] = 
{
	// size: 708
	0x47,0x49,0x46,0x38,0x39,0x61,0xaa,0x00,0x20,0x00,0xb3,0x00,0x00,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x80,0x00,
	0x80,0x80,0xc0,0xc0,0xc0,0x80,0x80,0x80,0xff,0x00,0x00,0x00,0xff,0x00,0xff,0xff,
	0x00,0x00,0x00,0xff,0xff,0x00,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0x2c,0x00,0x00,
	0x00,0x00,0xaa,0x00,0x20,0x00,0x00,0x04,0xfe,0xf0,0xc9,0x49,0xab,0xbd,0x38,0xeb,
	0xcd,0x3b,0x3f,0x20,0xe8,0x3d,0x61,0x38,0x9e,0x5b,0xa9,0xae,0x68,0xeb,0xbe,0x1d,
	0x20,0xcb,0x88,0x37,0xcf,0xf0,0x8b,0xdc,0x7c,0x7f,0xe4,0xc0,0x20,0xe5,0x80,0x28,
	0x22,0x7e,0x16,0x5e,0x2d,0xc6,0x13,0x9e,0x76,0xbd,0x28,0x00,0xe9,0xac,0xb6,0x7a,
	0x17,0xa5,0xad,0x69,0xe5,0x40,0xa5,0x3c,0x6a,0x77,0xbc,0xc1,0x26,0x6f,0x4b,0x8e,
	0x99,0x8c,0xf9,0x82,0x67,0x62,0xb6,0x9c,0xb2,0xa6,0xa3,0xe7,0x55,0x37,0xe0,0xb8,
	0x12,0xe1,0xff,0x12,0x75,0x13,0x5a,0x80,0x40,0x7a,0x71,0x16,0x46,0x45,0x88,0x44,
	0x8a,0x16,0x44,0x4a,0x8c,0x8a,0x54,0x8d,0x8a,0x45,0x17,0x96,0x20,0x7a,0x7b,0x46,
	0x48,0x84,0x6e,0x69,0x13,0x95,0x97,0xa2,0x93,0x12,0x07,0x77,0x19,0xa0,0x9a,0x96,
	0x61,0x65,0x5c,0x76,0x37,0x43,0x6f,0xa1,0x0f,0x58,0xa8,0x33,0x7a,0xb5,0x87,0x6f,
	0x38,0xb6,0x68,0xb8,0x82,0x7a,0xb1,0x70,0x52,0x8c,0xbd,0xb9,0xae,0x1a,0x87,0xb3,
	0xa9,0x24,0xc8,0x00,0xc4,0x34,0x61,0xc2,0x4d,0xd0,0x81,0xd0,0x7b,0x14,0xc3,0x83,
	0xd9,0xd1,0xcd,0xd0,0x9b,0xc6,0xa7,0x84,0x12,0xcc,0xcf,0x4a,0x9b,0x69,0xc8,0x90,
	0xb2,0xd2,0x35,0xd7,0xbf,0xd9,0xdb,0xb0,0xf2,0xd0,0x94,0xde,0x7b,0xd9,0xeb,0xf5,
	0x6b,0xca,0xe6,0xb0,0xd8,0x05,0xa3,0x21,0xca,0x15,0x08,0x1f,0x25,0xb0,0xa5,0xdb,
	0x44,0x85,0x9b,0x42,0x34,0x45,0x7a,0xa4,0x61,0x38,0x41,0xd7,0xbe,0x8a,0xfd,0x52,
	0x0d,0xfc,0x56,0xf0,0x86,0xa7,0x48,0xfe,0xa2,0x1e,0xca,0xc0,0xe8,0x4e,0x24,0xc7,
	0x77,0xf4,0x9c,0x39,0xb4,0x37,0xb2,0x63,0xc9,0x6a,0x68,0x2e,0x76,0xcb,0x05,0xb0,
	0x24,0xa8,0x33,0x70,0x4c,0xb6,0x09,0xe8,0x8c,0xe5,0xc9,0x99,0x04,0x5d,0x06,0x7d,
	0xb0,0x52,0x90,0x99,0x8d,0x71,0x2c,0xde,0x93,0xf6,0xed,0xa6,0xc9,0x5e,0x4b,0x04,
	0xa1,0xdc,0x58,0x41,0x2a,0xcb,0x5a,0xe5,0x8a,0xd6,0xf3,0x59,0xd3,0x57,0xca,0x64,
	0x1e,0x3b,0x50,0x65,0x49,0x25,0x9f,0x36,0xae,0x18,0xb0,0xac,0x44,0x5b,0xb5,0xe7,
	0x55,0x92,0x5e,0xd9,0xea,0x24,0xba,0x95,0x6e,0xcc,0xb0,0x4c,0xe0,0x8c,0x7d,0x4a,
	0x6b,0xee,0x05,0xaa,0xe5,0xfc,0xa2,0x1c,0x6c,0x37,0xae,0x51,0x2e,0x6b,0x0b,0x4f,
	0xc3,0xeb,0x05,0x62,0xcf,0x48,0x7d,0x4c,0x08,0xc6,0xb9,0x38,0xee,0x64,0x91,0x58,
	0x55,0x66,0x2c,0x39,0x37,0xb1,0xd2,0x9c,0x62,0xdf,0x88,0x09,0xac,0xc1,0x2a,0x5c,
	0x89,0x94,0x5b,0xb6,0xa5,0x49,0x58,0x2b,0x67,0xb4,0x48,0x2b,0x7c,0x96,0x81,0x28,
	0xc3,0x1b,0xd9,0xff,0x1e,0x01,0x55,0xfd,0x17,0x0c,0x22,0xab,0xa4,0xb3,0x6e,0xb6,
	0xec,0x6f,0xeb,0x6c,0x4e,0x96,0x8c,0x24,0x92,0x52,0x6b,0xe3,0x94,0x47,0xb4,0x2f,
	0xaf,0x06,0x5e,0xf7,0x6d,0xeb,0xe1,0xbc,0xd9,0x46,0x31,0xb2,0xdd,0x5b,0x1c,0xe7,
	0xd1,0xa7,0x43,0x8c,0x08,0xda,0xf4,0x69,0xd6,0xb8,0xc3,0x34,0xc2,0x2c,0x7e,0x22,
	0xf6,0x9f,0x6b,0xc0,0x83,0x11,0x37,0x4e,0xbc,0xe5,0xdd,0xa2,0xa5,0xdb,0x7f,0x0e,
	0x1d,0x0c,0xfb,0xeb,0x9c,0x1d,0x7b,0xc6,0x19,0x7d,0xdd,0x65,0x83,0x88,0x3a,0xb6,
	0x2d,0x25,0x57,0x5a,0xe6,0xdd,0x66,0xdd,0x6e,0xee,0x05,0xb8,0x15,0x4c,0xe9,0x28,
	0xe1,0x5d,0x7f,0xaf,0x61,0xb8,0x5d,0x59,0xd5,0xa5,0x87,0x1e,0x26,0x52,0xfc,0x07,
	0xe1,0x79,0x4c,0xd9,0x87,0x09,0x14,0x47,0x28,0xf6,0x53,0x21,0x24,0x4c,0x52,0x1b,
	0x0a,0xe6,0x0d,0x61,0x0a,0x8b,0x41,0xc4,0x48,0x63,0x10,0xce,0xdd,0xc8,0x86,0x1f,
	0x15,0x6c,0x54,0x8b,0x8e,0x79,0xe4,0x06,0xa4,0x13,0xb9,0x94,0xb0,0xc9,0x90,0x64,
	0xd8,0x88,0xe4,0x15,0xe1,0x2c,0x69,0x85,0x2e,0x4e,0x56,0x31,0x4f,0x94,0x44,0x0a,
	0x49,0xe5,0x0b,0xc8,0xfc,0x78,0xa5,0x0e,0x96,0x6c,0x29,0xc4,0x28,0x33,0x4e,0x10,
	0x01,0x00,0x00,0x3b,
};

static const BYTE http_server_up_gif[] = 
{
	// size: 819
	0x47,0x49,0x46,0x38,0x39,0x61,0x07,0x00,0x04,0x00,0xf7,0x00,0x00,0x00,0x00,0x00,
	0x84,0x84,0x84,0xc6,0xc6,0xc6,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x21,0xf9,0x04,
	0x01,0x00,0x00,0x03,0x00,0x2c,0x00,0x00,0x00,0x00,0x07,0x00,0x04,0x00,0x00,0x08,
	0x10,0x00,0x07,0x08,0x0c,0x20,0xb0,0x60,0x80,0x83,0x06,0x0f,0x22,0x54,0xc8,0x30,
	0x20,0x00,0x3b,
};

static const BYTE http_server_down_gif[] = 
{
	// size: 822
	0x47,0x49,0x46,0x38,0x39,0x61,0x07,0x00,0x04,0x00,0xf7,0x00,0x00,0x00,0x00,0x00,
	0x84,0x84,0x84,0xc6,0xc6,0xc6,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x21,0xf9,0x04,
	0x01,0x00,0x00,0x03,0x00,0x2c,0x00,0x00,0x00,0x00,0x07,0x00,0x04,0x00,0x00,0x08,
	0x13,0x00,0x03,0x08,0x1c,0x18,0x60,0x00,0xc1,0x82,0x03,0x0c,0x0a,0x4c,0xc8,0x10,
	0xe1,0x80,0x80,0x00,0x00,0x3b,
};

static const BYTE http_server_updir_gif[] = 
{
	// size: 145
	0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xb3,0x00,0x00,0x00,0x80,0x00,
	0x00,0xff,0x00,0x91,0x7b,0x09,0xff,0x00,0xff,0xfc,0xce,0x2d,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0xf9,0x04,
	0x01,0x00,0x00,0x03,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x04,
	0x3e,0x70,0xc8,0x49,0x07,0x00,0x54,0xe8,0x3d,0x41,0x08,0xd8,0x20,0x10,0x64,0x29,
	0x58,0xdf,0x87,0x8d,0x65,0xe9,0xa5,0x2a,0xdb,0x0a,0xd7,0x5b,0x6f,0xb8,0xab,0x12,
	0x5a,0xeb,0xdb,0x3c,0x99,0x8f,0x00,0xec,0x0d,0x49,0x45,0xe1,0xaf,0x16,0x3c,0x1e,
	0x8d,0xce,0x99,0x32,0x0a,0x8d,0x92,0x70,0xd8,0xac,0xa6,0xc2,0xad,0x44,0x00,0x00,
	0x3b,
};

// internal resources lookup table.
static http_server_resource_t http_server_resources[] =
{
	{(const everything_plugin_utf8_t *)"main.css",(const everything_plugin_utf8_t *)"text/css",http_server_main_css,(sizeof(http_server_main_css) / sizeof(BYTE)) - 1,130065084536092528},
	{(const everything_plugin_utf8_t *)"favicon.ico",(const everything_plugin_utf8_t *)"image/x-icon",http_server_favicon_ico,sizeof(http_server_favicon_ico) / sizeof(BYTE),130065084536092528},
	{(const everything_plugin_utf8_t *)"folder.gif",(const everything_plugin_utf8_t *)"image/gif",http_server_folder_gif,sizeof(http_server_folder_gif) / sizeof(BYTE),130065084536092528},
	{(const everything_plugin_utf8_t *)"file.gif",(const everything_plugin_utf8_t *)"image/gif",http_server_file_gif,sizeof(http_server_file_gif) / sizeof(BYTE),130065084536092528},
	{(const everything_plugin_utf8_t *)"everything.gif",(const everything_plugin_utf8_t *)"image/gif",http_server_everything_gif,sizeof(http_server_everything_gif) / sizeof(BYTE),130065084536092528},
	{(const everything_plugin_utf8_t *)"up.gif",(const everything_plugin_utf8_t *)"image/gif",http_server_up_gif,sizeof(http_server_up_gif) / sizeof(BYTE),130065084536092528},
	{(const everything_plugin_utf8_t *)"down.gif",(const everything_plugin_utf8_t *)"image/gif",http_server_down_gif,sizeof(http_server_down_gif) / sizeof(BYTE),130065084536092528},
	{(const everything_plugin_utf8_t *)"updir.gif",(const everything_plugin_utf8_t *)"image/gif",http_server_updir_gif,sizeof(http_server_updir_gif) / sizeof(BYTE),130065084536092528},
};
	
#define HTTP_SERVER_RESOURCE_COUNT (sizeof(http_server_resources) / sizeof(http_server_resource_t))

static void *(EVERYTHING_PLUGIN_API *everything_plugin_mem_alloc)(uintptr_t size);
static void *(EVERYTHING_PLUGIN_API *everything_plugin_mem_calloc)(uintptr_t size);
static void (EVERYTHING_PLUGIN_API *everything_plugin_mem_free)(void *ptr);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_thread_wait_and_close)(everything_plugin_os_thread_t *t);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_closesocket)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_cancel_query)(everything_plugin_db_query_t *q);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_init)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_kill)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_title)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *program_name,const everything_plugin_utf8_t *search,const everything_plugin_utf8_t *setting_format);
static const everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_localization_get_string)(int id);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_path_cat_filename)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *filename);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_get_setting_string)(struct sorted_list_s *sorted_list,const everything_plugin_utf8_t *name,everything_plugin_utf8_t *current_string);
static int (EVERYTHING_PLUGIN_API *everything_plugin_get_setting_int)(struct sorted_list_s *sorted_list,const everything_plugin_utf8_t *name,int current_value);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_is_url_scheme_name_with_double_forward_slash)(const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_get_volume_label)(const everything_plugin_utf8_t *volume_path,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_debug_printf)(const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_printf)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_empty)(everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_compare_nocase_s_sla)(const everything_plugin_utf8_t *s1start,const everything_plugin_utf8_t *lowercase_ascii_s2start);
static HANDLE (EVERYTHING_PLUGIN_API *everything_plugin_os_open_file)(const everything_plugin_utf8_t *filename);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_get_extension)(const everything_plugin_utf8_t *filename);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_registry_get_string)(HKEY root,const everything_plugin_utf8_t *key,const everything_plugin_utf8_t *value,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_copy_utf8_string)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *s);
static HANDLE (EVERYTHING_PLUGIN_API *everything_plugin_os_event_create)(void);
static everything_plugin_os_thread_t *(EVERYTHING_PLUGIN_API *everything_plugin_os_thread_create)(DWORD (WINAPI *thread_proc)(void *),void *param);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_get_app_data_path_cat_filename)(const everything_plugin_utf8_t *filename,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_folder_exists)(everything_plugin_db_t *db,const everything_plugin_utf8_t *filename);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_file_exists)(everything_plugin_db_t *db,const everything_plugin_utf8_t *filename);
static everything_plugin_db_find_t *(EVERYTHING_PLUGIN_API *everything_plugin_db_find_first_file)(everything_plugin_db_t *db,const everything_plugin_utf8_t *path,everything_plugin_utf8_buf_t *filename_cbuf,everything_plugin_fileinfo_fd_t *fd);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_find_next_file)(everything_plugin_db_find_t *fh,everything_plugin_utf8_buf_t *filename_cbuf,everything_plugin_fileinfo_fd_t *fd);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_find_close)(everything_plugin_db_find_t *fh);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_db_find_get_count)(everything_plugin_db_find_t *fh);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_safe_uintptr_mul_sizeof_pointer)(uintptr_t a);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_safe_uintptr_add)(uintptr_t a,uintptr_t b);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_copy_memory)(void *dst,const void *src,uintptr_t size);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_sort_MT)(void **base,uintptr_t count,int (EVERYTHING_PLUGIN_API *comp)(const void *,const void *));
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_get_path_part)(const everything_plugin_utf8_t *file_name,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_unicode_base64_index)(int c);
static int (EVERYTHING_PLUGIN_API *everything_plugin_debug_is_verbose)(void);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_skip_ascii_ws)(const everything_plugin_utf8_t *p);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_realloc_utf8_string)(everything_plugin_utf8_t *ptr,const everything_plugin_utf8_t *s);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_parse_check)(const everything_plugin_utf8_t **pp,const everything_plugin_utf8_t *string);
static EVERYTHING_PLUGIN_QWORD (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_parse_qword)(const everything_plugin_utf8_t **pp);
static int (EVERYTHING_PLUGIN_API *everything_plugin_unicode_is_digit)(int c);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_compare)(const everything_plugin_utf8_t *start1,const everything_plugin_utf8_t *start2);
static DWORD (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_to_dword)(const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_path_canonicalize)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_vprintf)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *format,va_list argptr);
static void (EVERYTHING_PLUGIN_API *everything_plugin_debug_color_printf)(DWORD color,const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_get_local_app_data_path_cat_make_filename)(const everything_plugin_utf8_t *name,const everything_plugin_utf8_t *extension,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_local_app_data_path_cat_filename)(const everything_plugin_utf8_t *filename,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_resize_file)(const everything_plugin_utf8_t *filename,uintptr_t max_size,uintptr_t delta_size);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_make_sure_path_to_file_exists)(const everything_plugin_utf8_t *file_name);
static everything_plugin_output_stream_t *(EVERYTHING_PLUGIN_API *everything_plugin_output_stream_append_file)(const everything_plugin_utf8_t *filename);
static void (EVERYTHING_PLUGIN_API *everything_plugin_output_stream_close)(everything_plugin_output_stream_t *s);
static EVERYTHING_PLUGIN_QWORD (EVERYTHING_PLUGIN_API *everything_plugin_os_get_system_time_as_file_time)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_version_get_text)(everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_filetime)(everything_plugin_utf8_buf_t *cbuf,EVERYTHING_PLUGIN_QWORD ft);
static void (EVERYTHING_PLUGIN_API *everything_plugin_output_stream_write_printf)(everything_plugin_output_stream_t *output_stream,const everything_plugin_utf8_t *format,...);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_peername)(everything_plugin_utf8_buf_t *cbuf,EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static int (EVERYTHING_PLUGIN_API *everything_plugin_unicode_hex_char)(int value);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_grow_length)(everything_plugin_utf8_buf_t *cbuf,uintptr_t length_in_bytes);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_escape_html)(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *str);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_size)(everything_plugin_utf8_buf_t *cbuf,EVERYTHING_PLUGIN_QWORD number);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_filetime_to_localtime)(SYSTEMTIME *localst,EVERYTHING_PLUGIN_QWORD ft);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_zero_memory)(void *ptr,uintptr_t size);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_getaddrinfo)(const char *nodename,const char *servname,const struct everything_plugin_os_winsock_addrinfo* hints,struct everything_plugin_os_winsock_addrinfo** res);
static EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_socket)(int af,int type,int protocol);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_bind)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,const struct everything_plugin_os_winsock_sockaddr *name,int namelen);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_listen)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,int backlog);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSAAsyncSelect)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,HWND hWnd,unsigned int wMsg,long lEvent);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSAGetLastError)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_freeaddrinfo)(struct everything_plugin_os_winsock_addrinfo* ai);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSAStartup)(WORD wVersionRequested,EVERYTHING_PLUGIN_OS_WINSOCK_WSADATA *lpWSAData);
static everything_plugin_ini_t *(EVERYTHING_PLUGIN_API *everything_plugin_ini_open)(everything_plugin_utf8_t *s,const everything_plugin_utf8_t *lowercase_ascii_section);
static void (EVERYTHING_PLUGIN_API *everything_plugin_ini_close)(everything_plugin_ini_t *ini);
static int (EVERYTHING_PLUGIN_API *everything_plugin_ini_find_keyvalue)(everything_plugin_ini_t *ini,const everything_plugin_utf8_t *nocase_ascii_key,everything_plugin_utf8_const_string_t *value_string);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_alloc_utf8_string_n)(const everything_plugin_utf8_t *s,uintptr_t slen);
static everything_plugin_utf8_basic_string_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_basic_string_get_text_plain_file)(const everything_plugin_utf8_t *filename);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_basic_string_free)(everything_plugin_utf8_basic_string_t *s);
static everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_alloc_utf8_string)(const everything_plugin_utf8_t *s);
static everything_plugin_db_t *(EVERYTHING_PLUGIN_API *everything_plugin_db_add_local_ref)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_release)(everything_plugin_db_t *db);
static everything_plugin_db_query_t *(EVERYTHING_PLUGIN_API *everything_plugin_db_query_create)(everything_plugin_db_t *db,void (EVERYTHING_PLUGIN_API *event_proc)(void *user_data,int type),void *user_data);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_destroy)(everything_plugin_db_query_t *q);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_register_class)(UINT style,const everything_plugin_utf8_t *lpszClassName,WNDPROC lpfnWndProc,uintptr_t window_extra,HICON hIcon,HICON hIconSm,HCURSOR hcursor);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_window)(DWORD dwExStyle,const everything_plugin_utf8_t *lpClassName,const everything_plugin_utf8_t *lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);
static const everything_plugin_utf8_t *(EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_parse_csv_item)(const everything_plugin_utf8_t *s,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_ui_task_dialog_show)(HWND parent_hwnd,UINT flags,const everything_plugin_utf8_t *caption,const everything_plugin_utf8_t *main_task,const everything_plugin_utf8_t *format,...);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_WSACleanup)(void);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_count)(const everything_plugin_db_query_t *q);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_name)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_path)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_get_result_indexed_fd)(everything_plugin_db_query_t *q,uintptr_t index,everything_plugin_fileinfo_fd_t *fd);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_query_is_folder_result)(everything_plugin_db_query_t *q,uintptr_t index);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_buf_format_qword)(everything_plugin_utf8_buf_t *cbuf,EVERYTHING_PLUGIN_QWORD number);
static EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_accept)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,struct everything_plugin_os_sockaddr *addr,int *addrlen);
static void (EVERYTHING_PLUGIN_API *everything_plugin_network_set_tcp_nodelay)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static void (EVERYTHING_PLUGIN_API *everything_plugin_network_set_keepalive)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle);
static int (EVERYTHING_PLUGIN_API *everything_plugin_network_recv)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,void *buf,uintptr_t len);
static int (EVERYTHING_PLUGIN_API *everything_plugin_network_send)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,const void *data,uintptr_t len);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_move_memory)(void *dst,const void *src,uintptr_t size);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_winsock_shutdown)(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET s,int how);
static uintptr_t (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_get_length_in_bytes)(const everything_plugin_utf8_t *string);
static int (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_compare_nice_n_n)(const everything_plugin_utf8_t *s1start,uintptr_t s1startlen,const everything_plugin_utf8_t *s2start,uintptr_t s2startlen);
static const everything_plugin_property_t *(EVERYTHING_PLUGIN_API *everything_plugin_property_get_builtin_type)(int type);
static int (EVERYTHING_PLUGIN_API *everything_plugin_db_query_is_fast_sort)(everything_plugin_db_query_t *q,const everything_plugin_property_t *property_type);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_sort)(everything_plugin_db_query_t *q,const everything_plugin_property_t *column_type,int ascending,const everything_plugin_property_t *column_type2,int ascending2,const everything_plugin_property_t *column_type3,int ascending3,int folders_first,int force,int find_duplicate_type,int sort_mix);
static int (EVERYTHING_PLUGIN_API *everything_plugin_property_get_type)(const everything_plugin_property_t *property_type);
static void (EVERYTHING_PLUGIN_API *everything_plugin_db_query_search)(everything_plugin_db_query_t *q,int match_case,int match_whole_word,int match_path,int match_diacritics,int match_prefix,int match_suffix,int ignore_punctuation,int ignore_whitespace,int match_regex,int hide_empty_search_results,int clear_selection,int clear_item_refs,const everything_plugin_utf8_t *search_string,int fast_sort_only,const everything_plugin_property_t *sort_property_type,int sort_ascending,const everything_plugin_property_t *sort_property_type2,int sort_ascending2,const everything_plugin_property_t *sort_property_type3,int sort_ascending3,int folders_first,int track_selected_and_total_file_size,int track_selected_folder_size,int force,int allow_query_access,int allow_read_access,int allow_disk_access,int hide_omit_results,int size_standard,int sort_mix);
static struct everything_plugin_ui_options_page_s *(EVERYTHING_PLUGIN_API *everything_plugin_ui_options_add_plugin_page)(struct everything_plugin_ui_options_add_custom_page_s *add_custom_page,void *user_data,const everything_plugin_utf8_t *name);
static void (EVERYTHING_PLUGIN_API *everything_plugin_set_setting_int)(struct everything_plugin_output_stream_s *output_stream,const everything_plugin_utf8_t *name,int value);
static void (EVERYTHING_PLUGIN_API *everything_plugin_set_setting_string)(everything_plugin_output_stream_t *output_stream,const everything_plugin_utf8_t *name,const everything_plugin_utf8_t *value);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_logical_wide)(void);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_logical_high)(void);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_set_dlg_rect)(HWND parent_hwnd,int id,int x,int y,int wide,int high);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_set_dlg_text)(HWND hDlg,int nIDDlgItem,const everything_plugin_utf8_t *s);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_get_dlg_text)(HWND hwnd,int id,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_browse_for_folder)(HWND parent,const everything_plugin_utf8_t *title,const everything_plugin_utf8_t *default_folder,everything_plugin_utf8_buf_t *cbuf);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_enable_or_disable_dlg_item)(HWND parent_hwnd,int id,int enable);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_save_file_name)(HWND parent,const everything_plugin_utf8_t *title,const everything_plugin_utf8_t *initial_file,everything_plugin_utf8_t *filter,uintptr_t filter_len,DWORD filter_index,const everything_plugin_utf8_t *default_extension,DWORD *out_filter_index,everything_plugin_utf8_buf_t *cbuf);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_get_open_file_name)(HWND parent,const everything_plugin_utf8_t *title,const everything_plugin_utf8_t *initial_file,const everything_plugin_utf8_t *filter,uintptr_t filter_len,DWORD filter_index,const everything_plugin_utf8_t *default_extension,DWORD *out_filter_index,everything_plugin_utf8_buf_t *cbuf);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_checkbox)(HWND parent,int id,DWORD extra_style,int checked,const everything_plugin_utf8_t *text);
static void (EVERYTHING_PLUGIN_API *everything_plugin_os_add_tooltip)(HWND tooltip,HWND parent,int id,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_static)(HWND parent,int id,DWORD extra_window_style,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_edit)(HWND parent,int id,DWORD extra_style,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_number_edit)(HWND parent,int id,DWORD extra_style,__int64 number);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_password_edit)(HWND parent,int id,DWORD extra_style,const everything_plugin_utf8_t *text);
static HWND (EVERYTHING_PLUGIN_API *everything_plugin_os_create_button)(HWND parent,int id,DWORD extra_window_style,const everything_plugin_utf8_t *text);
static int (EVERYTHING_PLUGIN_API *everything_plugin_os_expand_dialog_text_logical_wide_no_prefix)(HWND parent,const everything_plugin_utf8_t *text,int wide);
static void (EVERYTHING_PLUGIN_API *everything_plugin_utf8_string_get_win32_file_namespace)(const everything_plugin_utf8_t *path,everything_plugin_utf8_buf_t *cbuf);

// required procs supplied from Everything.
static http_server_everything_plugin_proc_t http_server_everything_plugin_proc_array[] =
{
	{"mem_alloc",(void *)&everything_plugin_mem_alloc},
	{"mem_calloc",(void *)&everything_plugin_mem_calloc},
	{"mem_free",(void *)&everything_plugin_mem_free},
	{"os_thread_wait_and_close",(void *)&everything_plugin_os_thread_wait_and_close},
	{"os_winsock_closesocket",(void *)&everything_plugin_os_winsock_closesocket},
	{"db_query_cancel",(void *)&everything_plugin_db_cancel_query},
	{"utf8_buf_init",(void *)&everything_plugin_utf8_buf_init},
	{"utf8_buf_kill",(void *)&everything_plugin_utf8_buf_kill},
	{"utf8_buf_format_title",(void *)&everything_plugin_utf8_buf_format_title},
	{"localization_get_string",(void *)&everything_plugin_localization_get_string},
	{"utf8_buf_path_cat_filename",(void *)&everything_plugin_utf8_buf_path_cat_filename},
	{"plugin_get_setting_string",(void *)&everything_plugin_get_setting_string},
	{"plugin_get_setting_int",(void *)&everything_plugin_get_setting_int},
	{"utf8_string_is_url_scheme_name_with_double_forward_slash",(void *)&everything_plugin_utf8_string_is_url_scheme_name_with_double_forward_slash},
	{"os_get_volume_label",(void *)&everything_plugin_os_get_volume_label},
	{"debug_printf",(void *)&everything_plugin_debug_printf},
	{"utf8_buf_printf",(void *)&everything_plugin_utf8_buf_printf},
	{"utf8_buf_empty",(void *)&everything_plugin_utf8_buf_empty},
	{"utf8_string_compare_nocase_s_sla",(void *)&everything_plugin_utf8_string_compare_nocase_s_sla},
	{"os_open_file",(void *)&everything_plugin_os_open_file},
	{"utf8_string_get_extension",(void *)&everything_plugin_utf8_string_get_extension},
	{"os_registry_get_string",(void *)&everything_plugin_os_registry_get_string},
	{"utf8_buf_copy_utf8_string",(void *)&everything_plugin_utf8_buf_copy_utf8_string},
	{"os_event_create",(void *)&everything_plugin_os_event_create},
	{"os_thread_create",(void *)&everything_plugin_os_thread_create},
	{"os_get_app_data_path_cat_filename",(void *)&everything_plugin_os_get_app_data_path_cat_filename},
	{"db_folder_exists",(void *)&everything_plugin_db_folder_exists},
	{"db_file_exists",(void *)&everything_plugin_db_file_exists},
	{"db_find_first_file",(void *)&everything_plugin_db_find_first_file},
	{"db_find_next_file",(void *)&everything_plugin_db_find_next_file},
	{"db_find_close",(void *)&everything_plugin_db_find_close},
	{"db_find_get_count",(void *)&everything_plugin_db_find_get_count},
	{"safe_uintptr_mul_sizeof_pointer",(void *)&everything_plugin_safe_uintptr_mul_sizeof_pointer},
	{"safe_uintptr_add",(void *)&everything_plugin_safe_uintptr_add},
	{"os_copy_memory",(void *)&everything_plugin_os_copy_memory},
	{"os_sort_MT",(void *)&everything_plugin_os_sort_MT},
	{"utf8_string_get_path_part",(void *)&everything_plugin_utf8_string_get_path_part},
	{"unicode_base64_index",(void *)&everything_plugin_unicode_base64_index},
	{"debug_is_verbose",(void *)&everything_plugin_debug_is_verbose},
	{"utf8_string_skip_ascii_ws",(void *)&everything_plugin_utf8_string_skip_ascii_ws},
	{"utf8_string_realloc_utf8_string",(void *)&everything_plugin_utf8_string_realloc_utf8_string},
	{"utf8_string_parse_check",(void *)&everything_plugin_utf8_string_parse_check},
	{"utf8_string_parse_qword",(void *)&everything_plugin_utf8_string_parse_qword},
	{"unicode_is_digit",(void *)&everything_plugin_unicode_is_digit},
	{"utf8_string_compare",(void *)&everything_plugin_utf8_string_compare},
	{"utf8_string_to_dword",(void *)&everything_plugin_utf8_string_to_dword},
	{"utf8_buf_path_canonicalize",(void *)&everything_plugin_utf8_buf_path_canonicalize},
	{"utf8_buf_vprintf",(void *)&everything_plugin_utf8_buf_vprintf},
	{"debug_color_printf",(void *)&everything_plugin_debug_color_printf},
	{"os_get_local_app_data_path_cat_make_filename",(void *)&everything_plugin_os_get_local_app_data_path_cat_make_filename},
	{"os_get_local_app_data_path_cat_filename",(void *)&everything_plugin_os_get_local_app_data_path_cat_filename},
	{"os_resize_file",(void *)&everything_plugin_os_resize_file},
	{"os_make_sure_path_to_file_exists",(void *)&everything_plugin_os_make_sure_path_to_file_exists},
	{"output_stream_append_file",(void *)&everything_plugin_output_stream_append_file},
	{"output_stream_close",(void *)&everything_plugin_output_stream_close},
	{"os_get_system_time_as_file_time",(void *)&everything_plugin_os_get_system_time_as_file_time},
	{"version_get_text",(void *)&everything_plugin_version_get_text},
	{"utf8_buf_format_filetime",(void *)&everything_plugin_utf8_buf_format_filetime},
	{"output_stream_write_printf",(void *)&everything_plugin_output_stream_write_printf},
	{"utf8_buf_format_peername",(void *)&everything_plugin_utf8_buf_format_peername},
	{"unicode_hex_char",(void *)&everything_plugin_unicode_hex_char},
	{"utf8_buf_grow_length",(void *)&everything_plugin_utf8_buf_grow_length},
	{"utf8_buf_escape_html",(void *)&everything_plugin_utf8_buf_escape_html},
	{"utf8_buf_format_size",(void *)&everything_plugin_utf8_buf_format_size},
	{"os_filetime_to_localtime",(void *)&everything_plugin_os_filetime_to_localtime},
	{"os_zero_memory",(void *)&everything_plugin_os_zero_memory},
	{"os_winsock_getaddrinfo",(void *)&everything_plugin_os_winsock_getaddrinfo},
	{"os_winsock_socket",(void *)&everything_plugin_os_winsock_socket},
	{"os_winsock_bind",(void *)&everything_plugin_os_winsock_bind},
	{"os_winsock_listen",(void *)&everything_plugin_os_winsock_listen},
	{"os_winsock_WSAAsyncSelect",(void *)&everything_plugin_os_winsock_WSAAsyncSelect},
	{"os_winsock_WSAGetLastError",(void *)&everything_plugin_os_winsock_WSAGetLastError},
	{"os_winsock_freeaddrinfo",(void *)&everything_plugin_os_winsock_freeaddrinfo},
	{"os_winsock_WSAStartup",(void *)&everything_plugin_os_winsock_WSAStartup},
	{"os_winsock_WSACleanup",(void *)&everything_plugin_os_winsock_WSACleanup},
	{"ini_open",(void *)&everything_plugin_ini_open},
	{"ini_close",(void *)&everything_plugin_ini_close},
	{"ini_find_keyvalue",(void *)&everything_plugin_ini_find_keyvalue},
	{"utf8_string_alloc_utf8_string_n",(void *)&everything_plugin_utf8_string_alloc_utf8_string_n},
	{"utf8_basic_string_get_text_plain_file",(void *)&everything_plugin_utf8_basic_string_get_text_plain_file},
	{"utf8_basic_string_free",(void *)&everything_plugin_utf8_basic_string_free},
	{"utf8_string_alloc_utf8_string",(void *)&everything_plugin_utf8_string_alloc_utf8_string},
	{"db_add_local_ref",(void *)&everything_plugin_db_add_local_ref},
	{"db_release",(void *)&everything_plugin_db_release},
	{"db_query_create",(void *)&everything_plugin_db_query_create},
	{"db_query_destroy",(void *)&everything_plugin_db_query_destroy},
	{"os_register_class",(void *)&everything_plugin_os_register_class},
	{"os_create_window",(void *)&everything_plugin_os_create_window},
	{"utf8_string_parse_csv_item",(void *)&everything_plugin_utf8_string_parse_csv_item},
	{"ui_task_dialog_show",(void *)&everything_plugin_ui_task_dialog_show},
	{"db_query_get_result_count",(void *)&everything_plugin_db_query_get_result_count},
	{"db_query_get_result_name",(void *)&everything_plugin_db_query_get_result_name},
	{"db_query_get_result_path",(void *)&everything_plugin_db_query_get_result_path},
	{"db_query_get_result_indexed_fd",(void *)&everything_plugin_db_query_get_result_indexed_fd},
	{"db_query_is_folder_result",(void *)&everything_plugin_db_query_is_folder_result},
	{"utf8_buf_format_qword",(void *)&everything_plugin_utf8_buf_format_qword},
	{"os_winsock_accept",(void *)&everything_plugin_os_winsock_accept},
	{"network_set_tcp_nodelay",(void *)&everything_plugin_network_set_tcp_nodelay},
	{"network_set_keepalive",(void *)&everything_plugin_network_set_keepalive},
	{"network_recv",(void *)&everything_plugin_network_recv},
	{"network_send",(void *)&everything_plugin_network_send},
	{"os_move_memory",(void *)&everything_plugin_os_move_memory},
	{"os_winsock_shutdown",(void *)&everything_plugin_os_winsock_shutdown},
	{"utf8_string_get_length_in_bytes",(void *)&everything_plugin_utf8_string_get_length_in_bytes},
	{"utf8_string_compare_nice_n_n",(void *)&everything_plugin_utf8_string_compare_nice_n_n},
	{"property_get_builtin_type",(void *)&everything_plugin_property_get_builtin_type},
	{"db_query_is_fast_sort",(void *)&everything_plugin_db_query_is_fast_sort},
	{"db_query_sort",(void *)&everything_plugin_db_query_sort},
	{"property_get_type",(void *)&everything_plugin_property_get_type},
	{"db_query_search",(void *)&everything_plugin_db_query_search},
	{"ui_options_add_plugin_page",(void *)&everything_plugin_ui_options_add_plugin_page},
	{"plugin_set_setting_int",(void *)&everything_plugin_set_setting_int},
	{"plugin_set_setting_string",(void *)&everything_plugin_set_setting_string},
	{"os_get_logical_wide",(void *)&everything_plugin_os_get_logical_wide},
	{"os_get_logical_high",(void *)&everything_plugin_os_get_logical_high},
	{"os_set_dlg_rect",(void *)&everything_plugin_os_set_dlg_rect},
	{"os_set_dlg_text",(void *)&everything_plugin_os_set_dlg_text},
	{"os_get_dlg_text",(void *)&everything_plugin_os_get_dlg_text},
	{"os_browse_for_folder",(void *)&everything_plugin_os_browse_for_folder},
	{"os_enable_or_disable_dlg_item",(void *)&everything_plugin_os_enable_or_disable_dlg_item},
	{"os_get_save_file_name",(void *)&everything_plugin_os_get_save_file_name},
	{"os_get_open_file_name",(void *)&everything_plugin_os_get_open_file_name},
	{"os_create_checkbox",(void *)&everything_plugin_os_create_checkbox},
	{"os_add_tooltip",(void *)&everything_plugin_os_add_tooltip},
	{"os_create_static",(void *)&everything_plugin_os_create_static},
	{"os_create_edit",(void *)&everything_plugin_os_create_edit},
	{"os_create_number_edit",(void *)&everything_plugin_os_create_number_edit},
	{"os_create_password_edit",(void *)&everything_plugin_os_create_password_edit},
	{"os_create_button",(void *)&everything_plugin_os_create_button},
	{"os_expand_dialog_text_logical_wide_no_prefix",(void *)&everything_plugin_os_expand_dialog_text_logical_wide_no_prefix},
	{"utf8_string_get_win32_file_namespace",(void *)&everything_plugin_utf8_string_get_win32_file_namespace},
};
	
#define HTTP_SERVER_EVERYTHING_PLUGIN_PROC_COUNT (sizeof(http_server_everything_plugin_proc_array) / sizeof(http_server_everything_plugin_proc_t))

__declspec( dllexport) void * EVERYTHING_PLUGIN_API everything_plugin_proc(DWORD msg,void *data)
{
	switch(msg)
	{
		case EVERYTHING_PLUGIN_PM_INIT:
			
			// find procs.
			
			{
				uintptr_t index;
				
				for(index=0;index<HTTP_SERVER_EVERYTHING_PLUGIN_PROC_COUNT;index++)
				{
					void *proc;
					
					proc = ((everything_plugin_get_proc_address_t)data)(http_server_everything_plugin_proc_array[index].name);
					
					if (!proc)
					{
						return (void *)0;
					}
					
					*http_server_everything_plugin_proc_array[index].proc_address_ptr = proc;
				}
			}
		
			http_server_title_format = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_username = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_password = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_home = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_default_page = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_log_file_name = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_bindings = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_strings_filename = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");
			http_server_header = everything_plugin_utf8_string_alloc_utf8_string((const everything_plugin_utf8_t *)"");

			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_KILL:
		
			everything_plugin_mem_free(http_server_title_format);
			everything_plugin_mem_free(http_server_username);
			everything_plugin_mem_free(http_server_password);
			everything_plugin_mem_free(http_server_home);
			everything_plugin_mem_free(http_server_default_page);
			everything_plugin_mem_free(http_server_log_file_name);
			everything_plugin_mem_free(http_server_bindings);
			everything_plugin_mem_free(http_server_strings_filename);
			everything_plugin_mem_free(http_server_header);

			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_START:
		
			// load settings
			http_server_enabled = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"enabled",http_server_enabled);
			http_server_title_format = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"title_format",http_server_title_format);
			http_server_port = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"port",http_server_port);
			http_server_username = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"username",http_server_username);
			http_server_password = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"password",http_server_password);
			http_server_home = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"home",http_server_home);
			http_server_default_page = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"default_page",http_server_default_page);
			http_server_log_file_name = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"log_file_name",http_server_log_file_name);
			http_server_logging_enabled = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"logging_enabled",http_server_logging_enabled);
			http_server_log_max_size = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"log_max_size",http_server_log_max_size);
			http_server_log_delta_size = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"log_delta_size",http_server_log_delta_size);
			http_server_allow_file_download = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"allow_file_download",http_server_allow_file_download);
			http_server_bindings = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"bindings",http_server_bindings);
			http_server_items_per_page = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"items_per_page",http_server_items_per_page);
			http_server_show_drive_labels = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"show_drive_label",http_server_show_drive_labels);
			http_server_strings_filename = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"strings_filename",http_server_strings_filename);
			http_server_header = everything_plugin_get_setting_string(data,(const everything_plugin_utf8_t *)"header",http_server_header);
			http_server_allow_query_access = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"allow_query_access",http_server_allow_query_access);
			http_server_allow_disk_access = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"allow_disk_access",http_server_allow_disk_access);
			http_server_default_sort = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"default_sort",http_server_default_sort);
			http_server_default_sort_ascending = everything_plugin_get_setting_int(data,(const everything_plugin_utf8_t *)"default_sort_ascending",http_server_default_sort_ascending);

			// apply settings.
			http_server_apply_settings();
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_STOP:
			http_server_shutdown();
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_GET_PLUGIN_VERSION:
			return (void *)EVERYTHING_PLUGIN_VERSION;
			
		case EVERYTHING_PLUGIN_PM_GET_NAME:
			return (void *)everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER);
			
		case EVERYTHING_PLUGIN_PM_GET_DESCRIPTION:
			return (void *)everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_DESCRIPTION);
			
		case EVERYTHING_PLUGIN_PM_GET_AUTHOR:
			return "voidtools";
			
		case EVERYTHING_PLUGIN_PM_GET_VERSION:
			return PLUGINVERSION;
			
		case EVERYTHING_PLUGIN_PM_GET_LINK:
			return (void *)everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_PLUGIN_LINK);
			
		case EVERYTHING_PLUGIN_PM_ADD_OPTIONS_PAGES:

			everything_plugin_ui_options_add_plugin_page(data,NULL,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER));
			
			return (void *)1;
	
		case EVERYTHING_PLUGIN_PM_LOAD_OPTIONS_PAGE:
		
			{
				HWND page_hwnd;
				
				page_hwnd = ((everything_plugin_load_options_page_t *)data)->page_hwnd;

				http_server_create_checkbox(data,HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_HTTP_SERVER,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_HTTP_SERVER_HELP,http_server_enabled);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_BINDINGS_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_BINDINGS);
				http_server_create_edit(data,HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_BINDINGS_HELP,http_server_bindings);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_PORT_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PORT);
				http_server_create_number_edit(data,HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PORT_HELP,http_server_port);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_USERNAME_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_USERNAME);
				http_server_create_edit(data,HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_USERNAME_HELP,http_server_username);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_PASSWORD_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PASSWORD);
				http_server_create_password_edit(data,HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PASSWORD_HELP,http_server_password);
				http_server_create_checkbox(data,HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_HTTP_SERVER_LOGGING,EVERYTHING_PLUGIN_LOCALIZATION_ENABLE_HTTP_SERVER_LOGGING_HELP,http_server_logging_enabled);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_LOG_FILE);
				http_server_create_edit(data,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_LOG_FILE_HELP,http_server_log_file_name);
				http_server_create_button(data,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_LOG_FILE_BROWSE,EVERYTHING_PLUGIN_LOCALIZATION_BROWSE_FOR_THE_HTTP_SERVER_LOG_FILE);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_MAX_LOG_SIZE);
				http_server_create_number_edit(data,HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_LOG_MAX_SIZE,(http_server_log_max_size+1023)/1024);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_KB_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_KB);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_HOME_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SERVE_PAGES_FROM);
				http_server_create_edit(data,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SERVE_PAGES_FROM_HELP,http_server_home);
				http_server_create_button(data,HTTP_SERVER_PLUGIN_ID_HOME_BROWSE_BUTTON,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SERVE_PAGES_FROM_BROWSE,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SERVE_PAGES_FROM_BROWSE_HELP);
				http_server_create_static(data,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_STATIC,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_DEFAULT_PAGE);
				http_server_create_edit(data,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_DEFAULT_PAGE_HELP,http_server_default_page);
				http_server_create_button(data,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_BROWSE_BUTTON,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_DEFAULT_PAGE_BROWSE,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_DEFAULT_PAGE_BROWSE_HELP);
				http_server_create_checkbox(data,HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_ALLOW_FILE_DOWNLOAD,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_ALLOW_FILE_DOWNLOAD_HELP,http_server_allow_file_download);
				http_server_create_button(data,HTTP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS,WS_GROUP,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_RESTORE_DEFAULTS,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_RESTORE_DEFAULTS_HELP);
				
				http_server_update_options_page(page_hwnd);
			}
			
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_SAVE_OPTIONS_PAGE:
		
			{
				HWND page_hwnd;
				
				page_hwnd = ((everything_plugin_save_options_page_t *)data)->page_hwnd;
				
				http_server_bindings = http_server_get_options_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX,http_server_bindings);
				http_server_port = GetDlgItemInt(page_hwnd,HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX,NULL,FALSE);
				http_server_username = http_server_get_options_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX,http_server_username);
				http_server_password = http_server_get_options_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX,http_server_password);
				http_server_logging_enabled = (IsDlgButtonChecked(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX) == BST_CHECKED);
				http_server_allow_file_download = (IsDlgButtonChecked(page_hwnd,HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX) == BST_CHECKED);
				http_server_enabled = (IsDlgButtonChecked(page_hwnd,HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX) == BST_CHECKED);
				http_server_log_file_name = http_server_get_options_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,http_server_log_file_name);
				http_server_log_max_size = GetDlgItemInt(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX,NULL,FALSE) * 1024;
				http_server_home = http_server_get_options_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,http_server_home);
				http_server_default_page = http_server_get_options_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,http_server_default_page);
				
				// restart servers?
				// why not ask the user..
				if (!http_server_apply_settings())
				{
					((everything_plugin_save_options_page_t *)data)->enable_apply = 1;
				}
			}
			
			return (void *)1;
					
		case EVERYTHING_PLUGIN_PM_SAVE_SETTINGS:
					
			// save settings
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"enabled",http_server_enabled);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"title_format",http_server_title_format);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"port",http_server_port);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"username",http_server_username);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"password",http_server_password);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"home",http_server_home);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"default_page",http_server_default_page);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"log_file_name",http_server_log_file_name);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"logging_enabled",http_server_logging_enabled);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"log_max_size",http_server_log_max_size);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"log_delta_size",http_server_log_delta_size);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"allow_file_download",http_server_allow_file_download);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"bindings",http_server_bindings);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"items_per_page",http_server_items_per_page);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"show_drive_label",http_server_show_drive_labels);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"strings_filename",http_server_strings_filename);
			everything_plugin_set_setting_string(data,(const everything_plugin_utf8_t *)"header",http_server_header);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"allow_query_access",http_server_allow_query_access);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"allow_disk_access",http_server_allow_disk_access);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"default_sort",http_server_default_sort);
			everything_plugin_set_setting_int(data,(const everything_plugin_utf8_t *)"default_sort_ascending",http_server_default_sort_ascending);
		
			return (void *)1;
				
					
		case EVERYTHING_PLUGIN_PM_GET_OPTIONS_PAGE_MINMAX:
			
			((everything_plugin_get_options_page_minmax_t *)data)->wide = 200;
			((everything_plugin_get_options_page_minmax_t *)data)->high = 392;
			return (void *)1;
			
		case EVERYTHING_PLUGIN_PM_SIZE_OPTIONS_PAGE:

			{
				HWND page_hwnd;
				int static_wide;
				int button_wide;
				RECT rect;
				int x;
				int y;
				int wide;
				int high;
				
				page_hwnd = ((everything_plugin_size_options_page_t *)data)->page_hwnd;
				GetClientRect(page_hwnd,&rect);
				wide = rect.right - rect.left;
				high = rect.bottom - rect.top;
	
				wide = (wide * 96) / everything_plugin_os_get_logical_wide();
				high = (high * 96) / everything_plugin_os_get_logical_high();

				x = 12;
				y = 12;
				wide -= 24;
				high -= 24;
				
				static_wide = 0;
				static_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_BINDINGS,static_wide);
				static_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PORT,static_wide);
				static_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_USERNAME,static_wide);
				static_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PASSWORD,static_wide);
				static_wide += 6;
				
				button_wide = 75 - 24;
				button_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_LOG_FILE_BROWSE,button_wide);
				button_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_DEFAULT_PAGE_BROWSE,button_wide);
				button_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SERVE_PAGES_FROM_BROWSE,button_wide);
				button_wide += 24;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_BINDINGS_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX,x+static_wide,y,wide - (static_wide),EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_PORT_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX,x+static_wide,y,75,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_USERNAME_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX,x+static_wide,y,wide - (static_wide),EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_PASSWORD_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX,x+static_wide,y,wide - (static_wide),EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				// log file
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH + EVERYTHING_PLUGIN_OS_DLG_STATIC_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,x,y+1,wide-button_wide-6,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,x+wide-button_wide,y,button_wide,EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;

				static_wide = 0;
				static_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_MAX_LOG_SIZE,static_wide);
				static_wide += 6;
				
				// max size				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,x,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX,x+static_wide,y,75,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_KB_STATIC,x+static_wide+75+7,y+3,static_wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				// home				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_STATIC,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH + EVERYTHING_PLUGIN_OS_DLG_STATIC_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,x,y+1,wide-button_wide-6,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_BROWSE_BUTTON,x+wide-button_wide,y,button_wide,EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				// default page				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_STATIC,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_STATIC_HIGH + EVERYTHING_PLUGIN_OS_DLG_STATIC_SEPARATOR;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,x,y+1,wide-button_wide-6,EVERYTHING_PLUGIN_OS_DLG_EDIT_HIGH);
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_BROWSE_BUTTON,x+wide-button_wide,y,button_wide,EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH + EVERYTHING_PLUGIN_OS_DLG_SEPARATOR;
				
				// check boxes
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX,x,y,wide,EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH);
				y += EVERYTHING_PLUGIN_OS_DLG_CHECKBOX_HIGH + 12;
				
				// restore defaults
				button_wide = 75 - 24;
				button_wide = http_server_expand_min_wide(page_hwnd,EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_RESTORE_DEFAULTS,button_wide);
				button_wide += 24;
				
				everything_plugin_os_set_dlg_rect(page_hwnd,HTTP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS,x + wide - button_wide,12 + high - EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH,button_wide,EVERYTHING_PLUGIN_OS_DLG_BUTTON_HIGH);
			
				break;
			}
			
			return (void *)1;

		case EVERYTHING_PLUGIN_PM_OPTIONS_PAGE_PROC:
		
			{
				HWND page_hwnd;
				
				page_hwnd = ((everything_plugin_options_page_proc_t *)data)->page_hwnd;
				
				switch(((everything_plugin_options_page_proc_t *)data)->msg)
				{
					case WM_COMMAND:
					
						switch(LOWORD(((everything_plugin_options_page_proc_t *)data)->wParam))
						{
							case HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX:
							case HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX:
							case HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX:
								http_server_update_options_page(page_hwnd);
								http_server_enable_options_apply(data);
								break;
						
							case HTTP_SERVER_PLUGIN_ID_RESTORE_DEFAULTS:

								CheckDlgButton(page_hwnd,HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX,BST_UNCHECKED);
								CheckDlgButton(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX,BST_CHECKED);
								CheckDlgButton(page_hwnd,HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX,BST_CHECKED);
								
								everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,(const everything_plugin_utf8_t *)"");
								SetDlgItemInt(page_hwnd,HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX,HTTP_SERVER_DEFAULT_PORT,FALSE);
								SetDlgItemInt(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX,4096,FALSE);
								everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX,(const everything_plugin_utf8_t *)"");
								everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX,(const everything_plugin_utf8_t *)"");

								http_server_update_options_page(page_hwnd);
								http_server_enable_options_apply(data);

								break;
							
							case HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX:
							case HTTP_SERVER_PLUGIN_ID_HOME_EDIT:
							case HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT:
							case HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX:
							case HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX:
							case HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX:
							case HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX:
							case HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX:

								if (HIWORD(((everything_plugin_options_page_proc_t *)data)->wParam) == EN_CHANGE)
								{
									http_server_enable_options_apply(data);
								}

								break;
									
							case HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON:
								{
									everything_plugin_utf8_buf_t absfilename_cbuf;
									
									everything_plugin_utf8_buf_init(&absfilename_cbuf);

									{
										everything_plugin_utf8_buf_t filename_cbuf;
										
										everything_plugin_utf8_buf_init(&filename_cbuf);
								
										everything_plugin_os_get_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,&filename_cbuf);
										
										if (*filename_cbuf.buf)
										{
											everything_plugin_os_get_local_app_data_path_cat_filename(filename_cbuf.buf,&absfilename_cbuf);
										}
										else
										{
											everything_plugin_os_get_local_app_data_path_cat_make_filename((const everything_plugin_utf8_t *)"Logs\\HTTP_Server_Log",(const everything_plugin_utf8_t *)".txt",&absfilename_cbuf);
										}
										
										everything_plugin_utf8_buf_kill(&filename_cbuf);
									}

									{			
										everything_plugin_utf8_buf_t filename_cbuf;
										everything_plugin_utf8_buf_t filter_cbuf;
										
										everything_plugin_utf8_buf_init(&filename_cbuf);
										everything_plugin_utf8_buf_init(&filter_cbuf);

										everything_plugin_utf8_buf_printf(&filter_cbuf,(const everything_plugin_utf8_t *)"%s (*.txt)%c*.txt%c%s (*.*)%c*.*%c%c",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_TEXT_FILES),0,0,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ALL_FILES),0,0,0);
										
										if (everything_plugin_os_get_save_file_name(((everything_plugin_options_page_proc_t *)data)->options_hwnd,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_SAVE_HTTP_SERVER_LOG_FILE),absfilename_cbuf.buf,filter_cbuf.buf,filter_cbuf.len,1,(const everything_plugin_utf8_t *)"txt",NULL,&filename_cbuf))
										{	
											everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,filename_cbuf.buf);
										}	

										everything_plugin_utf8_buf_kill(&filter_cbuf);
										everything_plugin_utf8_buf_kill(&filename_cbuf);
									}

									everything_plugin_utf8_buf_kill(&absfilename_cbuf);
								}
									
								break;
								
							case HTTP_SERVER_PLUGIN_ID_HOME_BROWSE_BUTTON:
								
								{			
									everything_plugin_utf8_buf_t abspath_cbuf;
								
									everything_plugin_utf8_buf_init(&abspath_cbuf);

									{
										everything_plugin_utf8_buf_t path_cbuf;
										
										everything_plugin_utf8_buf_init(&path_cbuf);
								
										everything_plugin_os_get_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,&path_cbuf);
										
										everything_plugin_os_get_app_data_path_cat_filename(*path_cbuf.buf ? path_cbuf.buf : (const everything_plugin_utf8_t *)"HTTP Server",&abspath_cbuf);
										
										everything_plugin_utf8_buf_kill(&path_cbuf);
									}

									{			
										everything_plugin_utf8_buf_t path_cbuf;
										
										everything_plugin_utf8_buf_init(&path_cbuf);
										
										if (everything_plugin_os_browse_for_folder(page_hwnd,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SELECT_HOME_CAPTION),abspath_cbuf.buf,&path_cbuf))
										{
											everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,path_cbuf.buf);
										}	

										everything_plugin_utf8_buf_kill(&path_cbuf);
									}

									everything_plugin_utf8_buf_kill(&abspath_cbuf);
								
								}		
							
								break;
			
							case HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_BROWSE_BUTTON:

								{
									everything_plugin_utf8_buf_t absfilename_cbuf;
									
									everything_plugin_utf8_buf_init(&absfilename_cbuf);

									{
										everything_plugin_utf8_buf_t filename_cbuf;
										
										everything_plugin_utf8_buf_init(&filename_cbuf);
								
										everything_plugin_os_get_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,&filename_cbuf);
										
										everything_plugin_os_get_app_data_path_cat_filename(*filename_cbuf.buf ? filename_cbuf.buf : (const everything_plugin_utf8_t *)"index.html",&absfilename_cbuf);
										
										everything_plugin_utf8_buf_kill(&filename_cbuf);
									}

									{			
										everything_plugin_utf8_buf_t filename_cbuf;
										everything_plugin_utf8_buf_t filter_cbuf;
										
										everything_plugin_utf8_buf_init(&filename_cbuf);
										everything_plugin_utf8_buf_init(&filter_cbuf);

										everything_plugin_utf8_buf_printf(&filter_cbuf,(const everything_plugin_utf8_t *)"%s (*.*)%c*.*%c%c",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_ALL_FILES),0,0,0);
										
										if (everything_plugin_os_get_open_file_name(((everything_plugin_options_page_proc_t *)data)->options_hwnd,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_SELECT_DEFAULT_PAGE_CAPTION),absfilename_cbuf.buf,filter_cbuf.buf,filter_cbuf.len,1,(const everything_plugin_utf8_t *)"html",NULL,&filename_cbuf))
										{	
											everything_plugin_os_set_dlg_text(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,filename_cbuf.buf);
										}	

										everything_plugin_utf8_buf_kill(&filter_cbuf);
										everything_plugin_utf8_buf_kill(&filename_cbuf);
									}

									everything_plugin_utf8_buf_kill(&absfilename_cbuf);
								}
										
								break;
						}
					
						break;
					
				}
			}
			
			return (void *)1;
	}
	
	return 0;
}

// insert the client
http_server_client_t *http_server_client_create(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET client_socket)
{
	http_server_client_t *c;
	
	// alloc
	c = everything_plugin_mem_calloc(sizeof(http_server_client_t));
	
	c->show_max = http_server_items_per_page;
	c->sort = http_server_default_sort;
	c->sort_ascending = http_server_default_sort_ascending;
	c->socket_handle = client_socket;
	c->data_file = INVALID_HANDLE_VALUE;
	
	// insert
	if (_http_server->client_start)
	{
		_http_server->client_last->next = c;
		c->prev = _http_server->client_last;
	}
	else
	{
		_http_server->client_start = c;
		c->prev = 0;
	}
	
	c->next = 0;
	_http_server->client_last = c;
	
	return c;
}

// remove the client
void http_server_client_destroy(http_server_client_t *c)
{
	if (c->prev)
	{
		c->prev->next = c->next;
	}
	else
	{
		_http_server->client_start = c->next;
	}
	
	if (c->next)
	{
		c->next->prev = c->prev;
	}
	else
	{
		_http_server->client_last = c->prev;
	}

	// free
	if (c->get)
	{
		everything_plugin_mem_free(c->get);
	}

	// destroy send buffer.	
	if (c->send_buffer)
	{
		everything_plugin_mem_free(c->send_buffer);
	}
	
	if (c->data_file != INVALID_HANDLE_VALUE)
	{
		EnterCriticalSection(&c->data_cs);
		c->data_abort = 1;
		LeaveCriticalSection(&c->data_cs);
		
		SetEvent(c->data_hevent);
		
		everything_plugin_os_thread_wait_and_close(c->data_thread);
		CloseHandle(c->data_hevent);

		DeleteCriticalSection(&c->data_cs);		

		CloseHandle(c->data_file);
		everything_plugin_mem_free(c->data_buffer);
	}

	// destroy recv packets.
	{
		http_server_recv_chunk_t *recv_chunk;
		http_server_recv_chunk_t *next_recv_chunk;
		
		recv_chunk = c->recv_chunk_start;
		
		while(recv_chunk)
		{
			next_recv_chunk = recv_chunk->next;
			
			http_server_recv_chunk_destroy(recv_chunk);
			
			recv_chunk = next_recv_chunk;
		}
	}	
	
	// destroy send packets.
	{
		http_server_send_chunk_t *send_chunk;
		http_server_send_chunk_t *next_send_chunk;
		
		send_chunk = c->send_chunk_start;
		
		while(send_chunk)
		{
			next_send_chunk = send_chunk->next;
			
			http_server_send_chunk_destroy(send_chunk);
			
			send_chunk = next_send_chunk;
		}
	}	
	
	// socket handle can be INVALID if we send a file
	// the thread will take over the socket and close it once the thread has completed.
	if (c->socket_handle != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
	{
		everything_plugin_os_winsock_closesocket(c->socket_handle);
	}
	
	if (c->search_string)
	{
		everything_plugin_mem_free(c->search_string);
	}
	
	// remove from query list
	if (c->is_query)
	{
		http_server_remove_query(c);
	}
	
	// remove active query
	if (_http_server->client_query_active == c)
	{
		// we should cancel the search too
		everything_plugin_db_cancel_query(_http_server->q);
		
		_http_server->is_current_query = 0;
		_http_server->client_query_active = 0;
	
		// start next query.
		http_server_start_next_query();
	}
	
	everything_plugin_mem_free(c);
}

static int gethex(everything_plugin_utf8_t c)
{
	if ((c >= '0') && (c <= '9')) return c-'0';
	if ((c >= 'A') && (c <= 'F')) return c-'A' +10;
	if ((c >= 'a') && (c <= 'f')) return c-'a' +10;
	
	return 0;
}

static void http_server_unescapeurl(everything_plugin_utf8_buf_t *cbuf)
{
	everything_plugin_utf8_t *d;
	const everything_plugin_utf8_t *p;
	
	d = cbuf->buf;
	p = cbuf->buf;
	
	while(*p)
	{
		if ((*p == '%') && (p[1]) && (p[2]))
		{
			p++;
			*d++ = (gethex(*p) << 4) | (gethex(p[1]));
			p += 2;
		}
		else
		{
			*d++ = *p++;
		}
	}
	
	*d = 0;
	
	cbuf->len = (d - cbuf->buf);
}

// 
void http_server_client_send_head(http_server_client_t *c,const everything_plugin_utf8_t *title)
{
	// requirements for IE to function.
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_HEAD_BEGIN);

	{
		everything_plugin_utf8_buf_t cbuf;
		
		everything_plugin_utf8_buf_init(&cbuf);
		
		everything_plugin_utf8_buf_format_title(&cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_EVERYTHING),title,*http_server_title_format ? http_server_title_format : everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_TITLE_FORMAT));

		http_server_send_escape_html(c,cbuf.buf);
		
		everything_plugin_utf8_buf_kill(&cbuf);
	}

	// requirements for IE to function.
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_HEAD_END);
}

// print folder
static void http_server_html_td(http_server_client_t *c,int alternate,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *name,int is_folder,everything_plugin_fileinfo_fd_t *fd,int show_path)
{
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TR);
	http_server_client_send_utf8(c,alternate ? (const everything_plugin_utf8_t *)"trdata2" : (const everything_plugin_utf8_t *)"trdata1");
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TR2);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD);
	http_server_client_send_utf8(c,is_folder ? (const everything_plugin_utf8_t *)"folder" : (const everything_plugin_utf8_t *)"file");
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD2);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR);

	if ((is_folder) || (http_server_allow_file_download))
	{
		everything_plugin_utf8_buf_t fullpathname_cbuf;
		
		everything_plugin_utf8_buf_init(&fullpathname_cbuf);
		
		everything_plugin_utf8_buf_path_cat_filename(&fullpathname_cbuf,path,name);

		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_A);

		if (everything_plugin_utf8_string_is_url_scheme_name_with_double_forward_slash(fullpathname_cbuf.buf))
		{
			http_server_send_escape_html(c,fullpathname_cbuf.buf);
		}
		else
		{
			http_server_client_printf(c,(const everything_plugin_utf8_t *)"/");

			{
				everything_plugin_utf8_buf_t link_cbuf;
				
				everything_plugin_utf8_buf_init(&link_cbuf);
				
				http_server_escape_url_filename(&link_cbuf,fullpathname_cbuf.buf);
				
				http_server_send_escape_html(c,link_cbuf.buf);
				
				everything_plugin_utf8_buf_kill(&link_cbuf);
			}
		}
		
		if (is_folder)
		{
			http_server_send_sort_url_param(c);
		}

		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_A2);

		// release
		everything_plugin_utf8_buf_kill(&fullpathname_cbuf);
	}

	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_ICON);
	http_server_client_send_utf8(c,is_folder ? (const everything_plugin_utf8_t *)"folder.gif" : (const everything_plugin_utf8_t *)"file.gif");
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_ICON2);

	// drive labels? drive: root ?
	// don't show drive labels from root:
	if ((http_server_show_drive_labels) && (!show_path) && (*name) && (name[1] == ':') && (!name[2]))
	{
		everything_plugin_utf8_buf_t volume_name_buf;
		
		everything_plugin_utf8_buf_init(&volume_name_buf);
		
		everything_plugin_os_get_volume_label(name,&volume_name_buf);
		
		http_server_send_escape_html(c,volume_name_buf.buf);
		http_server_send_escape_html(c," (");
		http_server_send_escape_html(c,name);
		http_server_send_escape_html(c,")");
	
		everything_plugin_utf8_buf_kill(&volume_name_buf);
	}
	else
	{
		http_server_send_escape_html(c,name);
	}

	if ((is_folder) || (http_server_allow_file_download))
	{
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_A_END);
	}

	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD_END);

	// path
	if (show_path)
	{
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD);
		http_server_client_send_utf8(c,"pathdata");
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD2);
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR);

		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_A);
		
		if (everything_plugin_utf8_string_is_url_scheme_name_with_double_forward_slash(path))
		{
			http_server_send_escape_html(c,path);
		}
		else
		{
			http_server_client_printf(c,(const everything_plugin_utf8_t *)"/");
			
			{
				everything_plugin_utf8_buf_t link_cbuf;
				
				everything_plugin_utf8_buf_init(&link_cbuf);
				
				http_server_escape_url_filename(&link_cbuf,path);
				
				http_server_send_escape_html(c,link_cbuf.buf);
				
				everything_plugin_utf8_buf_kill(&link_cbuf);
			}
		}

		http_server_send_sort_url_param(c);

		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_A2);

		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR);
		
		http_server_send_escape_html(c,path);
		
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR_END);
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_A_END);
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD_END);
	}
	
	// size
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD);
	http_server_client_send_utf8(c,"sizedata");
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD2);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR);


	if (fd->size != EVERYTHING_PLUGIN_QWORD_MAX)
	{
		http_server_send_size(c,fd->size);
	}

	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD_END);
	
	// date modified
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD);
	http_server_client_send_utf8(c,"modifieddata");
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD2);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR);
	
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR);

	if (fd->date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
	{
		http_server_send_date(c,fd->date_modified);
	}
	
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_NOBR_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TD_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_RESULT_TR_END);
}

static void http_server_html_foot(http_server_client_t *c)
{
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_BODY_END);
}

static uintptr_t http_server_add_page(uintptr_t *pagelist,uintptr_t numpages,uintptr_t pagei)
{
	uintptr_t i;
	
	for(i=0;i<numpages;i++)
	{
		if (pagelist[i] == pagei) 
		{
			return numpages;
		}
	}
	
	pagelist[numpages] = pagei;
	
	return numpages + 1;
}

// search and send results.
static void http_server_search(http_server_client_t *c)
{
	const everything_plugin_property_t *db_sort_column_type;
	int db_sort_ascending;

	// header
	http_server_client_send_header(c);
	
	db_sort_column_type = http_server_get_property_from_sort(c->sort);
	db_sort_ascending = c->sort_ascending;
	
everything_plugin_debug_printf("sort type %d : %d\n",everything_plugin_property_get_type(db_sort_column_type),db_sort_ascending);	
	
	// copy search
	c->db_sort_column_type = db_sort_column_type;
	c->db_sort_ascending = db_sort_ascending;
	
	// insert at the end of the pending query list.
	http_server_insert_query(c);

	if (!_http_server->client_query_active)
	{
		// start a query
		http_server_start_next_query();
	}
}

// server errors SHOULD NOT BE LOCALIZED.
void http_server_error(http_server_client_t *c,int code,const everything_plugin_utf8_t *desc)
{
	http_server_log(c,(const everything_plugin_utf8_t *)"%d %s\r\n",code,desc);

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"HTTP/1.0 %d %s\r\n",code,desc);
	if (code == 401) 
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"WWW-Authenticate: Basic realm=\"");

		http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_EVERYTHING));
		
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"\"\r\n");
	}
	
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Type: text/html; charset=UTF-8\r\n");
	
	if (*http_server_header)
	{
		http_server_client_send_header_list(c,http_server_header);
	}
		
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"\r\n");
	
	// send head
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_ERROR1);
	
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"%d ",code);

	http_server_send_escape_html(c,desc);
				
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_ERROR2);

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"%d ",code);
			
	http_server_send_escape_html(c,desc);

	http_server_client_send_string_id(c,HTTP_SERVER_STRING_ERROR3);

	// shutdown.
	http_server_client_send_shutdown(c);
}

// header Date: Fri, 09 Nov 2007 08:59:34 GMT
// localization is not required.
static void http_server_format_time(everything_plugin_utf8_buf_t *cbuf,SYSTEMTIME *st)
{
	everything_plugin_utf8_t *wday[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	everything_plugin_utf8_t *mon[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	if (((st->wDayOfWeek >= 0) && (st->wDayOfWeek < 7)) && ((st->wMonth > 0) && (st->wMonth <= 12)))
	{
		everything_plugin_utf8_buf_printf(cbuf,(const everything_plugin_utf8_t *)"%s, %02d %s %d %02d:%02d:%02d GMT",wday[st->wDayOfWeek],st->wDay,mon[st->wMonth-1],st->wYear,st->wHour,st->wMinute,st->wSecond);
	}
	else
	{
		everything_plugin_utf8_buf_empty(cbuf);
	}
}

int http_server_send_resource(http_server_client_t *c,const everything_plugin_utf8_t *filename)
{
	int resource_i;
	int ret;

	ret = 0;

	// find the resource by name.	
	for(resource_i = 0;resource_i < HTTP_SERVER_RESOURCE_COUNT;resource_i++)
	{
		if (everything_plugin_utf8_string_compare_nocase_s_sla(filename,http_server_resources[resource_i].filename) == 0)
		{
			http_server_file_header(c,http_server_resources[resource_i].content_type,http_server_resources[resource_i].size,http_server_resources[resource_i].date_modified,1,0,0,0);

			// send in this thread, since the resource should be small..
			http_server_client_send_add(c,http_server_resources[resource_i].data,http_server_resources[resource_i].size);
			
			// shutdown.
			http_server_client_send_shutdown(c);
			
			ret = 1;
			
			// found
			break;
		}
	}

    return ret;
}

static void http_server_file_header(http_server_client_t *c,const everything_plugin_utf8_t *content_type,EVERYTHING_PLUGIN_QWORD size,EVERYTHING_PLUGIN_QWORD date_modified,int cache,int allow_range,EVERYTHING_PLUGIN_QWORD content_range_start,EVERYTHING_PLUGIN_QWORD content_range_size)
{
	SYSTEMTIME st;
	everything_plugin_utf8_buf_t time_cbuf;
	
	everything_plugin_utf8_buf_init(&time_cbuf);
	
	if ((content_range_start) || (content_range_size))
	{
		if (!content_range_size)
		{
			content_range_size = size - content_range_start;
		}
		
		http_server_log(c,(const everything_plugin_utf8_t *)"206 OK\r\n");
		
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"HTTP/1.0 206 OK\r\n");
	}
	else
	{
		http_server_log(c,(const everything_plugin_utf8_t *)"200 OK\r\n");
		
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"HTTP/1.0 200 OK\r\n");
	}
	
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Server: Everything HTTP Server\r\n");

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Type: %s\r\n",content_type);
//	if (filename)
//	{
//		http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Disposition: attachment; filename=\"%s\"\r\n",filename);
//	}

	if ((content_range_start) || (content_range_size))
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Length: %I64u\r\n",content_range_size);
	}
	else
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Length: %I64u\r\n",size);
	}
	
	if (allow_range)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"Accept-Ranges: bytes\r\n");
	}
	
	if ((content_range_start) || (content_range_size))
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Range: bytes %I64u-%I64u/%I64u\r\n",content_range_start,content_range_start+content_range_size-1,size);
	}
	
	// current date.
	GetSystemTime(&st);
	http_server_format_time(&time_cbuf,&st);
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Date: %s\r\n",time_cbuf.buf);

	// last-modified	
	FileTimeToSystemTime((FILETIME *)&date_modified,&st);
	http_server_format_time(&time_cbuf,&st);
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Last-Modified: %s\r\n",time_cbuf.buf);
	
	if (cache)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"Cache-Control: private, max-age=3600\r\n");
	}

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Connection: Close\r\n");
	
	if (*http_server_header)
	{
		http_server_client_send_header_list(c,http_server_header);
	}
	
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"\r\n");

	everything_plugin_utf8_buf_kill(&time_cbuf);
}

static void http_server_client_send_header(http_server_client_t *c)
{
	SYSTEMTIME st;
	everything_plugin_utf8_buf_t time_cbuf;
	
	everything_plugin_utf8_buf_init(&time_cbuf);
	
	http_server_log(c,(const everything_plugin_utf8_t *)"200 OK\r\n");

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"HTTP/1.0 200 OK\r\n");
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Server: Everything HTTP Server\r\n");
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Content-Type: %s\r\n",c->json ? "application/json" : "text/html; charset=UTF-8");
	
	// current date.
	GetSystemTime(&st);
	http_server_format_time(&time_cbuf,&st);
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Date: %s\r\n",time_cbuf.buf);

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Cache-Control: no-cache\r\n");
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"Connection: Close\r\n");
	
	if (*http_server_header)
	{
		http_server_client_send_header_list(c,http_server_header);
	}
	
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"\r\n");

	everything_plugin_utf8_buf_kill(&time_cbuf);
}

static int http_server_send_file2(http_server_client_t *c,const everything_plugin_utf8_t *filename,int iscache)
{
	HANDLE h;

	h = everything_plugin_os_open_file(filename);
	if (h != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		
		GetFileInformationByHandle(h,&bhfi);

		{
			everything_plugin_utf8_buf_t content_type_cbuf;
			everything_plugin_utf8_buf_t class_key_cbuf;
			
			everything_plugin_utf8_buf_init(&content_type_cbuf);
			everything_plugin_utf8_buf_init(&class_key_cbuf);

			everything_plugin_utf8_buf_printf(&class_key_cbuf,(const everything_plugin_utf8_t *)".%s",everything_plugin_utf8_string_get_extension(filename));

			if (!everything_plugin_os_registry_get_string(HKEY_CLASSES_ROOT,class_key_cbuf.buf,(const everything_plugin_utf8_t *)"Content Type",&content_type_cbuf))
			{
				everything_plugin_utf8_buf_copy_utf8_string(&content_type_cbuf,(const everything_plugin_utf8_t *)"application/octet-stream");
			}
		
			http_server_file_header(c,content_type_cbuf.buf,(((EVERYTHING_PLUGIN_QWORD)bhfi.nFileSizeHigh) << 32) | bhfi.nFileSizeLow,*((EVERYTHING_PLUGIN_QWORD *)&bhfi.ftLastWriteTime),iscache,1,c->content_range_start,c->content_range_size);

			everything_plugin_utf8_buf_kill(&class_key_cbuf);
			everything_plugin_utf8_buf_kill(&content_type_cbuf);
		}

		// init data state
		c->data_buffer = everything_plugin_mem_alloc(HTTP_SERVER_DATA_CHUNK_SIZE);
		c->data_file = h;
		InitializeCriticalSection(&c->data_cs);
		c->data_size = 0;
		c->data_remaining = 0;
		c->data_state = 0;
		c->data_abort = 0;
		c->data_complete = 0;
		c->data_hevent = everything_plugin_os_event_create();
		c->data_thread = everything_plugin_os_thread_create(http_server_send_file_thread_proc,c);
		
		http_server_client_send_shutdown(c);

		return 1;
	}
	
	return 0;
}

// send a file
// url must be escaped.
static int http_server_send_file(http_server_client_t *c,const everything_plugin_utf8_t *filename)
{	
	int ret;
	
	ret = 0;

	// try a resource first.
	// ..'s have already been removed.
	{
		everything_plugin_utf8_buf_t full_filename_cbuf;
		everything_plugin_utf8_buf_t path_cbuf;
		everything_plugin_utf8_buf_t win32_file_namespace_cbuf;

		everything_plugin_utf8_buf_init(&full_filename_cbuf);
		everything_plugin_utf8_buf_init(&path_cbuf);
		everything_plugin_utf8_buf_init(&win32_file_namespace_cbuf);

		// dont worry about the instance name, if the user reallys wants a unique http server folder for an instance they 
		// can specify the home themselfs.
		everything_plugin_os_get_app_data_path_cat_filename(*http_server_home ? http_server_home : (const everything_plugin_utf8_t *)"HTTP Server",&path_cbuf);

		everything_plugin_utf8_buf_path_cat_filename(&full_filename_cbuf,path_cbuf.buf,filename);
		
		// DO NOT ALLOW RELATIVE PATHS
		// utf8 => wchar can inject relative paths.
		// "\\?\path" will enforce absolute paths.
		everything_plugin_utf8_string_get_win32_file_namespace(full_filename_cbuf.buf,&win32_file_namespace_cbuf);
		
		// try sending the file first
		// this should always override internal files.
		ret = http_server_send_file2(c,win32_file_namespace_cbuf.buf,1);
		
		if (!ret)
		{
			// try sending as a internal resource.
			ret = http_server_send_resource(c,filename);
		}

		everything_plugin_utf8_buf_kill(&win32_file_namespace_cbuf);
		everything_plugin_utf8_buf_kill(&path_cbuf);
		everything_plugin_utf8_buf_kill(&full_filename_cbuf);
	}

	if (!ret)
	{
		if (http_server_allow_file_download)
		{
			// try sending a real file.
			if (everything_plugin_db_file_exists(_http_server->db,filename))
			{
				ret = http_server_send_file2(c,filename,0);
			}
		}
	}

	return ret;
}

// send a file
int http_server_send_path(http_server_client_t *c,const everything_plugin_utf8_t *filename)
{
	int ret;
	
	ret = 0;
	
	// the path can be the root, or an existing db folder.
	if ((!*filename) || (everything_plugin_db_folder_exists(_http_server->db,filename)))
	{
		everything_plugin_utf8_buf_t name_cbuf;
		everything_plugin_fileinfo_fd_t fd;
		
		everything_plugin_utf8_buf_init(&name_cbuf);

everything_plugin_debug_printf((const everything_plugin_utf8_t *)"send path %s\n",filename);

		http_server_client_send_header(c);
		
		if (c->json)
		{
			everything_plugin_db_find_t *db_find;
			
			http_server_client_printf(c,(const everything_plugin_utf8_t *)"{\r\n");
			http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\"results\":[\r\n");
			
			db_find = everything_plugin_db_find_first_file(_http_server->db,filename,&name_cbuf,&fd);

			if (db_find)
			{
				uintptr_t count;
				
				count = everything_plugin_db_find_get_count(db_find);

				if ((c->sort) || (c->sort_ascending == 0))
				{
					http_server_sort_item_t **sort_items;
					uintptr_t sort_index;
					int (WINAPI *sort_compare_proc)(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
					
					sort_items = everything_plugin_mem_alloc(everything_plugin_safe_uintptr_mul_sizeof_pointer(count));
					sort_index = 0;
				
					// sort results
					for(;;)
					{
						uintptr_t alloc_size;
						
						alloc_size = sizeof(http_server_sort_item_t);
						alloc_size = everything_plugin_safe_uintptr_add(alloc_size,name_cbuf.len);
						alloc_size = everything_plugin_safe_uintptr_add(alloc_size,1);
					
						sort_items[sort_index] = everything_plugin_mem_alloc(alloc_size);
				
						everything_plugin_os_copy_memory(&sort_items[sort_index]->fd,&fd,sizeof(everything_plugin_fileinfo_fd_t));
						sort_items[sort_index]->len = name_cbuf.len;
						sort_items[sort_index]->is_folder = (fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
						everything_plugin_os_copy_memory(sort_items[sort_index] + 1,name_cbuf.buf,name_cbuf.len + 1);
						
						sort_index++;
						
						if (!everything_plugin_db_find_next_file(db_find,&name_cbuf,&fd)) 
						{
							break;
						}
					}
					
					switch(c->sort)
					{
						case HTTP_SERVER_SORT_SIZE: sort_compare_proc = c->sort_ascending ? http_server_compare_size_ascending : http_server_compare_size_descending; break;
						case HTTP_SERVER_SORT_DATE_MODIFIED: sort_compare_proc = c->sort_ascending ? http_server_compare_date_ascending : http_server_compare_date_descending; break;
						default: sort_compare_proc = http_server_compare_name_descending; break;
					}
					
					everything_plugin_os_sort_MT(sort_items,count,sort_compare_proc);

					for(sort_index=0;sort_index < count;sort_index++)
					{
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t{\r\n");
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t\t\"type\":\"%s\",\r\n",sort_items[sort_index]->is_folder ? (const everything_plugin_utf8_t *)"folder" : (const everything_plugin_utf8_t *)"file");
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t\t\"name\":\"");

						http_server_send_escaped_json(c,HTTP_SERVER_SORT_ITEM_FILENAME(sort_items[sort_index]));
						
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"size\":\"");
						if (sort_items[sort_index]->fd.size != EVERYTHING_PLUGIN_QWORD_MAX)
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)"%I64u",sort_items[sort_index]->fd.size);
						}
						
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"date_modified\":\"");
						if (sort_items[sort_index]->fd.date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)"%I64u",sort_items[sort_index]->fd.date_modified);
						}

						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\"\r\n\t\t}");
					
						if (sort_index + 1 < count) 
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)",\r\n");
						}
						else
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)"\r\n");
						}
											
						everything_plugin_mem_free(sort_items[sort_index]);
					}
					
					everything_plugin_mem_free(sort_items);
				}
				else
				{
					for(;;)
					{
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t{\r\n");
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t\t\"type\":\"%s\",\r\n",(fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? (const everything_plugin_utf8_t *)"folder" : (const everything_plugin_utf8_t *)"file");
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t\t\"name\":\"");

						http_server_send_escaped_json(c,name_cbuf.buf);
						
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"size\":\"");
						if (fd.size != EVERYTHING_PLUGIN_QWORD_MAX)
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)"%I64u",fd.size);
						}
						
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"date_modified\":\"");
						if (fd.date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)"%I64u",fd.date_modified);
						}

						http_server_client_printf(c,(const everything_plugin_utf8_t *)"\"\r\n\t\t}");
					
						if (!everything_plugin_db_find_next_file(db_find,&name_cbuf,&fd)) 
						{
							http_server_client_printf(c,(const everything_plugin_utf8_t *)"\r\n");
							
							break;
						}

						http_server_client_printf(c,(const everything_plugin_utf8_t *)",\r\n");
					}
				}
				
				everything_plugin_db_find_close(db_find);
			}
			
			http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t]\r\n");
			http_server_client_printf(c,(const everything_plugin_utf8_t *)"}\r\n");
		}
		else
		{
			everything_plugin_db_find_t *db_find;
			
			http_server_client_send_head(c,filename);
			
			http_server_client_table_begin(c,0);

			// address
			http_server_client_send_string_id(c,HTTP_SERVER_STRING_PATH_INDEX_OF);
			
			http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_INDEX_OF));
			
			http_server_send_escape_html(c,*filename ? filename : (const everything_plugin_utf8_t *)"/");
			
			http_server_client_send_string_id(c,HTTP_SERVER_STRING_PATH_INDEX_OF2);

			// up one directory.
			if (*filename)
			{
				everything_plugin_utf8_buf_t parent_path_cbuf;
				everything_plugin_utf8_buf_t link_cbuf;
				
				everything_plugin_utf8_buf_init(&parent_path_cbuf);
				everything_plugin_utf8_buf_init(&link_cbuf);
				
				everything_plugin_utf8_string_get_path_part(filename,&parent_path_cbuf);
				
				if (!everything_plugin_db_folder_exists(_http_server->db,parent_path_cbuf.buf))
				{
					everything_plugin_utf8_buf_empty(&parent_path_cbuf);
				}

				http_server_escape_url_filename(&link_cbuf,parent_path_cbuf.buf);
			
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_PATH_UPDIR);
				
				http_server_send_escape_html(c,link_cbuf.buf);

				http_server_send_sort_url_param(c);
				
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_PATH_UPDIR2);
				
				http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_UP_ONE_DIRECTORY));
				
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_PATH_UPDIR3);

				everything_plugin_utf8_buf_kill(&link_cbuf);
				everything_plugin_utf8_buf_kill(&parent_path_cbuf);
			}

			// name
			http_server_client_send_table_headers(c,filename,0,c->sort,c->sort_ascending);
			
			db_find = everything_plugin_db_find_first_file(_http_server->db,filename,&name_cbuf,&fd);

			if (db_find)
			{
				uintptr_t count;
				
				count = everything_plugin_db_find_get_count(db_find);

				if ((c->sort) || (c->sort_ascending == 0))
				{
					http_server_sort_item_t **sort_items;
					uintptr_t sort_index;
					int (WINAPI *sort_compare_proc)(const http_server_sort_item_t *a,const http_server_sort_item_t *b);
					
					sort_items = everything_plugin_mem_alloc(everything_plugin_safe_uintptr_mul_sizeof_pointer(count));
					sort_index = 0;
				
					// sort results
					for(;;)
					{
						uintptr_t alloc_size;
						
						alloc_size = sizeof(http_server_sort_item_t);
						alloc_size = everything_plugin_safe_uintptr_add(alloc_size,name_cbuf.len);
						alloc_size = everything_plugin_safe_uintptr_add(alloc_size,1);
					
						sort_items[sort_index] = everything_plugin_mem_alloc(alloc_size);
				
						everything_plugin_os_copy_memory(&sort_items[sort_index]->fd,&fd,sizeof(everything_plugin_fileinfo_fd_t));
						sort_items[sort_index]->len = name_cbuf.len;
						sort_items[sort_index]->is_folder = (fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
						everything_plugin_os_copy_memory(sort_items[sort_index] + 1,name_cbuf.buf,name_cbuf.len + 1);
						
						sort_index++;
						
						if (!everything_plugin_db_find_next_file(db_find,&name_cbuf,&fd)) 
						{
							break;
						}
					}
					
					switch(c->sort)
					{
						case HTTP_SERVER_SORT_SIZE: sort_compare_proc = c->sort_ascending ? http_server_compare_size_ascending : http_server_compare_size_descending; break;
						case HTTP_SERVER_SORT_DATE_MODIFIED: sort_compare_proc = c->sort_ascending ? http_server_compare_date_ascending : http_server_compare_date_descending; break;
						default: sort_compare_proc = http_server_compare_name_descending; break;
					}
					
					everything_plugin_os_sort_MT(sort_items,count,sort_compare_proc);
					
					for(sort_index=0;sort_index < count;sort_index++)
					{
						http_server_html_td(c,(int)(sort_index & 1),filename,HTTP_SERVER_SORT_ITEM_FILENAME(sort_items[sort_index]),sort_items[sort_index]->is_folder,&sort_items[sort_index]->fd,0);

						everything_plugin_mem_free(sort_items[sort_index]);
					}
					
					everything_plugin_mem_free(sort_items);
				}
				else
				{
					uintptr_t index;
					
					index = 0;
					
					for(;;)
					{
						http_server_html_td(c,(int)(index & 1),filename,name_cbuf.buf,(fd.attributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0,&fd,0);
					
						if (!everything_plugin_db_find_next_file(db_find,&name_cbuf,&fd)) 
						{
							break;
						}
						
						index++;
					}
				}
				
				everything_plugin_db_find_close(db_find);
			}
			
			http_server_client_table_end(c);
			
			// footer
			http_server_html_foot(c);
		}
		
		// shutdown.
		http_server_client_send_shutdown(c);

		everything_plugin_utf8_buf_kill(&name_cbuf);
		
		ret = 1;
	}

	return ret;
}

int http_server_check_authorization(const everything_plugin_utf8_t *base64)
{
	int bi;
	int i;
	everything_plugin_utf8_t b;
	int index;
	everything_plugin_utf8_t *p;
	int gotcolon;

	if (base64[0] != 'B') return 0;
	if (base64[1] != 'a') return 0;
	if (base64[2] != 's') return 0;
	if (base64[3] != 'i') return 0;
	if (base64[4] != 'c') return 0;
	if (base64[5] != ' ') return 0;
	
	base64 += 6;
	
	bi = 7;
	b = 0;
	
	gotcolon = 0;
	if (*http_server_username)
	{
		p = http_server_username;
	}
	else
	{
		p = 0;
	}
	
//	everything_plugin_debug_printf((const everything_plugin_utf8_t *)"base64: %s\n",base64);
	
	while(*base64)
	{
		index = everything_plugin_unicode_base64_index(*base64);
		
//everything_plugin_debug_printf((const everything_plugin_utf8_t *)"b64: %d\n",index);
		for(i=0;i<6;i++)
		{
			// start with high bit first
			b |= (((index) >> (5-i)) & 1) << bi;
			
//everything_plugin_debug_printf((const everything_plugin_utf8_t *)"BIT %d\n",((index) >> (5-i)) & 1);
			if (!bi) 
			{
				// check this byte..
//everything_plugin_debug_printf((const everything_plugin_utf8_t *)"%c %c\n",b,*p);
				if (!b) break;

				if (p)
				{
					if (*p)
					{
						if (*p != b) return 0;
						p++;
					}
					else
					{
						if (b != ':') return 0;
						if (gotcolon) return 0;
						gotcolon = 1;
						
						if (*http_server_password)
						{
							p = http_server_password;
						}
						else
						{
							p = 0;
						}					
					}
				}
				else
				{
					if (b == ':') 
					{
						if (gotcolon) return 0;
						gotcolon = 1;
						
						if (*http_server_password)
						{
							p = http_server_password;
						}
						else
						{
							p = 0;
						}
					}
				}

				bi = 7;
				b = 0;
			}
			else
			{
				bi--;
			}
		}
		
		base64++;
	}
	
	// did we get the username ?
	if (!gotcolon) 
	{
		return 0;
	}
	
	// did all the password match ?
	if (p)
	{
		if (*p != 0) 
		{
			return 0;
		}
	}

	return 1;
}

// update a client
static void http_server_client_process_command(http_server_client_t *c,everything_plugin_utf8_t *command)
{
	if (!c->got_header)
	{
		if (everything_plugin_debug_is_verbose())
		{
			everything_plugin_debug_printf((const everything_plugin_utf8_t *)"HEADER %s\r\n",command);
		}
		
		// remove any trailing '\r'
		{
			everything_plugin_utf8_t *p;
			
			p = command;
			
			while(*p)
			{
				if ((*p == '\r') && (!p[1]))
				{
					*p = 0;
					
					break;
				}
				
				p++;
			}
		}

	//everything_plugin_debug_color_printf(0xff0000ff,(const everything_plugin_utf8_t *)"%s\n",command);

		if (*command)
		{
			everything_plugin_utf8_t *param;
			everything_plugin_utf8_t *p;
			
			p = command;
			while(*p)
			{
				if (*p == ' ')
				{
					*p++ = 0;
					
					break;
				}
				
				p++;
			}
			
			param = everything_plugin_utf8_string_skip_ascii_ws(p);
			
			if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"get") == 0)
			{
				p = param;
				
				while(*p)
				{
					if (*p == ' ') 
					{
						*p++ = 0;
						
						break;
					}
					
					p++;
				}
				
				c->get = everything_plugin_utf8_string_realloc_utf8_string(c->get,param);
			}
			else
			if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"authorization:") == 0)
			{
				c->got_authentication = http_server_check_authorization(param);
			}			
			else
			if (everything_plugin_utf8_string_compare_nocase_s_sla(command,(const everything_plugin_utf8_t *)"range:") == 0)
			{
				p = param;
				
				// Content-Range: bytes 500-1233/1234
				everything_plugin_utf8_string_parse_check(&p,"bytes=");
				c->content_range_start = everything_plugin_utf8_string_parse_qword(&p);
				
				if (everything_plugin_utf8_string_parse_check(&p,(const everything_plugin_utf8_t *)"-"))
				{
					if (everything_plugin_unicode_is_digit(*p))
					{
						c->content_range_size = everything_plugin_utf8_string_parse_qword(&p) - c->content_range_start + 1;
					}
				}
everything_plugin_debug_printf("content range start: %I64u, size: %I64u\n",c->content_range_start,c->content_range_size);
			}
		}
		else
		{
			if ((c->get) && (*c->get == '/'))
			{
				http_server_log(c,(const everything_plugin_utf8_t *)"GET %s\r\n",c->get);

				if (((*http_server_username) || (*http_server_password)) && (!c->got_authentication))
				{
					http_server_error(c,401,(const everything_plugin_utf8_t *)"Unauthorized");
				}
				else
				{
					uintptr_t count_specified;
					everything_plugin_utf8_t *p;
					everything_plugin_utf8_t *filename;
					
					count_specified = 0;
					
					p = c->get + 1;
					filename = p;
					
					while(*p)
					{
						if (*p == '#')
						{
							break;
						}
						else
						if (*p == '?')
						{
							// skip ? and terminate last value
							*p++ = 0;
						
							while(*p)
							{
								everything_plugin_utf8_t *key;
								everything_plugin_utf8_t *value;
								
								key = p;
								
								while(*p)
								{
									if (*p == '#')
									{
										break;
									}
									else
									if (*p == '=')
									{
										*p++ = 0;
										
										value = p;
										
										while(*p)
										{
											if (*p == '#') 
											{
												break;
											}
											else
											if (*p == '&') 
											{
												*p++ = 0;
												
												break;
											}
											else
											if (*p == '+')
											{
												// treat any + after the ? as a space.
												*p = ' ';
											}
											
											p++;
										}
										
										// p path, i case, r regex, m diacritics, w wholeword
										
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"s") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"q") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"search") == 0))
										{
											everything_plugin_utf8_buf_t search_cbuf;
											
											everything_plugin_utf8_buf_init(&search_cbuf);
											
											everything_plugin_utf8_buf_copy_utf8_string(&search_cbuf,value);
											

											// convert + to white space
											// we must do this before unescaping the url to preserve any escaped +'s
											{
												everything_plugin_utf8_t *p;
												
												p = search_cbuf.buf;	
												while(*p)
												{
													if (*p == '+')
													{
														*p++ = ' ';
													}
													else
													{
														p++;
													}
												}				
											}
											
											// now unescape the url.
											http_server_unescapeurl(&search_cbuf);
											
											c->search_string = everything_plugin_utf8_string_realloc_utf8_string(c->search_string,search_cbuf.buf);
											
											everything_plugin_utf8_buf_kill(&search_cbuf);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"o") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"offset") == 0))
										{
											c->offset = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"c") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"n") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"count") == 0))
										{
											c->show_max = everything_plugin_utf8_string_to_dword(value);
											count_specified = 1;
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"j") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"json") == 0))
										{
											c->json = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"i") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"case") == 0))
										{
											c->match_case = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"w") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"wholeword") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"whole_word") == 0))
										{
											c->match_whole_word = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"p") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"path") == 0))
										{
											c->match_path = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"r") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"regex") == 0))
										{
											c->match_regex = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if ((everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"m") == 0) || (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"diacritics") == 0))
										{
											c->match_diacritics = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"prefix") == 0)
										{
											c->match_prefix = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"suffix") == 0)
										{
											c->match_suffix = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"ignore_punctuation") == 0)
										{
											c->ignore_punctuation = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"ignore_whitespace") == 0)
										{
											c->ignore_whitespace = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"path_column") == 0)
										{
											c->path_column = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"size_column") == 0)
										{
											c->size_column = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"date_modified_column") == 0)
										{
											c->date_modified_column = everything_plugin_utf8_string_to_dword(value);
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"sort") == 0)
										{
											if (everything_plugin_utf8_string_compare(value,(const everything_plugin_utf8_t *)"size") == 0)
											{
												c->sort = HTTP_SERVER_SORT_SIZE;
											}
											else
											if (everything_plugin_utf8_string_compare(value,(const everything_plugin_utf8_t *)"path") == 0)
											{
												c->sort = HTTP_SERVER_SORT_PATH;
											}
											else
											if (everything_plugin_utf8_string_compare(value,(const everything_plugin_utf8_t *)"date_modified") == 0)
											{
												c->sort = HTTP_SERVER_SORT_DATE_MODIFIED;
											}
											else
											if (everything_plugin_utf8_string_compare(value,(const everything_plugin_utf8_t *)"name") == 0)
											{
												c->sort = HTTP_SERVER_SORT_NAME;
											}
										}
										else
										if (everything_plugin_utf8_string_compare(key,(const everything_plugin_utf8_t *)"ascending") == 0)
										{
											c->sort_ascending = everything_plugin_utf8_string_to_dword(value);
										}
										
										break;
									}
									else
									{
										p++;
									}
								}
							}
							
							break;
						}
						else
						{
							p++;
						}
					}
						
					// dont search if a filename is specified, pass the query strings to filename incase 
					// its a htm with java script.
					if ((!*filename) && (c->search_string))
					{
						if (c->json)
						{
							if (!count_specified)
							{
								c->show_max = 0xffffffff;
							}
						}
						
						http_server_search(c);
					}
					else
					{
						everything_plugin_utf8_buf_t filename_cbuf;
						
						everything_plugin_utf8_buf_init(&filename_cbuf);

						everything_plugin_utf8_buf_copy_utf8_string(&filename_cbuf,filename);

						// escapeurl first
						http_server_unescapeurl(&filename_cbuf);

						// convert '/' to '\\' for canonizcalize.
						{
							everything_plugin_utf8_t *p;
							
							// skip starting '/' 
							
							p = filename_cbuf.buf;	
							
							if (*p == '/')
							{
								p++;
							}				
							
							while(*p)
							{
								if (*p == '/')
								{
									*p++ = '\\';
								}
								else
								{
									p++;
								}
							}				
						}

						// remove ..'s
						everything_plugin_utf8_buf_path_canonicalize(&filename_cbuf);

						if ((!*filename_cbuf.buf) && (*http_server_default_page))
						{
							// root
							// serve default page 
							// if no default page set, serve the volume list below..
							if (!http_server_send_file2(c,http_server_default_page,0))
							{
								http_server_error(c,404,(const everything_plugin_utf8_t *)"Not Found");
							}
						}
						else
						{
							// send file or path..
							if (!http_server_send_file(c,filename_cbuf.buf))
							{
								// remove trailing backslash.
								{
									everything_plugin_utf8_t *p;
									
									p = filename_cbuf.buf;	
									while(*p)
									{
										if ((*p == '\\') && (!p[1]))
										{
											*p = 0;

											break;
										}

										p++;
									}								
								}
							
								if (!http_server_send_path(c,filename_cbuf.buf))
								{
									http_server_error(c,404,(const everything_plugin_utf8_t *)"Not Found");
								}
							}
						}

						everything_plugin_utf8_buf_kill(&filename_cbuf);
					}		
				}
			}
			else
			{
				// get not specified, or bad get
				http_server_error(c,400,(const everything_plugin_utf8_t *)"Bad Request");
			}
			
			c->got_header = 1;
		}
	}
}

// log
void http_server_log(http_server_client_t *c,const everything_plugin_utf8_t *format,...)
{
	if (http_server_logging_enabled)
	{
		everything_plugin_utf8_buf_t message_cbuf;
		
		everything_plugin_utf8_buf_init(&message_cbuf);

		{
			va_list argptr;
			
			va_start(argptr,format);

			everything_plugin_utf8_buf_vprintf(&message_cbuf,format,argptr);
			
			va_end(argptr);
		}

		everything_plugin_debug_color_printf(0xff00ffff,(const everything_plugin_utf8_t *)"%s",message_cbuf.buf);
	
		if (!_http_server->log_file)
		{
			everything_plugin_utf8_buf_t filename_cbuf;

			everything_plugin_utf8_buf_init(&filename_cbuf);

			if (*http_server_log_file_name)
			{
				everything_plugin_os_get_local_app_data_path_cat_filename(http_server_log_file_name,&filename_cbuf);
			}
			else
			{
				everything_plugin_os_get_local_app_data_path_cat_make_filename((const everything_plugin_utf8_t *)"Logs\\HTTP_Server_Log",(const everything_plugin_utf8_t *)".txt",&filename_cbuf);
			}
			
			everything_plugin_os_resize_file(filename_cbuf.buf,http_server_log_max_size,http_server_log_delta_size);
			
			everything_plugin_os_make_sure_path_to_file_exists(filename_cbuf.buf);

			_http_server->log_file = everything_plugin_output_stream_append_file(filename_cbuf.buf);

			everything_plugin_utf8_buf_kill(&filename_cbuf);
		}
		
		if (_http_server->log_file)
		{
			EVERYTHING_PLUGIN_QWORD ft;
			
			// write date time
			ft = everything_plugin_os_get_system_time_as_file_time();

			{
				everything_plugin_utf8_buf_t date_time_cbuf;
				everything_plugin_utf8_buf_t version_cbuf;
				
				everything_plugin_utf8_buf_init(&version_cbuf);
				everything_plugin_utf8_buf_init(&date_time_cbuf);
			
				// get version	
				everything_plugin_version_get_text(&version_cbuf);

				// init_format_system_time?
				everything_plugin_utf8_buf_format_filetime(&date_time_cbuf,ft);
				
				everything_plugin_output_stream_write_printf(_http_server->log_file,(const everything_plugin_utf8_t *)"%s: ",date_time_cbuf.buf);
				
				// write to file.
				if (c)
				{
					everything_plugin_utf8_buf_t sockaddr_cbuf;

					everything_plugin_utf8_buf_init(&sockaddr_cbuf);

					everything_plugin_utf8_buf_format_peername(&sockaddr_cbuf,c->socket_handle);
					
					everything_plugin_output_stream_write_printf(_http_server->log_file,(const everything_plugin_utf8_t *)"%p: ",c->socket_handle);
					
					everything_plugin_output_stream_write_printf(_http_server->log_file,(const everything_plugin_utf8_t *)"%s: ",sockaddr_cbuf.buf);
					
					everything_plugin_utf8_buf_kill(&sockaddr_cbuf);
				}
				
				everything_plugin_output_stream_write_printf(_http_server->log_file,(const everything_plugin_utf8_t *)"%s",message_cbuf.buf);

				everything_plugin_utf8_buf_kill(&date_time_cbuf);
				everything_plugin_utf8_buf_kill(&version_cbuf);
			}
		}
		
		everything_plugin_utf8_buf_kill(&message_cbuf);
	}
}

static everything_plugin_utf8_t *http_server_get_escape_url(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *str,int escape_forward_slashes)
{
	const everything_plugin_utf8_t *p;
	everything_plugin_utf8_t *d;
	
	p = str;
	d = buf;
	
	// escape the first '/'
	// but only if escape_forward_slashes == 0 (escaping a filename)
	if (!escape_forward_slashes)
	{
		if (*p == '/')
		{
			// escape it.
			if (buf) 
			{
				*d++ = '%';
				*d++ = everything_plugin_unicode_hex_char(*p / 16);
				*d++ = everything_plugin_unicode_hex_char(*p & 15);
			}
			else
			{
				d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,3);
			}
			
			p++;
		}
	}

	// rfc3986
	
	while(*p)
	{
		if (http_server_is_valid_url_char(*p,escape_forward_slashes))
		{
			// just copy it.
			if (buf) 
			{
				*d++ = *p;
			}
			else
			{
				d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,1);
			}
		}
		else
		{
			// escape it.
			if (buf) 
			{
				*d++ = '%';
				*d++ = everything_plugin_unicode_hex_char(*p / 16);
				*d++ = everything_plugin_unicode_hex_char(*p & 15);
			}
			else
			{
				d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,3);
			}
		}

		p++;
	}
	
	if (buf)
	{
		*d = 0;
	}
	
	return d;
}

static int http_server_is_valid_url_char(everything_plugin_utf8_t c,int escape_forward_slashes)
{
	if ((c >= 'a') && (c <= 'z')) 
	{
		return 1;
	}

	if ((c >= 'A') && (c <= 'Z') )
	{
		return 1;
	}
	
	if ((c >= '0') && (c <= '9')) 
	{
		return 1;
	}
	
	// non-ascii
	// non-ascii chars worked fine on firefox.
	if (c >= 128)
	{
		return 1;
	}

	if (!escape_forward_slashes)
	{
		if (c == '/')
		{
			return 1;
		}
	}

	// rfc3986
	switch(c)	
	{
		case '-':
		case '_':
		case '.':
		case '!':
		case '~':
		case '*':
		case '\'':
		case '(':
		case ')':

			return 1;
	}
	
	return 0;
}

// send a message to the client
static void http_server_escape_url(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *str,int escape_forward_slashes)
{	
	everything_plugin_utf8_buf_grow_length(cbuf,(uintptr_t)http_server_get_escape_url(0,str,escape_forward_slashes));
	
	http_server_get_escape_url(cbuf->buf,str,escape_forward_slashes);
}

// send a message to the client
static void http_server_escape_url_filename(everything_plugin_utf8_buf_t *cbuf,const everything_plugin_utf8_t *filename)
{	
	everything_plugin_utf8_buf_t forward_slash_filename_cbuf;
	
	everything_plugin_utf8_buf_init(&forward_slash_filename_cbuf);

	everything_plugin_utf8_buf_copy_utf8_string(&forward_slash_filename_cbuf,filename);

	// convert to forward slashes
	{
		everything_plugin_utf8_t *p;
		
		p = forward_slash_filename_cbuf.buf;
		
		// skip the \\ in a \\server\share
		// this will allow \\server/share to work.
		// since a double // is treated as ".."
		// IE is converting '\\' back to '/'....
		while(*p)
		{
			if (*p != '\\')
			{
				break;
			}
			
			p++;
		}
		
		while(*p)
		{
			if (*p == '\\')
			{
				*p = '/';
			}
			p++;
		}
	}
	
	http_server_escape_url(cbuf,forward_slash_filename_cbuf.buf,0);

	everything_plugin_utf8_buf_kill(&forward_slash_filename_cbuf);
}

static void http_server_send_escape_html(http_server_client_t *c,const everything_plugin_utf8_t *str)
{
	everything_plugin_utf8_buf_t escape_html_cbuf;
	
	everything_plugin_utf8_buf_init(&escape_html_cbuf);
	
	everything_plugin_utf8_buf_escape_html(&escape_html_cbuf,str);
	
	http_server_client_send_add(c,escape_html_cbuf.buf,escape_html_cbuf.len);
	
	everything_plugin_utf8_buf_kill(&escape_html_cbuf);
}

static void http_server_send_size(http_server_client_t *c,EVERYTHING_PLUGIN_QWORD size)
{
	everything_plugin_utf8_buf_t size_cbuf;
	
	everything_plugin_utf8_buf_init(&size_cbuf);
	
	everything_plugin_utf8_buf_format_size(&size_cbuf,size);
	
	http_server_send_escape_html(c,size_cbuf.buf);
	
	everything_plugin_utf8_buf_kill(&size_cbuf);
}

static void http_server_send_date(http_server_client_t *c,EVERYTHING_PLUGIN_QWORD date)
{
	everything_plugin_utf8_buf_t date_cbuf;
	SYSTEMTIME localst;
	
	everything_plugin_utf8_buf_init(&date_cbuf);
	
	everything_plugin_os_filetime_to_localtime(&localst,date);
	
	everything_plugin_utf8_buf_format_filetime(&date_cbuf,date);
	
	http_server_send_escape_html(c,date_cbuf.buf);
	
	everything_plugin_utf8_buf_kill(&date_cbuf);
}

static everything_plugin_utf8_t *http_server_escape_json(everything_plugin_utf8_t *buf,const everything_plugin_utf8_t *s)
{
	const everything_plugin_utf8_t *p;
	everything_plugin_utf8_t *d;
	
	p = s;
	d = buf;
	
	while(*p)
	{
		if (*p == '\\')
		{
			if (buf)
			{
				*d++ = '\\';
				*d++ = '\\';
			}
			else
			{
				d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,2);
			}
		}
		else
		if (*p == '"')
		{
			if (buf)
			{
				*d++ = '\\';
				*d++ = '"';
			}
			else
			{
				d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,2);
			}
		}
		else
		{
			if (*p < 32)
			{
				// escape it..
				if (buf)
				{
					*d++ = '\\';
					*d++ = 'u';
					*d++ = '0';
					*d++ = '0';
					*d++ = everything_plugin_unicode_hex_char((*p >> 4) & 15);
					*d++ = everything_plugin_unicode_hex_char((*p >> 0) & 15);
				}
				else
				{
					d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,6);
				}
				
			}
			else
			{
				if (buf)
				{
					*d++ = *p;
				}
				else
				{
					d = (void *)everything_plugin_safe_uintptr_add((uintptr_t)d,1);
				}
			}
		}

		p++;
	}
	
	return d;
}

static void http_server_send_escaped_json(http_server_client_t *c,const everything_plugin_utf8_t *text)
{	
	everything_plugin_utf8_buf_t cbuf;	
	
	everything_plugin_utf8_buf_init(&cbuf);
	
	everything_plugin_utf8_buf_grow_length(&cbuf,(uintptr_t)http_server_escape_json(0,text));
	
	http_server_escape_json(cbuf.buf,text);
	
	http_server_client_send_add(c,cbuf.buf,cbuf.len);
	
	everything_plugin_utf8_buf_kill(&cbuf);
}

// get the localhost everything_plugin_os_winsock_addrinfo
// save stack from main too.
static int http_server_get_bind_addrinfo(const everything_plugin_utf8_t *nodename,struct everything_plugin_os_winsock_addrinfo **ai)
{
	struct everything_plugin_os_winsock_addrinfo hints;
	everything_plugin_utf8_buf_t port_cbuf;
	int ret;
	
	everything_plugin_utf8_buf_init(&port_cbuf);
	ret = 0;

	// Fill out the local socket address data.
	everything_plugin_os_zero_memory(&hints,sizeof(struct everything_plugin_os_winsock_addrinfo));
	hints.ai_protocol = EVERYTHING_PLUGIN_OS_WINSOCK_IPPROTO_TCP;
	hints.ai_socktype = EVERYTHING_PLUGIN_OS_WINSOCK_SOCK_STREAM;
	hints.ai_flags = EVERYTHING_PLUGIN_OS_WINSOCK_AI_PASSIVE;	

	everything_plugin_utf8_buf_printf(&port_cbuf,(const everything_plugin_utf8_t *)"%d",_http_server->port);

	if (everything_plugin_os_winsock_getaddrinfo((const everything_plugin_utf8_t *)nodename,(const everything_plugin_utf8_t *)port_cbuf.buf,&hints,ai) == 0)
	{
		ret = 1;
	}

	everything_plugin_utf8_buf_kill(&port_cbuf);
	
	return ret;
}

static int http_server_add_binding(everything_plugin_utf8_buf_t *error_cbuf,const everything_plugin_utf8_t *nodename)
{
	struct everything_plugin_os_winsock_addrinfo *ai;
	int ret;
	
	ret = 0;
		
	if (http_server_get_bind_addrinfo(nodename,&ai))
	{
		struct everything_plugin_os_winsock_addrinfo *aip;
		
		aip = ai;
		
		while(aip)
		{
			// ipv4 or ipv6 please.
			if ((aip->ai_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET) || (aip->ai_family == EVERYTHING_PLUGIN_OS_WINSOCK_AF_INET6))
			{
				EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET listen_socket;
				
				// reset ret, as a previous bind would have set it to 1.
				ret = 0;
				
	//DBEUG:
	everything_plugin_debug_printf((const everything_plugin_utf8_t *)"bind to family %d, protocol %d, socktype %d\n",aip->ai_family,aip->ai_protocol,aip->ai_socktype);

				listen_socket = everything_plugin_os_winsock_socket(aip->ai_family,aip->ai_socktype,aip->ai_protocol);
				if (listen_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
				{
					if (everything_plugin_os_winsock_bind(listen_socket,aip->ai_addr,(int)aip->ai_addrlen) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
					{
						if (everything_plugin_os_winsock_listen(listen_socket,EVERYTHING_PLUGIN_OS_WINSOCK_SOMAXCONN) != EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
						{
							http_server_listen_t *listen;
							
							// alloc
							listen = everything_plugin_mem_alloc(sizeof(http_server_listen_t));
							
							// init
							listen->listen_socket = listen_socket;
							
							// insert
							if (_http_server->listen_start)
							{
								_http_server->listen_last->next = listen;
							}
							else
							{
								_http_server->listen_start = listen;
							}
							
							listen->next = 0;
							_http_server->listen_last = listen;
							
							// select
							everything_plugin_os_winsock_WSAAsyncSelect(listen_socket,_http_server->hwnd,HTTP_SERVER_WM_LISTEN,EVERYTHING_PLUGIN_OS_WINSOCK_FD_ACCEPT|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);
							
							ret = 1;

							goto next_ai;
						}
						else
						{
							everything_plugin_utf8_buf_printf(error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_LISTEN_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
						}
					}
					else
					{
						everything_plugin_utf8_buf_printf(error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_BIND_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
					}

					everything_plugin_os_winsock_closesocket(listen_socket);
				}
				else
				{
					everything_plugin_utf8_buf_printf(error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_SOCKET_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
				}

				if (!ret)
				{
					break;
				}
			}			
			
next_ai:

			
			aip = aip->ai_next;
		}
		
		everything_plugin_os_winsock_freeaddrinfo(ai);
	}
	
	return ret;
}

// host
static void http_server_start(void)
{
	everything_plugin_utf8_buf_t error_cbuf;
	
	everything_plugin_utf8_buf_init(&error_cbuf);
	
	if (!_http_server)
	{
		EVERYTHING_PLUGIN_OS_WINSOCK_WSADATA wsadata;
		int wsaret;

		// please use version 1.1 to be compatible with other plugins.
		wsaret = everything_plugin_os_winsock_WSAStartup(MAKEWORD(1,1),&wsadata);

		if (wsaret == 0)
		{
			// make the server now
			// the server holds the ref to WSAStartup.


			// alloc
			_http_server = everything_plugin_mem_calloc(sizeof(http_server_t));

			// init strings.
			{
				uintptr_t i;
				
				for(i=0;i<HTTP_SERVER_STRING_COUNT;i++)
				{
					_http_server->strings[i] = http_server_strings[i];
				}
			}
			
			// load strings
			if (*http_server_strings_filename)
			{
				everything_plugin_utf8_basic_string_t *file_basic_string;
				
				file_basic_string = everything_plugin_utf8_basic_string_get_text_plain_file(http_server_strings_filename);
				
				if (file_basic_string)
				{
					everything_plugin_ini_t *ini;
					uintptr_t i;
					
					ini = everything_plugin_ini_open(EVERYTHING_PLUGIN_UTF8_BASIC_STRING_TEXT(file_basic_string),"http_server_strings");
					if (ini)
					{
						for(i=0;i<HTTP_SERVER_STRING_COUNT;i++)
						{
							everything_plugin_utf8_const_string_t value_string;
							
							if (everything_plugin_ini_find_keyvalue(ini,http_server_string_names[i],&value_string))
							{
								_http_server->strings[i] = everything_plugin_utf8_string_alloc_utf8_string_n(value_string.text,value_string.len);
							}
						}
						
						everything_plugin_ini_close(ini);
					}
					
					everything_plugin_utf8_basic_string_free(file_basic_string);
				}
			}		
			
			// running
			_http_server->port = http_server_port;
			_http_server->bindings = everything_plugin_utf8_string_alloc_utf8_string(http_server_bindings);

			_http_server->db = everything_plugin_db_add_local_ref();
			_http_server->q = everything_plugin_db_query_create(_http_server->db,http_server_db_query_event_proc,0);

			everything_plugin_os_register_class(0,(const everything_plugin_utf8_t *)"EVERYTHING_HTTP_SERVER",http_server_window_proc,0,0,0,0);

			_http_server->hwnd = everything_plugin_os_create_window(
				0,
				(const everything_plugin_utf8_t *)"EVERYTHING_HTTP_SERVER",
				(const everything_plugin_utf8_t *)"EVERYTHING_HTTP_SERVER",
				0,0,0,0,0,0,0,GetModuleHandle(0),0);

			if ((LOBYTE(wsadata.wVersion) == 1) || (HIBYTE(wsadata.wVersion) == 1))
			{
				// parse the list of bindings.
				if ((*http_server_bindings) && (everything_plugin_utf8_string_compare(http_server_bindings,"*") != 0))
				{
					const everything_plugin_utf8_t *bindp;
					everything_plugin_utf8_buf_t bind_cbuf;
					
					everything_plugin_utf8_buf_init(&bind_cbuf);
					
					bindp = http_server_bindings;
				
					for(;;)
					{
						bindp = everything_plugin_utf8_string_parse_csv_item(bindp,&bind_cbuf);
						if (!bindp)
						{
							break;
						}
						
						if (*bind_cbuf.buf)
						{
							if (!http_server_add_binding(&error_cbuf,bind_cbuf.buf))
							{
								break;
							}
						}
					}

					everything_plugin_utf8_buf_kill(&bind_cbuf);
				}
				else
				{
					http_server_add_binding(&error_cbuf,0);
				}
			}
			else
			{
				everything_plugin_utf8_buf_printf(&error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_BIND_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
			}
						
			// did we bind to anything?
			if (_http_server->listen_start)
			{
				http_server_log(0,(const everything_plugin_utf8_t *)"HTTP server online.\r\n");
	
				// started.
				goto exit;
			}
			
			http_server_shutdown();
		}
		else
		{
			everything_plugin_utf8_buf_printf(&error_cbuf,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_WSASTARTUP_FAILED_FORMAT),everything_plugin_os_winsock_WSAGetLastError());
		}
	}

	if (*error_cbuf.buf)
	{
		everything_plugin_ui_task_dialog_show(NULL,MB_OK|MB_ICONERROR,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_EVERYTHING),NULL,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_ERROR_FORMAT),error_cbuf.buf);
	}

exit:

	everything_plugin_utf8_buf_kill(&error_cbuf);
}

// disconect
static void http_server_shutdown(void)
{
	if (_http_server) 
	{
		// destroy listening sockets.
		{
			http_server_listen_t *l;
			http_server_listen_t *next_l;

			l = _http_server->listen_start;
			while(l)			
			{
				next_l = l->next;
				
				everything_plugin_os_winsock_closesocket(l->listen_socket);
				everything_plugin_mem_free(l);
				
				l = next_l;
			}
		}
		
		// delete the clients.
		{
			http_server_client_t *c;
			http_server_client_t *next_c;
		
			c = _http_server->client_start;
			while(c)
			{
				next_c = c->next;

				http_server_client_destroy(c);
				
				c = next_c;
			}
		}
		
		everything_plugin_mem_free(_http_server->bindings);
		
		everything_plugin_db_query_destroy(_http_server->q);
		everything_plugin_db_release(_http_server->db);
		
		// deref
		DestroyWindow(_http_server->hwnd);

		if (_http_server->current_search_string)
		{
			everything_plugin_mem_free(_http_server->current_search_string);
		}
		
		// free strings
		{
			uintptr_t i;
			
			for(i=0;i<HTTP_SERVER_STRING_COUNT;i++)
			{
				if (_http_server->strings[i] != http_server_strings[i])
				{
					everything_plugin_mem_free((everything_plugin_utf8_t *)_http_server->strings[i]);
				}
			}
		}
		
		http_server_log(0,(const everything_plugin_utf8_t *)"HTTP server offline.\r\n");
		
		if (_http_server->log_file)
		{
			everything_plugin_output_stream_close(_http_server->log_file);
		}
		
		everything_plugin_mem_free(_http_server);

		_http_server = 0;

		everything_plugin_os_winsock_WSACleanup();
	}
}

static int http_server_is_config_change(void)
{
	if (_http_server)
	{
		if (http_server_port != _http_server->port)
		{
			return 1;
		}

		if (everything_plugin_utf8_string_compare(http_server_bindings,_http_server->bindings) != 0)
		{
			return 1;
		}
		
//DEBUG_FIXME("check for username/password change");
	}
	
	return 0;
}

static int http_server_apply_settings(void)
{
	if (http_server_enabled)
	{
		if (http_server_is_config_change())
		{
			http_server_shutdown();
		}

		http_server_start();
		
		if (_http_server)
		{
			return 1;
		}
	}
	else
	{
		http_server_shutdown();
		
		return 1;
	}
	
	return 0;
}

static void http_server_send_query_results(http_server_client_t *c)
{
	uintptr_t totresults;
	uintptr_t numresults;
	everything_plugin_utf8_buf_t path_cbuf;
	everything_plugin_utf8_buf_t name_cbuf;
	
	everything_plugin_utf8_buf_init(&name_cbuf);
	everything_plugin_utf8_buf_init(&path_cbuf);

	totresults = everything_plugin_db_query_get_result_count(_http_server->q);
	
	if (c->offset > totresults)
	{
		numresults = 0;
	}
	else
	{
		numresults = totresults - c->offset;

		if (numresults > c->show_max)
		{
			numresults = c->show_max;
		}
	}
	
	if (c->json)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"{\r\n");
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\"totalResults\":%zu,\r\n",totresults);
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\"results\":[\r\n");

		{
			uintptr_t index;
			uintptr_t count;

			index = c->offset;
			count = numresults;
			while(count)
			{
				everything_plugin_fileinfo_fd_t fd;
				
				everything_plugin_db_query_get_result_name(_http_server->q,index,&name_cbuf);
				
				if (c->path_column)
				{
					everything_plugin_db_query_get_result_path(_http_server->q,index,&path_cbuf);
				}
				
				if ((c->size_column) || (c->date_modified_column))
				{
					everything_plugin_db_query_get_result_indexed_fd(_http_server->q,index,&fd);
				}
				
				http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t{\r\n");
				http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t\t\"type\":\"%s\",\r\n",everything_plugin_db_query_is_folder_result(_http_server->q,index) ? "folder" : "file");
				http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t\t\t\"name\":\"");

				http_server_send_escaped_json(c,name_cbuf.buf);
				
				if (c->path_column)
				{
					http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"path\":\"");

					http_server_send_escaped_json(c,path_cbuf.buf);
				}
				
				if (c->size_column)
				{
					http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"size\":\"");
					if (fd.size != EVERYTHING_PLUGIN_QWORD_MAX)
					{
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"%I64u",fd.size);
					}
				}
				
				if (c->date_modified_column)
				{
					http_server_client_printf(c,(const everything_plugin_utf8_t *)"\",\r\n\t\t\t\"date_modified\":\"");
					if (fd.date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
					{
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"%I64u",fd.date_modified);
					}
				}
		
				http_server_client_printf(c,(const everything_plugin_utf8_t *)"\"\r\n\t\t}%s\r\n",(count > 1) ? "," : "");
				
				count--;
				index++;
			}
		}

		http_server_client_printf(c,(const everything_plugin_utf8_t *)"\t]\r\n");
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"}\r\n");
	}
	else
	{
		http_server_client_send_head(c,c->search_string);
		
		http_server_client_table_begin(c,c->search_string);

		// number of results
		{
			everything_plugin_utf8_buf_t tot_cbuf;
			
			everything_plugin_utf8_buf_init(&tot_cbuf);
			
			everything_plugin_utf8_buf_format_qword(&tot_cbuf,totresults);
			
			http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NUMRESULTS);

			if (totresults == 1)
			{
				http_server_client_printf(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_ONE_RESULT_FORMAT),tot_cbuf.buf);
			}
			else
			if ((numresults != totresults) && (numresults))
			{
				everything_plugin_utf8_buf_t offset_cbuf;
				everything_plugin_utf8_buf_t num_cbuf;

				everything_plugin_utf8_buf_init(&offset_cbuf);
				everything_plugin_utf8_buf_init(&num_cbuf);

				everything_plugin_utf8_buf_format_qword(&offset_cbuf,c->offset + 1);
				everything_plugin_utf8_buf_format_qword(&num_cbuf,c->offset + numresults);
				
				http_server_client_printf(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_X_TO_Y_OF_Z_RESULTS_FORMAT),offset_cbuf.buf,num_cbuf.buf,tot_cbuf.buf);

				everything_plugin_utf8_buf_kill(&num_cbuf);
				everything_plugin_utf8_buf_kill(&offset_cbuf);
			}
			else
			{
				http_server_client_printf(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_X_RESULTS_FORMAT),tot_cbuf.buf);
			}
			
			http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NUMRESULTS2);

			everything_plugin_utf8_buf_kill(&tot_cbuf);
		}

		http_server_client_send_table_headers(c,0,c->search_string,c->sort,c->sort_ascending);

		{
			uintptr_t index;
			uintptr_t count;
			
			index = c->offset;
			count = numresults;
			
			while(count)
			{
				everything_plugin_fileinfo_fd_t fd;
				
				everything_plugin_db_query_get_result_name(_http_server->q,index,&name_cbuf);
				everything_plugin_db_query_get_result_path(_http_server->q,index,&path_cbuf);
				everything_plugin_db_query_get_result_indexed_fd(_http_server->q,index,&fd);
		
				http_server_html_td(c,(int)((index - c->offset) & 1),path_cbuf.buf,name_cbuf.buf,everything_plugin_db_query_is_folder_result(_http_server->q,index),&fd,1);

				count--;
				index++;
			}
		}

		// end html			
		http_server_client_table_end(c);

		if (c->show_max)
		{
			if ((c->offset) || (totresults > c->show_max))
			{
				uintptr_t pagelist[10];
				uintptr_t i;
				uintptr_t pagecount;
				uintptr_t totpages;
				
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_PAGE);
				
				//
				totpages = (everything_plugin_db_query_get_result_count(_http_server->q) + (c->show_max - 1)) / c->show_max;
				if (totpages <= 10)
				{
					
					// display all pages
					for(i=0;i<totpages;i++)
					{
						pagelist[i] = i;
					}
					
					pagecount = totpages;
				}
				else
				{
					uintptr_t current_page;
					
					current_page = ((c->offset + (c->show_max - 1)) / c->show_max);
					
					// display 1st, prev, this, next and last
					
					pagecount = 0;
					
					pagecount = http_server_add_page(pagelist,pagecount,0);
//								pagecount = http_server_add_page(pagelist,pagecount,1);

					if ((current_page > 0) && (current_page - 1 >= 0) && (current_page - 1 < totpages))
					{
						pagecount = http_server_add_page(pagelist,pagecount,current_page-1);
					}
					
					if (current_page < totpages)
					{
						pagecount = http_server_add_page(pagelist,pagecount,current_page);
					}
					
					if (current_page + 1 < totpages)
					{
						pagecount = http_server_add_page(pagelist,pagecount,current_page+1);
					}

					// totpages is > 10 (we check above.)
//								pagecount = http_server_add_page(pagelist,pagecount,totpages-2);
					pagecount = http_server_add_page(pagelist,pagecount,totpages-1);
				}

				if (c->offset)
				{
					http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_PREV);

					http_server_client_send_search_link(c,c->search_string,c->sort,c->sort_ascending,c->offset - c->show_max,c->show_max);

					http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_PREV2);
					
					http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_PREVIOUS));
					
					http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_PREV3);
				}

				for(i=0;i<pagecount;i++)
				{
					if (i)
					{
						if (pagelist[i] != pagelist[i-1] + 1)
						{
							http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NAV);
						}
					}
					
					if (pagelist[i] * c->show_max == c->offset)
					{
						http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_CUR);
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"%zu",pagelist[i]+1);
						http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_CUR2);
					}
					else
					{
						http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NUM);
						
						http_server_client_send_search_link(c,c->search_string,c->sort,c->sort_ascending,pagelist[i] * c->show_max,c->show_max);
						
						http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NUM2);
						http_server_client_printf(c,(const everything_plugin_utf8_t *)"%zu",pagelist[i]+1);
						http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NUM3);
						
					}
				}

				if (everything_plugin_db_query_get_result_count(_http_server->q) - c->offset > c->show_max)
				{
					http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NEXT);

					http_server_client_send_search_link(c,c->search_string,c->sort,c->sort_ascending,c->offset + c->show_max,c->show_max);
					
					http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NEXT2);
					
					http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_HTTP_SERVER_NEXT));

					http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_NEXT3);
				}

				http_server_client_send_string_id(c,HTTP_SERVER_STRING_QUERY_PAGE_END);
			}
		}

		http_server_html_foot(c);
	}
	
	// flush
	http_server_client_send_shutdown(c);
	
	everything_plugin_utf8_buf_kill(&path_cbuf);
	everything_plugin_utf8_buf_kill(&name_cbuf);
}

static void EVERYTHING_PLUGIN_API http_server_db_query_event_proc(void *user_data,int type)
{
	switch(type)
	{
		case EVERYTHING_PLUGIN_DB_QUERY_EVENT_SORT_COMPLETE:
		case EVERYTHING_PLUGIN_DB_QUERY_EVENT_QUERY_COMPLETE:
		{
everything_plugin_debug_printf((const everything_plugin_utf8_t *)"results changed\n")		;
			if (_http_server->client_query_active)
			{
				http_server_send_query_results(_http_server->client_query_active);
				
				_http_server->client_query_active = 0;
			}
			
			// post new query.
			http_server_start_next_query();
				
			break;
		}
	}
}

static LRESULT WINAPI http_server_window_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		// update listening sockets.
		case HTTP_SERVER_WM_LISTEN:
		{
			http_server_listen_t *l;
			
			l = _http_server->listen_start;
			while(l)
			{
				EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET client_socket;
				
				// accept could fail with WSAEWOULDBLOCK because EVERYTHING_PLUGIN_OS_WINSOCK_FD_ACCEPT could be an old post.
				client_socket = everything_plugin_os_winsock_accept(l->listen_socket,0,0);
				if (client_socket != EVERYTHING_PLUGIN_OS_WINSOCK_INVALID_SOCKET)
				{
					everything_plugin_network_set_tcp_nodelay(client_socket);
					everything_plugin_network_set_keepalive(client_socket);
												
					everything_plugin_os_winsock_WSAAsyncSelect(client_socket,hwnd,HTTP_SERVER_WM_CLIENT,EVERYTHING_PLUGIN_OS_WINSOCK_FD_READ|EVERYTHING_PLUGIN_OS_WINSOCK_FD_WRITE|EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE);

					http_server_client_create(client_socket);
				}
				
				l = l->next;
			}

			break;
		}
			
		// update client socket
		case HTTP_SERVER_WM_CLIENT:
		{
			http_server_client_t *c;

			c = http_server_client_find((EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET)wParam);
			if (c)
			{
				// try to read.
				// if it fails we know the socket has really been closed.
				// we HAVE to do this because EVERYTHING_PLUGIN_OS_WINSOCK_FD_CLOSE could be posted 
				// to an old socket, that happens to be the same as this one.
				if (!http_server_client_update_recv(c))
				{
					http_server_client_destroy(c);
					
					break;
				}

				if (!http_server_client_update_send(c))
				{
					http_server_client_destroy(c);
					
					break;
				}
			}
			
			break;
		}			
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

static http_server_client_t *http_server_client_find(EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET socket_handle)
{
	http_server_client_t *c;
	
	c = _http_server->client_start;
	
	while(c)
	{
		if (c->socket_handle == socket_handle)
		{
			return c;
		}
	
		c = c->next;
	}
	
	return 0;
}

static int http_server_client_update_recv(http_server_client_t *c)
{
	for(;;)
	{
		int ret;
		
		// create a recv packet buffer..
		if (c->recv_front == c->recv_end)
		{
			http_server_recv_chunk_add(c);
		}
		
		ret = everything_plugin_network_recv(c->socket_handle,c->recv_front,(c->recv_end - c->recv_front));

		if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
		{
			if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}

			return 0;
		}
		else
		if (ret == 0)
		{
			// socket closed.
			return 0;
		}
		else
		{
			everything_plugin_utf8_t *start;
			everything_plugin_utf8_t *p;
			uintptr_t run;
			
			start = 0;
			p = (everything_plugin_utf8_t *)c->recv_front;
			run = ret;
			
			c->recv_front += ret;

			// did we read a newline?
			while(run)
			{
				if (*p == '\n')
				{
					*p = 0;
					
					if (start)
					{
						// another command in the same recv.
						http_server_client_process_command(c,start);
					}
					else
					{
						if (c->recv_chunk_count == 1)
						{
							// a single packet.
							http_server_client_process_command(c,HTTP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last));
						}
						else
						{
							everything_plugin_utf8_t *linear_buf;
							everything_plugin_utf8_t *d;
							http_server_recv_chunk_t *recv_chunk;
							uintptr_t alloc_size;
							
							alloc_size = c->recv_chunk_count - 1;
							
							// calc size
							recv_chunk = c->recv_chunk_start;
							while(recv_chunk != c->recv_chunk_last)
							{
								alloc_size = everything_plugin_safe_uintptr_add(alloc_size,HTTP_SERVER_RECV_CHUNK_SIZE - sizeof(http_server_recv_chunk_t));
								
								recv_chunk = recv_chunk->next;
							}
							
							alloc_size = everything_plugin_safe_uintptr_add(alloc_size,p - HTTP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last));
							alloc_size = everything_plugin_safe_uintptr_add(alloc_size,1);

							// we MUST allocate a linear buffer
							linear_buf = everything_plugin_mem_alloc(alloc_size);
							
							d = linear_buf;
							
							// copy full chunks
							recv_chunk = c->recv_chunk_start;
							while(recv_chunk != c->recv_chunk_last)
							{
								http_server_recv_chunk_t *next_recv_chunk;
								
								next_recv_chunk = recv_chunk->next;
								
								everything_plugin_os_copy_memory(d,recv_chunk+1,HTTP_SERVER_RECV_CHUNK_SIZE - sizeof(http_server_recv_chunk_t));
								
								d += HTTP_SERVER_RECV_CHUNK_SIZE - sizeof(http_server_recv_chunk_t);
								
								http_server_recv_chunk_destroy(recv_chunk);
								
								recv_chunk = next_recv_chunk;
							}
							
							// last partial chunk
							everything_plugin_os_copy_memory(d,c->recv_chunk_last + 1,p - HTTP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last));
							d += p - HTTP_SERVER_RECV_CHUNK_DATA(c->recv_chunk_last);
							
							*d = 0;
							
							// keep last chunk.
							c->recv_chunk_count = 1;
							c->recv_chunk_start = c->recv_chunk_last;
							
							// execute command.
							http_server_client_process_command(c,linear_buf);
							
							// free
							everything_plugin_mem_free(linear_buf);
						}
					}
					
					p++;
					start = p;
				}
				else
				{
					p++;
				}
				
				run--;
			}
			
			if (start)
			{
				// we processed at least one command
				// we must move any incomplete commands to the front of the buffer.
				everything_plugin_os_move_memory(c->recv_chunk_last + 1,start,(c->recv_front - (everything_plugin_utf8_t *)start));
				
				c->recv_front = (everything_plugin_utf8_t *)(c->recv_chunk_last + 1) + (c->recv_front - (everything_plugin_utf8_t *)start);
			}
		}
	}
	
	return 1;
}


static void http_server_recv_chunk_add(http_server_client_t *c)
{
	http_server_recv_chunk_t *recv_chunk;	
	
	recv_chunk = everything_plugin_mem_alloc(HTTP_SERVER_RECV_CHUNK_SIZE);
	
	if (c->recv_chunk_start)
	{
		c->recv_chunk_last->next = recv_chunk;
	}
	else
	{
		c->recv_chunk_start = recv_chunk;
	}
	
	recv_chunk->next = 0;
	c->recv_chunk_last = recv_chunk;
	c->recv_chunk_count++;
	
	c->recv_end = ((everything_plugin_utf8_t *)recv_chunk) + HTTP_SERVER_RECV_CHUNK_SIZE;
	c->recv_front = (everything_plugin_utf8_t *)(recv_chunk + 1);
}	

static void http_server_recv_chunk_destroy(http_server_recv_chunk_t *recv_chunk)
{
	everything_plugin_mem_free(recv_chunk);
}

static void http_server_send_chunk_destroy(http_server_send_chunk_t *send_chunk)
{
	everything_plugin_mem_free(send_chunk->data);
	everything_plugin_mem_free(send_chunk);
}

// size MUST be > 0
static void http_server_client_send_add(http_server_client_t *c,const void *data,uintptr_t size)
{
	const everything_plugin_utf8_t *d;
	
	d = data;
	
	while(size)
	{
		uintptr_t copy_size;
		
		// buffer full?
		if (c->send_front == c->send_end)
		{
			http_server_client_flush(c);
			
			// new buffer
			c->send_buffer = everything_plugin_mem_alloc(HTTP_SERVER_SEND_CHUNK_SIZE);
			c->send_front = c->send_buffer;
			c->send_end = c->send_buffer + HTTP_SERVER_SEND_CHUNK_SIZE;
		}
		
		// copy as much as we can into the chunk.
		copy_size = (c->send_end - c->send_front);
		
		if (copy_size > size)
		{
			copy_size = size;
		}
		
		// copy_size is at least one byte.
		everything_plugin_os_copy_memory(c->send_front,d,copy_size);
		
		c->send_front += copy_size;
		d += copy_size;
		size -= copy_size;
	}
}

// adds the current send buffer to the send chunk list.
static void http_server_client_flush(http_server_client_t *c)
{
	if (c->send_buffer) 
	{
		if (c->send_front - c->send_buffer)
		{
			http_server_send_chunk_t *send_chunk;
			
			send_chunk = everything_plugin_mem_alloc(sizeof(http_server_send_chunk_t));
			
			send_chunk->size = (c->send_front - c->send_buffer);
			send_chunk->data = c->send_buffer;
			
			// insert
			if (c->send_chunk_start)
			{
				c->send_chunk_last->next = send_chunk;
			}
			else
			{
				// start sending.
				PostMessage(_http_server->hwnd,HTTP_SERVER_WM_CLIENT,c->socket_handle,0);

				c->send_chunk_start = send_chunk;
			}
			
			c->send_chunk_last = send_chunk;
			send_chunk->next = 0;
			
			// clear buffer.
			c->send_buffer = 0;
		}
	}
}

// update send
static int http_server_client_update_send(http_server_client_t *c)
{
	for(;;)
	{
		int ret;
		
		// grab some data to send.
		if (!c->send_remaining)
		{
			// is there a full chunk?
			if (c->send_chunk_start)
			{
				c->send_remaining = c->send_chunk_start->size;
			}
			else
			{
				// no data to send.
				if (c->send_shutdown)
				{
					// is there a file or resource to send?
					if (c->data_file != INVALID_HANDLE_VALUE)
					{
						for(;;)
						{
							int ret;
							uintptr_t retr_remaining;
							uintptr_t retr_size;
							int retr_state;
							
							// fill buffer.
							EnterCriticalSection(&c->data_cs);
							
							retr_remaining = c->data_remaining;
							retr_size = c->data_size;
							retr_state = c->data_state;
							
							LeaveCriticalSection(&c->data_cs);
							
							if (!retr_remaining)
							{
								// read
								if (retr_state == 1)
								{
									if (!c->data_complete)
									{
										c->data_complete = 1;
										
										// never close the socket, the client MUST close the socket
										// otherwise data WILL BE LOST.
										// we use shutdown to let the client know there is no more data.
										everything_plugin_os_winsock_shutdown(c->socket_handle,EVERYTHING_PLUGIN_OS_WINSOCK_SD_SEND);
									}
									
									break;
								}
								else
								if (retr_state == 2)
								{
									return 0;
								}
								else
								if (retr_state == 0)
								{
									// request more data..
									SetEvent(c->data_hevent);
									
									break;
								}
								
								break;
							}
					
							// c->send_cur->packet_len would be nonzero
							// we test for this after sending the header.
							// send data
							ret = everything_plugin_network_send(c->socket_handle,c->data_buffer + retr_size - retr_remaining,retr_remaining);

							if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
							{
								if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
								{
									break;
								}
								
								return 0;
							}
							else
							if (ret == 0)
							{
								return 0;
							}
							else
							{
								EnterCriticalSection(&c->data_cs);
								c->data_remaining = retr_remaining - ret;
								LeaveCriticalSection(&c->data_cs);
							}
						}		
					}
					else
					{
						// shutdown the socket.
						// BUT dont close it yet...
						// we MUST wait for the client to close their end.
						everything_plugin_os_winsock_shutdown(c->socket_handle,EVERYTHING_PLUGIN_OS_WINSOCK_SD_SEND);
					}
				}

				break;
			}
		}
		
		// send the packet.
		ret = everything_plugin_network_send(c->socket_handle,c->send_chunk_start->data + c->send_chunk_start->size - c->send_remaining,c->send_remaining);

		if (ret == EVERYTHING_PLUGIN_OS_WINSOCK_SOCKET_ERROR)
		{
			if (everything_plugin_os_winsock_WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}
			
			return 0;
		}
		else
		if (ret == 0)
		{
			// socket closed.
			return 0;
		}
		else
		{
			c->send_remaining -= ret;
			
			if (!c->send_remaining)
			{
				http_server_send_chunk_t *next_send_chunk;
				
				next_send_chunk = c->send_chunk_start->next;
				
				// free
				http_server_send_chunk_destroy(c->send_chunk_start);
				
				// unlink
				c->send_chunk_start	= next_send_chunk;
				if (!c->send_chunk_start)
				{
					c->send_chunk_last = 0;
				}
			}
		}
	}
	
	return 1;
}

// shutdown send, makes sure all data is sent first.
static void http_server_client_send_shutdown(http_server_client_t *c)
{
	http_server_client_flush(c);

	c->send_shutdown = 1;

	// update send
	PostMessage(_http_server->hwnd,HTTP_SERVER_WM_CLIENT,c->socket_handle,0);
}

static void http_server_client_printf(http_server_client_t *c,const everything_plugin_utf8_t *format,...)
{
	everything_plugin_utf8_buf_t cbuf;
	
	everything_plugin_utf8_buf_init(&cbuf);

	{
		va_list argptr;
			
		va_start(argptr,format);
	
		everything_plugin_utf8_buf_vprintf(&cbuf,format,argptr);

		va_end(argptr);
	}

	http_server_client_send_add(c,cbuf.buf,cbuf.len);

	everything_plugin_utf8_buf_kill(&cbuf);
}

static void http_server_client_send_string_id(http_server_client_t *c,DWORD string_id)
{
	http_server_client_send_add(c,_http_server->strings[string_id],everything_plugin_utf8_string_get_length_in_bytes(_http_server->strings[string_id]));
}

static void http_server_client_send_utf8(http_server_client_t *c,const everything_plugin_utf8_t *text)
{
	http_server_client_send_add(c,text,everything_plugin_utf8_string_get_length_in_bytes(text));
}

static int http_server_compare_folder(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	if (a->is_folder)
	{
		if (!(b->is_folder))
		{
			return -1;
		}
	}
	else
	{
		if (b->is_folder)
		{
			return 1;
		}
	}
	
	return 0;
}

static int WINAPI http_server_compare_name_ascending(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	int i;
	
	i = http_server_compare_folder(a,b);
	if (i) return i;
	
	return everything_plugin_utf8_string_compare_nice_n_n(HTTP_SERVER_SORT_ITEM_FILENAME(a),a->len,HTTP_SERVER_SORT_ITEM_FILENAME(b),b->len);
}

static int WINAPI http_server_compare_name_descending(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	return -http_server_compare_name_ascending(a,b);
}

static int WINAPI http_server_compare_size_ascending(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	int i;
	
	i = http_server_compare_folder(a,b);
	if (i) return i;
	
	if (a->fd.size == EVERYTHING_PLUGIN_QWORD_MAX)
	{
		if (b->fd.size != EVERYTHING_PLUGIN_QWORD_MAX)
		{
			return -1;
		}
	}
	else
	{
		if (b->fd.size == EVERYTHING_PLUGIN_QWORD_MAX)
		{
			return 1;
		}
	}

	if (a->fd.size < b->fd.size) return -1;
	if (a->fd.size > b->fd.size) return 1;
	
	return http_server_compare_name_ascending(a,b);
}

static int WINAPI http_server_compare_size_descending(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	return -http_server_compare_size_ascending(a,b);
}

static int WINAPI http_server_compare_date_ascending(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	int i;
	
	i = http_server_compare_folder(a,b);
	if (i) return i;
	
	if (a->fd.date_modified == EVERYTHING_PLUGIN_QWORD_MAX)
	{
		if (b->fd.date_modified != EVERYTHING_PLUGIN_QWORD_MAX)
		{
			return -1;
		}
	}
	else
	{
		if (b->fd.date_modified == EVERYTHING_PLUGIN_QWORD_MAX)
		{
			return 1;
		}
	}
	
	if (a->fd.date_modified < b->fd.date_modified) return -1;
	if (a->fd.date_modified > b->fd.date_modified) return 1;
	
	return http_server_compare_name_ascending(a,b);
}

static int WINAPI http_server_compare_date_descending(const http_server_sort_item_t *a,const http_server_sort_item_t *b)
{
	return -http_server_compare_date_ascending(a,b);
}


static void http_server_client_table_begin(http_server_client_t *c,const everything_plugin_utf8_t *search_string)
{
	// begin table.
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_CENTER);

	// logo
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_LOGO);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_LOGO2);
	http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_EVERYTHING));
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_LOGO3);

	// input form
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM2);
	http_server_send_escape_html(c,everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_SEARCH_EVERYTHING));
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM3);
	http_server_send_escape_html(c,search_string?search_string:(const everything_plugin_utf8_t *)"");
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM4);
	
	//if (!((c->sort == HTTP_SERVER_SORT_NAME) && (c->sort_ascending)))
	{
		const everything_plugin_property_t *property_type;
		
		property_type = http_server_get_property_from_sort(c->sort);
		if (property_type)
		{
			// only if it's a fast sort!
			if (everything_plugin_db_query_is_fast_sort(_http_server->q,property_type))
			{
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM5);
				http_server_client_send_utf8(c,http_server_get_sort_name(c->sort));
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM6);
				http_server_client_printf(c,(const everything_plugin_utf8_t *)"%d",c->sort_ascending);
				http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM7);
			}
		}
	}
	
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_FORM8);

	// begin table.
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TB_TABLE);
}

static void http_server_client_send_table_header(http_server_client_t *c,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *search,int is_current_sort,int ascending,const everything_plugin_utf8_t *classname,const everything_plugin_utf8_t *title,const everything_plugin_utf8_t *sortname,int default_ascending)
{
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_TD);
	http_server_client_send_utf8(c,classname);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_TD2);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_TD3);
	
	if (path)
	{
		everything_plugin_utf8_buf_t url_cbuf;
		
		everything_plugin_utf8_buf_init(&url_cbuf);

		http_server_escape_url_filename(&url_cbuf,path);

		http_server_send_escape_html(c,url_cbuf.buf);
		
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"?sort=%s&amp;ascending=%d",sortname,is_current_sort ? !ascending : default_ascending);

		everything_plugin_utf8_buf_kill(&url_cbuf);
	}
	else
	{
		everything_plugin_utf8_buf_t url_cbuf;
		
		everything_plugin_utf8_buf_init(&url_cbuf);

		http_server_escape_url(&url_cbuf,search,1);

		// convert spaces to +'s
		{
			everything_plugin_utf8_t *p;
			
			p = url_cbuf.buf;
			
			while(*p)
			{
				if (*p == ' ')
				{
					*p = '+';
				}
				
				p++;
			}
		}

		http_server_client_printf(c,(const everything_plugin_utf8_t *)"?search=");

		http_server_send_escape_html(c,url_cbuf.buf);

		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;sort=%s&amp;ascending=%d",sortname,is_current_sort ? !ascending : default_ascending);

		everything_plugin_utf8_buf_kill(&url_cbuf);
	}
	
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_TD4);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_NOBR);
	http_server_send_escape_html(c,title);
	
	if (is_current_sort)
	{
		// localized?
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_UPDOWN);
		http_server_client_send_utf8(c,ascending ? "up.gif" : "down.gif");
		http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_UPDOWN2);
	}

	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_NOBR_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_A_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TH_TD_END);
}

static void http_server_client_send_table_headers(http_server_client_t *c,const everything_plugin_utf8_t *path,const everything_plugin_utf8_t *search,int sort,int ascending)
{
	// columns
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_THS_TR);
	
	http_server_client_send_table_header(c,path,search,sort == HTTP_SERVER_SORT_NAME,ascending,(const everything_plugin_utf8_t *)"nameheader",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_NAME),(const everything_plugin_utf8_t *)"name",1);
	
	if (search)
	{
		http_server_client_send_table_header(c,path,search,sort == HTTP_SERVER_SORT_PATH,ascending,(const everything_plugin_utf8_t *)"pathheader",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_PATH),(const everything_plugin_utf8_t *)"path",1);
	}
	
	http_server_client_send_table_header(c,path,search,sort == HTTP_SERVER_SORT_SIZE,ascending,(const everything_plugin_utf8_t *)"sizeheader",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_SIZE),(const everything_plugin_utf8_t *)"size",0);
	http_server_client_send_table_header(c,path,search,sort == HTTP_SERVER_SORT_DATE_MODIFIED,ascending,(const everything_plugin_utf8_t *)"modifiedheader",everything_plugin_localization_get_string(EVERYTHING_PLUGIN_LOCALIZATION_DATE_MODIFIED),(const everything_plugin_utf8_t *)"date_modified",0);
		
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_THS_TR_END);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_THS_LINE);
	http_server_client_printf(c,(const everything_plugin_utf8_t *)"%d",search?4:3);
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_THS_LINE2);
}

static void http_server_client_table_end(http_server_client_t *c)
{
	http_server_client_send_string_id(c,HTTP_SERVER_STRING_TE_END);
}

static void http_server_client_send_search_link(http_server_client_t *c,const everything_plugin_utf8_t *search_string,int sort,int ascending,uintptr_t offset,DWORD show_max)
{
	everything_plugin_utf8_buf_t url_cbuf;
	
	everything_plugin_utf8_buf_init(&url_cbuf);

	http_server_escape_url(&url_cbuf,search_string,1);
	
	// convert spaces to +'s
	{
		everything_plugin_utf8_t *p;
		
		p = url_cbuf.buf;
		
		while(*p)
		{
			if (*p == ' ')
			{
				*p = '+';
			}
			
			p++;
		}
	}	

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"?search=");

	http_server_send_escape_html(c,url_cbuf.buf);

	http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;sort=%s&amp;ascending=%d",http_server_get_sort_name(sort),ascending);
	
	if (offset)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;offset=%zu",offset);
	}
	
	if (show_max != http_server_items_per_page)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;count=%u",show_max);
	}

	if (c->match_case)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;case=1");
	}
	
	if (c->match_whole_word)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;wholeword=1");
	}
		
	if (c->match_path)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;path=1");
	}
		
	if (c->match_regex)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;regex=1");
	}
		
	if (c->match_diacritics)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;diacritics=1");
	}
	
	if (c->match_prefix)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;prefix=1");
	}
	
	if (c->match_suffix)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;suffix=1");
	}
	
	if (c->ignore_punctuation)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;ignore_punctuation=1");
	}
	
	if (c->ignore_whitespace)
	{
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"&amp;ignore_whitespace=1");
	}
	
	everything_plugin_utf8_buf_kill(&url_cbuf);
}

static DWORD WINAPI http_server_send_file_thread_proc(http_server_client_t *c)
{
	EVERYTHING_PLUGIN_QWORD totread;
	
	totread = 0;
	
	if (c->content_range_start)
	{
		LARGE_INTEGER start;
		
		start.QuadPart = c->content_range_start;
		
		if (!(SetFilePointer(c->data_file,start.LowPart,&start.HighPart,FILE_BEGIN)))
		{
			if (GetLastError() != NO_ERROR)
			{
				EnterCriticalSection(&c->data_cs);
				c->data_state = 2;
				LeaveCriticalSection(&c->data_cs);
					
				// update data socket with new retr data.
				PostMessage(_http_server->hwnd,HTTP_SERVER_WM_CLIENT,c->socket_handle,0);

				goto exit;			
			}
		}
	}
	
	for(;;)
	{
		DWORD num_bytes_read;
		DWORD bytes_to_read;
		
		// wait for data request.
		WaitForSingleObject(c->data_hevent,INFINITE);

		// reset event		
		ResetEvent(c->data_hevent);
		
		// abort?
		EnterCriticalSection(&c->data_cs);
		
		if (c->data_abort)
		{
			LeaveCriticalSection(&c->data_cs);
			
			break;
		}

		// still data remaining?		
		if (c->data_remaining)
		{
			LeaveCriticalSection(&c->data_cs);
			
			continue;
		}
		
		LeaveCriticalSection(&c->data_cs);
		
		bytes_to_read = HTTP_SERVER_DATA_CHUNK_SIZE;
		
		if (c->content_range_size)
		{
			// end is inclusive.
			if (c->content_range_size - totread < bytes_to_read)
			{
				bytes_to_read = (DWORD)(c->content_range_size - totread);
			}
		}

		// get some data.
		if (ReadFile(c->data_file,c->data_buffer,bytes_to_read,&num_bytes_read,0))
		{
			if (num_bytes_read)
			{
				// this is only accessed outs
				EnterCriticalSection(&c->data_cs);
				c->data_size = num_bytes_read;
				c->data_remaining = num_bytes_read;
				LeaveCriticalSection(&c->data_cs);
				
				// update data socket with new retr data.
				PostMessage(_http_server->hwnd,HTTP_SERVER_WM_CLIENT,c->socket_handle,0);
				
				totread += num_bytes_read;
			}
			else
			{
				// EOF
				EnterCriticalSection(&c->data_cs);
				c->data_state = 1;
				LeaveCriticalSection(&c->data_cs);
				
				// update data socket with new retr data.
				PostMessage(_http_server->hwnd,HTTP_SERVER_WM_CLIENT,c->socket_handle,0);

				break;			
			}
		}
		else
		{
			EnterCriticalSection(&c->data_cs);
			c->data_state = 2;
			LeaveCriticalSection(&c->data_cs);
				
			// update data socket with new retr data.
			PostMessage(_http_server->hwnd,HTTP_SERVER_WM_CLIENT,c->socket_handle,0);

			break;
		}
	}
	
exit:	

	return 0;
}

static void http_server_remove_query(http_server_client_t *c)
{
	if (_http_server->client_query_start == c)
	{
		_http_server->client_query_start = c->query_next;
	}
	else
	{
		c->query_prev->query_next = c->query_next;
	}

	if (_http_server->client_query_last == c)
	{
		_http_server->client_query_last = c->query_prev;
	}
	else
	{
		c->query_next->query_prev = c->query_prev;
	}
	
	c->is_query = 0;
}

static void http_server_insert_query(http_server_client_t *c)
{
	if (_http_server->client_query_start)
	{
		_http_server->client_query_last->query_next = c;
		c->query_prev = _http_server->client_query_last;
	}
	else
	{
		_http_server->client_query_start = c;
		c->query_prev = 0;
	}
	
	c->query_next = 0;
	_http_server->client_query_last = c;
	
	c->is_query = 1;
}

static void http_server_start_next_query(void)
{
	// we might be able to send more than one query
	// if they are all cached querys.
	for(;;)
	{
		http_server_client_t *c;
		
		c = _http_server->client_query_start;
		
		if (!c) 
		{
			break;
		}
		
		// remove from pending query list.
		http_server_remove_query(c);
		
		// check cache
		if ((_http_server->is_current_query)
			&&  (everything_plugin_utf8_string_compare(_http_server->current_search_string,c->search_string) == 0)
			&&  (_http_server->current_match_case == c->match_case)
			&&  (_http_server->current_match_diacritics == c->match_diacritics)
			&&  (_http_server->current_match_prefix == c->match_prefix)
			&&  (_http_server->current_match_suffix == c->match_suffix)
			&&  (_http_server->current_ignore_punctuation == c->ignore_punctuation)
			&&  (_http_server->current_ignore_whitespace == c->ignore_whitespace)
			&&  (_http_server->current_match_path == c->match_path)
			&&  (_http_server->current_match_regex == c->match_regex)
			&&  (_http_server->current_match_whole_word == c->match_whole_word)
		)
		{
			// same query...
			// check sort
			if ((_http_server->current_db_sort_column_type == c->db_sort_column_type) && (!!_http_server->current_db_sort_ascending == !!c->db_sort_ascending))
			{
				// everything_plugin_debug_printf((const everything_plugin_utf8_t *)"same query\n");
				// same sort.
				http_server_send_query_results(c);
			}
			else
			{
				// copy to current.
				_http_server->current_db_sort_column_type = c->db_sort_column_type;
				_http_server->current_db_sort_ascending = c->db_sort_ascending;
				
				// everything_plugin_debug_printf((const everything_plugin_utf8_t *)"same query, different sort\n");
				// sort
				everything_plugin_db_query_sort(_http_server->q,c->db_sort_column_type,c->db_sort_ascending,NULL,0,NULL,0,0,0,EVERYTHING_PLUGIN_DB_QUERY_FIND_DUPLICATES_NONE,0);
				
				// mark this client as the active query
				_http_server->client_query_active = c;				
			}
			
			// continue with next search.
		}
		else
		{
			// copy to current.
			_http_server->current_search_string = everything_plugin_utf8_string_realloc_utf8_string(_http_server->current_search_string,c->search_string);
			_http_server->current_match_case = c->match_case;
			_http_server->current_match_diacritics = c->match_diacritics;
			_http_server->current_match_prefix = c->match_prefix;
			_http_server->current_match_suffix = c->match_suffix;
			_http_server->current_ignore_punctuation = c->ignore_punctuation;
			_http_server->current_ignore_whitespace = c->ignore_whitespace;
			_http_server->current_match_path = c->match_path;
			_http_server->current_match_regex = c->match_regex;
			_http_server->current_match_whole_word = c->match_whole_word;
			_http_server->current_db_sort_column_type = c->db_sort_column_type;
			_http_server->current_db_sort_ascending = c->db_sort_ascending;
			_http_server->is_current_query = 1;

			// start the query	
			everything_plugin_db_query_search(
				_http_server->q,
				c->match_case,
				c->match_whole_word,
				c->match_path,
				c->match_diacritics,
				c->match_prefix,
				c->match_suffix,
				c->ignore_punctuation,
				c->ignore_whitespace,
				c->match_regex,
				0,
				1,
				1,
				c->search_string,
				1,
				c->db_sort_column_type,
				c->db_sort_ascending,
				NULL,
				0,
				NULL,
				0,
				0,
				0,
				0,
				0,
				http_server_allow_query_access,
				http_server_allow_disk_access,
				http_server_allow_disk_access,
				0,
				EVERYTHING_PLUGIN_CONFIG_SIZE_STANDARD_JEDEC,
				0);
			
			// mark this client as the active query
			_http_server->client_query_active = c;
			
			break;
		}
	}
}

static const everything_plugin_utf8_t *http_server_get_sort_name(int sort)
{
	switch(sort)
	{
		case HTTP_SERVER_SORT_PATH: return (const everything_plugin_utf8_t *)"path"; 
		case HTTP_SERVER_SORT_SIZE: return (const everything_plugin_utf8_t *)"size"; 
		case HTTP_SERVER_SORT_DATE_MODIFIED: return (const everything_plugin_utf8_t *)"date_modified"; 
	}

	return (const everything_plugin_utf8_t *)"name";
}

static const everything_plugin_property_t *http_server_get_property_from_sort(int sort)
{
	int property_type;
	
	switch(sort)
	{
		case HTTP_SERVER_SORT_PATH: property_type = EVERYTHING_PLUGIN_PROPERTY_TYPE_PATH; break;
		case HTTP_SERVER_SORT_SIZE: property_type = EVERYTHING_PLUGIN_PROPERTY_TYPE_SIZE; break;
		case HTTP_SERVER_SORT_DATE_MODIFIED: property_type = EVERYTHING_PLUGIN_PROPERTY_TYPE_DATE_MODIFIED; break;
		default: property_type = EVERYTHING_PLUGIN_PROPERTY_TYPE_NAME; break;
	}
	
	return everything_plugin_property_get_builtin_type(property_type);
}

static void http_server_update_options_page(HWND page_hwnd)
{
	int is_enabled;
	int is_logging_enabled;
	
	is_enabled = (IsDlgButtonChecked(page_hwnd,HTTP_SERVER_PLUGIN_ID_ENABLED_TICKBOX) == BST_CHECKED) ? 1 : 0;
	is_logging_enabled = 0;
	
	if (is_enabled)
	{
		is_logging_enabled = (IsDlgButtonChecked(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX) == BST_CHECKED) ? 1 : 0;
	}

	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOGGING_ENABLED_TICKBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_ALLOW_FILE_DOWNLOAD_TICKBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_EDIT,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_HOME_BROWSE_BUTTON,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_EDIT,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_DEFAULT_PAGE_BROWSE_BUTTON,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_PORT_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_PORT_EDITBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_USERNAME_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_USERNAME_EDITBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_PASSWORD_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_PASSWORD_EDITBOX,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_BINDINGS_STATIC,is_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_BINDINGS_EDITBOX,is_enabled);
	
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_EDITBOX,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_NAME_BROWSE_BUTTON,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_MAX_SIZE_EDITBOX,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_LOG_FILE_STATIC,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_MAX_SIZE_STATIC,is_logging_enabled);
	everything_plugin_os_enable_or_disable_dlg_item(page_hwnd,HTTP_SERVER_PLUGIN_ID_KB_STATIC,is_logging_enabled);
}

static void http_server_create_checkbox(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,int checked)
{
	everything_plugin_os_create_checkbox(load_options_page->page_hwnd,id,extra_style,checked,everything_plugin_localization_get_string(text_localization_id));

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id));
}

static void http_server_create_static(everything_plugin_load_options_page_t *load_options_page,int id,int text_localization_id)
{
	everything_plugin_os_create_static(load_options_page->page_hwnd,id,SS_LEFTNOWORDWRAP|WS_GROUP,everything_plugin_localization_get_string(text_localization_id));
}

static void http_server_create_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,const everything_plugin_utf8_t *text)
{
	everything_plugin_os_create_edit(load_options_page->page_hwnd,id,WS_GROUP,text);

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id));
}

static void http_server_create_number_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,int value)
{
	everything_plugin_os_create_number_edit(load_options_page->page_hwnd,id,WS_GROUP,value);

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id));
}

static void http_server_create_password_edit(everything_plugin_load_options_page_t *load_options_page,int id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id,const everything_plugin_utf8_t *text)
{
	everything_plugin_os_create_password_edit(load_options_page->page_hwnd,id,WS_GROUP,text);

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id));
}

static void http_server_create_button(everything_plugin_load_options_page_t *load_options_page,int id,DWORD extra_style,int text_localization_id,int tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id)
{
	everything_plugin_os_create_button(load_options_page->page_hwnd,id,extra_style,everything_plugin_localization_get_string(text_localization_id));

	everything_plugin_os_add_tooltip(load_options_page->tooltip_hwnd,load_options_page->page_hwnd,id,everything_plugin_localization_get_string(tooltip_EVERYTHING_PLUGIN_LOCALIZATION_id));
}

static void http_server_enable_options_apply(everything_plugin_options_page_proc_t *options_page_proc)
{
	everything_plugin_os_enable_or_disable_dlg_item(options_page_proc->options_hwnd,1001,1);
}

static int http_server_expand_min_wide(HWND page_hwnd,int text_localization_id,int current_wide)
{
	int wide;
	
	wide = everything_plugin_os_expand_dialog_text_logical_wide_no_prefix(page_hwnd,everything_plugin_localization_get_string(text_localization_id),current_wide);
	
	if (wide > current_wide)
	{
		return wide;
	}
	
	return current_wide;
}

static everything_plugin_utf8_t *http_server_get_options_text(HWND page_hwnd,int id,everything_plugin_utf8_t *old_value)
{
	everything_plugin_utf8_buf_t cbuf;
	everything_plugin_utf8_t *ret;

	everything_plugin_utf8_buf_init(&cbuf);
	
	everything_plugin_os_get_dlg_text(page_hwnd,id,&cbuf);
	
	ret = everything_plugin_utf8_string_realloc_utf8_string(old_value,cbuf.buf);

	everything_plugin_utf8_buf_kill(&cbuf);
	
	return ret;
}

static int http_server_is_valid_header(const everything_plugin_utf8_t *s)
{
	const everything_plugin_utf8_t *p;
	
	p = s;
	
	while(*p)
	{
		if (*p < ' ')
		{
			return 0;
		}
	
		p++;
	}
	
	return 1;
}

static void http_server_client_send_header_list(http_server_client_t *c,const everything_plugin_utf8_t *list)
{
	everything_plugin_utf8_buf_t cbuf;
	const everything_plugin_utf8_t *p;
	
	everything_plugin_utf8_buf_init(&cbuf);
	p = list;
	
	for(;;)
	{
		p = everything_plugin_utf8_string_parse_csv_item(p,&cbuf);
		if (!p)
		{
			break;
		}
		
		// make sure there's no \r\n.
		if (http_server_is_valid_header(cbuf.buf))
		{
			if (cbuf.len)
			{
				http_server_client_send_add(c,cbuf.buf,cbuf.len);

				http_server_client_printf(c,(const everything_plugin_utf8_t *)"\r\n");
			}
		}
	}

	everything_plugin_utf8_buf_kill(&cbuf);
}

static void http_server_send_sort_url_param(http_server_client_t *c)
{
	if (!((c->sort == http_server_default_sort) && (!!c->sort_ascending == !!http_server_default_sort_ascending)))
	{
		// not the default sort
		http_server_client_printf(c,(const everything_plugin_utf8_t *)"?sort=%s&amp;ascending=%d",http_server_get_sort_name(c->sort),c->sort_ascending);
	}
}

