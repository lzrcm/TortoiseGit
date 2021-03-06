// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2012-2014, 2016 - TortoiseGit
// Copyright (C) 2007, 2009, 2013-2014 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "../targetver.h"
#include <Windows.h>
#include <tchar.h>
#include "Debug.h"

const HINSTANCE NIL = (HINSTANCE)((char*)(0)-1);

static HINSTANCE hInst = nullptr;

static HINSTANCE hTortoiseGit = nullptr;
static LPFNGETCLASSOBJECT pDllGetClassObject = nullptr;
static LPFNCANUNLOADNOW pDllCanUnloadNow = nullptr;

static BOOL DebugActive(void)
{
	static const WCHAR TGitRootKey[]=_T("Software\\TortoiseGit");
	static const WCHAR ExplorerOnlyValue[]=_T("DebugShell");

	DWORD bDebug = 0;

	HKEY hKey = HKEY_CURRENT_USER;
	LONG Result = ERROR;
	DWORD Type = REG_DWORD;
	DWORD Len = sizeof(DWORD);

	BOOL bDebugActive = FALSE;


	TRACE(_T("DebugActive() - Enter\n"));

	if (IsDebuggerPresent())
	{
		Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TGitRootKey, 0, KEY_READ, &hKey);
		if (Result == ERROR_SUCCESS)
		{
			Result = RegQueryValueEx(hKey, ExplorerOnlyValue, nullptr, &Type, (BYTE*)&bDebug, &Len);
			if ((Result == ERROR_SUCCESS) && (Type == REG_DWORD) && (Len == sizeof(DWORD)) && bDebug)
			{
				TRACE(_T("DebugActive() - debug active\n"));
				bDebugActive = TRUE;
			}

			RegCloseKey(hKey);
		}
	}

	TRACE(_T("WantRealVersion() - Exit\n"));
	return bDebugActive;
}

/**
 * \ingroup TortoiseShell
 * Check whether to load the full TortoiseGit.dll or not.
 */
static BOOL WantRealVersion(void)
{
	static const WCHAR TGitRootKey[]=_T("Software\\TortoiseGit");
	static const WCHAR ExplorerOnlyValue[]=_T("LoadDllOnlyInExplorer");

	static const WCHAR ExplorerEnvPath[]=_T("%SystemRoot%\\explorer.exe");


	DWORD bExplorerOnly = 0;
	WCHAR ModuleName[MAX_PATH] = {0};

	HKEY hKey = HKEY_CURRENT_USER;
	DWORD Type = REG_DWORD;
	DWORD Len = sizeof(DWORD);

	BOOL bWantReal = TRUE;

	TRACE(_T("WantRealVersion() - Enter\n"));

	LONG Result = RegOpenKeyEx(HKEY_CURRENT_USER, TGitRootKey, 0, KEY_READ, &hKey);
	if (Result == ERROR_SUCCESS)
	{
		Result = RegQueryValueEx(hKey, ExplorerOnlyValue, nullptr, &Type, (BYTE*)&bExplorerOnly, &Len);
		if ((Result == ERROR_SUCCESS) && (Type == REG_DWORD) && (Len == sizeof(DWORD)) && bExplorerOnly)
		{
			TRACE(_T("WantRealVersion() - Explorer Only\n"));

			// check if the current process is in fact the explorer
			Len = GetModuleFileName(nullptr, ModuleName, _countof(ModuleName));
			if (Len)
			{
				TRACE(_T("Process is %s\n"), ModuleName);

				WCHAR ExplorerPath[MAX_PATH] = {0};
				Len = ExpandEnvironmentStrings(ExplorerEnvPath, ExplorerPath, _countof(ExplorerPath));
				if (Len && (Len <= _countof(ExplorerPath)))
				{
					TRACE(_T("Explorer path is %s\n"), ExplorerPath);
					bWantReal = !lstrcmpi(ModuleName, ExplorerPath);
				}

				// we also have to allow the verclsid.exe process - that process determines
				// first whether the shell is allowed to even use an extension.
				Len = lstrlen(ModuleName);
				if ((Len > 13)&&(lstrcmpi(&ModuleName[Len-13], _T("\\verclsid.exe")) == 0))
					bWantReal = TRUE;
			}
		}

		RegCloseKey(hKey);
	}

	TRACE(_T("WantRealVersion() - Exit\n"));
	return bWantReal;
}

static void LoadRealLibrary(void)
{
	static const char GetClassObject[] = "DllGetClassObject";
	static const char CanUnloadNow[] = "DllCanUnloadNow";

	WCHAR ModuleName[MAX_PATH] = {0};
	DWORD Len = 0;
	HINSTANCE hUseInst = hInst;

	if (hTortoiseGit)
		return;

	if (!WantRealVersion())
	{
		TRACE(_T("LoadRealLibrary() - Bypass\n"));
		hTortoiseGit = NIL;
		return;
	}
	// if HKCU\Software\TortoiseGit\DebugShell is set, load the dlls from the location of the current process
	// which is for our debug purposes an instance of usually TortoiseProc. That way we can force the load
	// of the debug dlls.
	if (DebugActive())
		hUseInst = nullptr;
	Len = GetModuleFileName(hUseInst, ModuleName, _countof(ModuleName));
	if (!Len)
	{
		TRACE(_T("LoadRealLibrary() - Fail\n"));
		hTortoiseGit = NIL;
		return;
	}

	// truncate the string at the last '\' char
	while(Len > 0)
	{
		--Len;
		if (ModuleName[Len] == '\\')
		{
			ModuleName[Len] = '\0';
			break;
		}
	}
	if (Len == 0)
	{
		TRACE(_T("LoadRealLibrary() - Fail\n"));
		hTortoiseGit = NIL;
		return;
	}
#ifdef _WIN64
	lstrcat(ModuleName, _T("\\TortoiseGit.dll"));
#else
	lstrcat(ModuleName, _T("\\TortoiseGit32.dll"));
#endif
	TRACE(_T("LoadRealLibrary() - Load %s\n"), ModuleName);

	hTortoiseGit = LoadLibraryEx(ModuleName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!hTortoiseGit)
	{
		TRACE(_T("LoadRealLibrary() - Fail\n"));
		hTortoiseGit = NIL;
		return;
	}

	TRACE(_T("LoadRealLibrary() - Success\n"));
	pDllGetClassObject = nullptr;
	pDllCanUnloadNow = nullptr;
	pDllGetClassObject = (LPFNGETCLASSOBJECT)GetProcAddress(hTortoiseGit, GetClassObject);
	if (!pDllGetClassObject)
	{
		TRACE(_T("LoadRealLibrary() - Fail\n"));
		FreeLibrary(hTortoiseGit);
		hTortoiseGit = NIL;
		return;
	}
	pDllCanUnloadNow = (LPFNCANUNLOADNOW)GetProcAddress(hTortoiseGit, CanUnloadNow);
	if (!pDllCanUnloadNow)
	{
		TRACE(_T("LoadRealLibrary() - Fail\n"));
		FreeLibrary(hTortoiseGit);
		hTortoiseGit = NIL;
		return;
	}
}

static void UnloadRealLibrary(void)
{
	if (!hTortoiseGit)
		return;

	if (hTortoiseGit != NIL)
		FreeLibrary(hTortoiseGit);

	hTortoiseGit = nullptr;
	pDllGetClassObject = nullptr;
	pDllCanUnloadNow = nullptr;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD Reason, LPVOID /*Reserved*/)
{
#ifdef _DEBUG
	// if no debugger is present, then don't load the dll.
	// this prevents other apps from loading the dll and locking
	// it.

	BOOL bInShellTest = FALSE;
	TCHAR buf[MAX_PATH + 1] = {0};       // MAX_PATH ok, the test really is for debugging anyway.
	DWORD pathLength = GetModuleFileName(nullptr, buf, MAX_PATH);

	if (pathLength >= 14)
	{
		if ((lstrcmpi(&buf[pathLength-14], _T("\\ShellTest.exe"))) == 0)
		{
			bInShellTest = TRUE;
		}
		if ((_tcsicmp(&buf[pathLength-13], _T("\\verclsid.exe"))) == 0)
		{
			bInShellTest = TRUE;
		}
	}

	if (!IsDebuggerPresent() && !bInShellTest)
	{
		TRACE(_T("In debug load preventer\n"));
		return FALSE;
	}
#endif

	switch(Reason)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hInstance;
		break;

	/*case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;*/
	}

	return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	TRACE(_T("DllGetClassObject() - Enter\n"));

	LoadRealLibrary();
	if (!pDllGetClassObject)
	{
		if (ppv)
			*ppv = nullptr;

		TRACE(_T("DllGetClassObject() - Bypass\n"));
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	TRACE(_T("DllGetClassObject() - Forward\n"));
	return pDllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
	TRACE(_T("DllCanUnloadNow() - Enter\n"));

	if (pDllCanUnloadNow)
	{
		TRACE(_T("DllCanUnloadNow() - Forward\n"));
		HRESULT Result = pDllCanUnloadNow();
		if (Result != S_OK)
			return Result;
	}

	TRACE(_T("DllCanUnloadNow() - Unload\n"));
	UnloadRealLibrary();
	return S_OK;
}

