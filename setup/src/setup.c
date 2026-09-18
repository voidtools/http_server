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

// TO UPDATE:
// Update "../src/version.h"

#define SETUP_NAME								"{{178}}"	// HTTP Server
#define SETUP_DESCRIPTION						"{{2398}}"	// Allow users to search and access your files from a webbrowser
#define SETUP_AUTHOR							"voidtools"
#define SETUP_VERSION							PLUGINVERSION
#define SETUP_LINK								"{{2402}}"	// https://www.voidtools.com/everything/plugins/

#if defined(_WIN64)
#define SETUP_DLL_NAME							"http_server64.dll"
#else
#define SETUP_DLL_NAME							"http_server32.dll"
#endif

#define SETUP_EVERYTHING_PROGRAM_NAME			"Everything"
#define SETUP_EVERYTHING_TASKBAR_NOTIFICATION	"EVERYTHING_TASKBAR_NOTIFICATION"
#define SETUP_EVERYTHING_TASKBAR_NOTIFICATION_1_5_ALPHA	"EVERYTHING_TASKBAR_NOTIFICATION_(1.5a)"

#define SETUP_MAX_STRING		MAX_PATH
#define SETUP_MAX_COMMAND_LINE	32768

// compiler options
#pragma warning(disable : 4311) // type cast void * to unsigned int
#pragma warning(disable : 4312) // type cast unsigned int to void *
#pragma warning(disable : 4244) // warning C4244: 'argument' : conversion from 'LONG_PTR' to 'LONG', possible loss of data
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4313) // 'printf' : '%x' in format string conflicts with argument 2 of type 'line_t *'
#pragma warning(disable : 4996) // deprecation
//#pragma warning(disable : 4100) // unreferenced formal parameter

// REQUIRES IE 5.01
#define _WIN32_IE 0x0501

// WINNT required for some header definitions.
// minimum is really 0x0400
#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN

#define _INC_CTYPE

#include <windows.h>
#include <shellapi.h>
#include <Psapi.h>
#include <shlobj.h> // SHBrowseForFolderW
#include <shobjidl.h> // IShellItem
#include "../res/resource.h"
#include "../../src/version.h"

typedef struct setup_IFileOpenDialogVtbl_s
{
	HRESULT(STDMETHODCALLTYPE* QueryInterface)(struct setup_IFileOpenDialog_s* This, REFIID riid, void** ppvObject);
	ULONG(STDMETHODCALLTYPE* AddRef)(struct setup_IFileOpenDialog_s* This);
	ULONG(STDMETHODCALLTYPE* Release)(struct setup_IFileOpenDialog_s* This);
	HRESULT(STDMETHODCALLTYPE* Show)(struct setup_IFileOpenDialog_s* This, HWND hwndOwner);
	HRESULT(STDMETHODCALLTYPE* SetFileTypes)(struct setup_IFileOpenDialog_s* This, UINT cFileTypes, const struct setup_COMDLG_FILTERSPEC_s* rgFilterSpec);
	HRESULT(STDMETHODCALLTYPE* SetFileTypeIndex)(struct setup_IFileOpenDialog_s* This, UINT iFileType);
	HRESULT(STDMETHODCALLTYPE* GetFileTypeIndex)(struct setup_IFileOpenDialog_s* This, UINT* piFileType);
	HRESULT(STDMETHODCALLTYPE* Advise)(struct setup_IFileOpenDialog_s* This, struct _os_IFileDialogEvents_s* pfde, DWORD* pdwCookie);
	HRESULT(STDMETHODCALLTYPE* Unadvise)(struct setup_IFileOpenDialog_s* This, DWORD dwCookie);
	HRESULT(STDMETHODCALLTYPE* SetOptions)(struct setup_IFileOpenDialog_s* This, int fos);
	HRESULT(STDMETHODCALLTYPE* GetOptions)(struct setup_IFileOpenDialog_s* This, int* pfos);
	HRESULT(STDMETHODCALLTYPE* SetDefaultFolder)(struct setup_IFileOpenDialog_s* This, IShellItem* psi);
	HRESULT(STDMETHODCALLTYPE* SetFolder)(struct setup_IFileOpenDialog_s* This, IShellItem* psi);
	HRESULT(STDMETHODCALLTYPE* GetFolder)(struct setup_IFileOpenDialog_s* This, IShellItem** ppsi);
	HRESULT(STDMETHODCALLTYPE* GetCurrentSelection)(struct setup_IFileOpenDialog_s* This, IShellItem** ppsi);
	HRESULT(STDMETHODCALLTYPE* SetFileName)(struct setup_IFileOpenDialog_s* This, LPCWSTR pszName);
	HRESULT(STDMETHODCALLTYPE* GetFileName)(struct setup_IFileOpenDialog_s* This, LPWSTR* pszName);
	HRESULT(STDMETHODCALLTYPE* SetTitle)(struct setup_IFileOpenDialog_s* This, LPCWSTR pszTitle);
	HRESULT(STDMETHODCALLTYPE* SetOkButtonLabel)(struct setup_IFileOpenDialog_s* This, LPCWSTR pszText);
	HRESULT(STDMETHODCALLTYPE* SetFileNameLabel)(struct setup_IFileOpenDialog_s* This, LPCWSTR pszLabel);
	HRESULT(STDMETHODCALLTYPE* GetResult)(struct setup_IFileOpenDialog_s* This, IShellItem** ppsi);
	HRESULT(STDMETHODCALLTYPE* AddPlace)(struct setup_IFileOpenDialog_s* This, IShellItem* psi, int fdap);
	HRESULT(STDMETHODCALLTYPE* SetDefaultExtension)(struct setup_IFileOpenDialog_s* This, LPCWSTR pszDefaultExtension);
	HRESULT(STDMETHODCALLTYPE* Close)(struct setup_IFileOpenDialog_s* This, HRESULT hr);
	HRESULT(STDMETHODCALLTYPE* SetClientGuid)(struct setup_IFileOpenDialog_s* This, REFGUID guid);
	HRESULT(STDMETHODCALLTYPE* ClearClientData)(struct setup_IFileOpenDialog_s* This);
	HRESULT(STDMETHODCALLTYPE* SetFilter)(struct setup_IFileOpenDialog_s* This, void* pFilter);
	HRESULT(STDMETHODCALLTYPE* GetResults)(struct setup_IFileOpenDialog_s* This, void** ppenum);
	HRESULT(STDMETHODCALLTYPE* GetSelectedItems)(struct setup_IFileOpenDialog_s* This, void** ppsai);

}setup_IFileOpenDialogVtbl_t;

typedef struct setup_IFileOpenDialog_s
{
	struct setup_IFileOpenDialogVtbl_s* lpVtbl;

}setup_IFileOpenDialog_t;

static const IID setup_CLSID_FileOpenDialog = { 0xdc1c5a9c,0xe88a,0x4dde,{0xa5,0xa1,0x60,0xf8,0x2a,0x20,0xae,0xf7} };
static const IID setup_IID_IFileOpenDialog = { 0xd57c7288,0xd4ad,0x4768,{0xbe,0x02,0x9d,0x96,0x95,0x32,0xd9,0x60} };

#ifdef SETUP_X86

// load unicode for windows 95/98
HMODULE LoadUnicowsProc(void);

extern FARPROC _PfnLoadUnicows = (FARPROC)&LoadUnicowsProc;

HMODULE LoadUnicowsProc(void)
{
	OSVERSIONINFOA osvi;

	// make sure we are win9x.
	// to prevent loading unicows.dll on NT as a securiry precausion.
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if (GetVersionExA(&osvi))
	{
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			return LoadLibraryA("unicows.dll");
		}
	}

	return NULL;
}
#endif

void setup_main(void);
int setup_get_reg_string(HKEY root_hkey, const char* key, const char* value, wchar_t* buf);
void setup_atow(const char* as, wchar_t* ws);
int setup_get_everything_exe_filename(wchar_t* everything_exe_filename_wbuf);
void setup_fatal(DWORD error_code, const char* msg);
void setup_cat_astring(wchar_t* wbuf, DWORD size_in_wchars, const char* as);
void setup_cat_wstring(wchar_t* wbuf, DWORD size_in_wchars, const wchar_t* ws);
void setup_cat_number(wchar_t* wbuf, DWORD size_in_wchars, DWORD number);
static int setup_browse_for_folder(wchar_t* everything_exe_filename_wbuf);
static void setup_copy_string(wchar_t* dst, const wchar_t* src);
static int setup_get_running_exe_filename(const char* window_class, wchar_t* everything_exe_filename_wbuf);

void setup_atow(const char* as, wchar_t* ws)
{
	DWORD run;
	const char* p;
	wchar_t* d;

	run = SETUP_MAX_STRING - 1;
	p = as;
	d = ws;

	while (run)
	{
		if (!*p)
		{
			break;
		}

		*d++ = *p;

		p++;
		run--;
	}

	*d = 0;
}

int setup_get_reg_string(HKEY root_hkey, const char* key, const char* value, wchar_t* buf)
{
	int ret;
	wchar_t key_wbuf[SETUP_MAX_STRING];
	HKEY hkey;

	ret = 0;
	setup_atow(key, key_wbuf);

	// no need to try KEY_WOW64_64KEY
	// because we will call loadlibrary on this exe which will fail because wrong bitness.
	if (RegOpenKeyExW(root_hkey, key_wbuf, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		wchar_t value_wbuf[SETUP_MAX_STRING];
		DWORD buf_size;

		setup_atow(value, value_wbuf);
		buf_size = SETUP_MAX_STRING * sizeof(wchar_t);

		if (RegQueryValueExW(hkey, value_wbuf, 0, NULL, (BYTE*)buf, &buf_size) == ERROR_SUCCESS)
		{
			DWORD buf_wlen;

			buf_wlen = buf_size / sizeof(wchar_t);

			// NULL terminated?
			if (buf_wlen)
			{
				if (buf[buf_wlen - 1] == 0)
				{
					ret = 1;
				}
			}
		}

		RegCloseKey(hkey);
	}

	return ret;
}

static void setup_copy_string(wchar_t* dst, const wchar_t* src)
{
	wchar_t* d;
	const wchar_t* p;
	SIZE_T run;

	d = dst;
	p = src;
	run = SETUP_MAX_STRING - 1;

	while (*p)
	{
		if (!run)
		{
			break;
		}

		*d++ = *p;
		run--;

		p++;
	}

	*d = 0;
}

static int setup_browse_for_folder(wchar_t* everything_exe_filename_wbuf)
{
	setup_IFileOpenDialog_t* file_open_dialog;

	// IFileOpenDialog, when closed, will focus the first dlg item on the previously active window.
	// it will then focus the correct, last focused item.
	// this breaks the previous edit selection
	if (SUCCEEDED(CoCreateInstance(&setup_CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &setup_IID_IFileOpenDialog, &file_open_dialog)))
	{
		int ret;
		IShellItem* shell_item;
		HRESULT hres;

		ret = 0;

		// setting advise for events does not allow us to select the control panel. 
		// we only get FolderChanging events with no SetFolderSelection event

		// FOS_NOCHANGEDIR             = 0x00000008,
		// FOS_PICKFOLDERS             = 0x00000020,
		// FOS_ALLNONSTORAGEITEMS	= 0x80
		// FOS_FORCESHOWHIDDEN	= 0x10000000,

		file_open_dialog->lpVtbl->SetOptions(file_open_dialog, 0x20 | 0x08 | 0x80 | 0x10000000);

		file_open_dialog->lpVtbl->SetTitle(file_open_dialog, L"Select Everything.exe Location");

		hres = file_open_dialog->lpVtbl->Show(file_open_dialog, NULL);
		if (SUCCEEDED(hres))
		{
			if (SUCCEEDED(file_open_dialog->lpVtbl->GetResult(file_open_dialog, &shell_item)))
			{
				LPOLESTR filename;

				if (SUCCEEDED(shell_item->lpVtbl->GetDisplayName(shell_item, SIGDN_DESKTOPABSOLUTEPARSING, &filename)))
				{
					setup_copy_string(everything_exe_filename_wbuf, filename);

					ret = 1;

					CoTaskMemFree(filename);
				}

				shell_item->lpVtbl->Release(shell_item);
			}
		}
		else
		{
			if (hres == 0x800704c7)
			{
				//  The operation was canceled by the user. 
				ExitProcess(1);
			}
		}

		file_open_dialog->lpVtbl->Release(file_open_dialog);

		return ret;
	}

	{
		BROWSEINFOW bi;
		ITEMIDLIST* iil;
		int ret;

		ret = 0;


		// BIF_USENEWUI wont bring the initial selection into view.
		// BIF_NOTRANSLATETARGETS we dont want symbolic links targets, we want the actual path.
		// BIF_NEWDIALOGSTYLE allows computer to be selected, and keeps the OK button focused, which keeps the dialog focused. Otherwise keyboard focus is lost
		ZeroMemory(&bi, sizeof(BROWSEINFOW));
		bi.lpszTitle = L"Select Everything.exe Location";
		bi.pszDisplayName = L"";
		bi.ulFlags = BIF_USENEWUI | BIF_NOTRANSLATETARGETS | BIF_NEWDIALOGSTYLE;
		bi.hwndOwner = NULL;

		iil = (ITEMIDLIST*)SHBrowseForFolderW(&bi);

		if (iil)
		{
			if (SHGetPathFromIDListW(iil, everything_exe_filename_wbuf))
			{
				ret = 1;
			}

			ILFree(iil);
		}
		else
		{
			// cancelled.
			ExitProcess(1);
		}

		return ret;
	}
}

static int setup_get_running_exe_filename(const char* window_class, wchar_t* everything_exe_filename_wbuf)
{
	int ret;
	HWND everything_hwnd;
	wchar_t class_name_wbuf[SETUP_MAX_STRING];

	ret = 0;

	setup_atow(window_class, class_name_wbuf);

	everything_hwnd = FindWindowW(class_name_wbuf, NULL);
	if (everything_hwnd)
	{
		DWORD process_id;

		if (GetWindowThreadProcessId(everything_hwnd, &process_id))
		{
			HANDLE process_handle;

			process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
			if (process_handle)
			{
				if (GetModuleFileNameExW(process_handle, NULL, everything_exe_filename_wbuf, SETUP_MAX_STRING))
				{
					ret = 1;
				}

				CloseHandle(process_handle);
			}
		}
	}

	return ret;
}

int setup_get_everything_exe_filename(wchar_t* everything_exe_filename_wbuf)
{
	int ret;

	ret = 0;

	if (!ret)
	{
		// get everything install folder
		if (setup_get_reg_string(HKEY_LOCAL_MACHINE, "SOFTWARE\\voidtools\\"SETUP_EVERYTHING_PROGRAM_NAME, "ExePath", everything_exe_filename_wbuf))
		{
			ret = 1;
		}
	}

	// portable version running?
	if (!ret)
	{
		if (setup_get_running_exe_filename(SETUP_EVERYTHING_TASKBAR_NOTIFICATION, everything_exe_filename_wbuf))
		{
			ret = 1;
		}
	}

	// 1.5a version running?
	if (!ret)
	{
		if (setup_get_running_exe_filename(SETUP_EVERYTHING_TASKBAR_NOTIFICATION_1_5_ALPHA, everything_exe_filename_wbuf))
		{
			ret = 1;
		}
	}

	if (!ret)
	{
		if (setup_browse_for_folder(everything_exe_filename_wbuf))
		{
			ret = 1;
		}
	}

	return ret;
}

void setup_fatal(DWORD error_code, const char* msg)
{
	wchar_t title_wbuf[SETUP_MAX_STRING];
	wchar_t msg_wbuf[SETUP_MAX_STRING];

	title_wbuf[0] = 0;
	msg_wbuf[0] = 0;

	setup_cat_astring(title_wbuf, SETUP_MAX_STRING, "Setup Everything Plug-in");
	setup_cat_astring(msg_wbuf, SETUP_MAX_STRING, "Error ");
	setup_cat_number(msg_wbuf, SETUP_MAX_STRING, error_code);
	setup_cat_astring(msg_wbuf, SETUP_MAX_STRING, ": ");
	setup_cat_astring(msg_wbuf, SETUP_MAX_STRING, msg);

	MessageBoxW(0, msg_wbuf, title_wbuf, MB_ICONERROR | MB_OK);

	ExitProcess(1);
}

void* setup_alloc(DWORD size)
{
	void* p;

	p = HeapAlloc(GetProcessHeap(), 0, size);
	if (!p)
	{
		setup_fatal(ERROR_OUTOFMEMORY, "Out of memory");
	}

	return p;
}

void setup_free(void* ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}

void setup_cat_astring(wchar_t* wbuf, DWORD size_in_wchars, const char* as)
{
	wchar_t* d;
	const char* p;
	DWORD run;

	run = size_in_wchars - 1;
	d = wbuf;

	while (run)
	{
		if (!*d)
		{
			break;
		}

		d++;
		run--;
	}

	p = as;
	while (run)
	{
		if (!*p)
		{
			break;
		}

		*d++ = *p;

		p++;
		run--;
	}

	*d = 0;
}

void setup_cat_wstring(wchar_t* wbuf, DWORD size_in_wchars, const wchar_t* ws)
{
	wchar_t* d;
	const wchar_t* p;
	DWORD run;

	run = size_in_wchars - 1;
	d = wbuf;

	while (run)
	{
		if (!*d)
		{
			break;
		}

		d++;
		run--;
	}

	p = ws;
	while (run)
	{
		if (!*p)
		{
			break;
		}

		*d++ = *p;

		p++;
		run--;
	}

	*d = 0;
}

void setup_cat_number(wchar_t* wbuf, DWORD size_in_wchars, DWORD number)
{
	char number_buf[SETUP_MAX_STRING];
	char* d;

	d = number_buf + SETUP_MAX_STRING;
	d--;
	*d = 0;

	if (number)
	{
		while (number)
		{
			int i;

			i = number % 10;

			d--;
			*d = '0' + i;

			number /= 10;
		}
	}
	else
	{
		d--;
		*d = '0';
	}

	setup_cat_astring(wbuf, size_in_wchars, d);
}

#pragma function(memset)
void* __cdecl memset(void* p, int v, size_t s) {
	unsigned char* d;

	d = p;
	while (s)
	{
		*d++ = v;

		s--;
	}

	return d;
}

void setup_main(void)
{
	wchar_t everything_exe_filename_wbuf[SETUP_MAX_STRING];

	// required for SHBrowseForFolder
	OleInitialize(0);

	// get everything install folder
	if (setup_get_everything_exe_filename(everything_exe_filename_wbuf))
	{
		wchar_t setup_exe_filename_wbuf[SETUP_MAX_STRING];

		if (GetModuleFileNameW(NULL, setup_exe_filename_wbuf, SETUP_MAX_STRING))
		{
			wchar_t* command_line_wbuf;
			STARTUPINFO startup_info;
			PROCESS_INFORMATION process_information;

			command_line_wbuf = setup_alloc(SETUP_MAX_COMMAND_LINE * sizeof(wchar_t));

			*command_line_wbuf = 0;

			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, "\"");
			setup_cat_wstring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, everything_exe_filename_wbuf);
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, "\" ");

			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin \""SETUP_DLL_NAME"\"");

			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-exe \"");
			setup_cat_wstring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, setup_exe_filename_wbuf);
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, "\"");

			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-resource-id \"");
			setup_cat_number(command_line_wbuf, SETUP_MAX_COMMAND_LINE, IDR_DLL_BZ2);
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, "\"");

			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-name \""SETUP_NAME"\"");
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-description \""SETUP_DESCRIPTION"\"");
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-author \""SETUP_AUTHOR"\"");
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-version \""SETUP_VERSION"\"");
			setup_cat_astring(command_line_wbuf, SETUP_MAX_COMMAND_LINE, " -setup-plugin-link \""SETUP_LINK"\"");

			ZeroMemory(&startup_info, sizeof(STARTUPINFO));
			startup_info.cb = sizeof(STARTUPINFO);

			if (CreateProcessW(everything_exe_filename_wbuf, command_line_wbuf, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_information))
			{
				// Everything handles any errors from here.
				CloseHandle(process_information.hThread);
				CloseHandle(process_information.hProcess);
			}
			else
			{
				setup_fatal(GetLastError(), "Unable to start Everything.");
			}

			setup_free(command_line_wbuf);
		}
		else
		{
			setup_fatal(GetLastError(), "Unable to get setup filename.");
		}
	}
	else
	{
		setup_fatal(ERROR_FILE_NOT_FOUND, "Unable to find Everything.\n\nPlease make sure Everything is installed or running.");
	}
}

void __cdecl WinMainCRTStartup()
{
	setup_main();

	ExitProcess(0);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	setup_main();

	ExitProcess(0);
}
