// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2012-2016 - TortoiseGit
// Copyright (C) 2003-2008 - Stefan Kueng

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
#pragma once
#include "registry.h"
#include "Globals.h"
#include "GitAdminDir.h"
#include "Git.h"

#define REGISTRYTIMEOUT 2000
#define EXCLUDELISTTIMEOUT 5000
#define ADMINDIRTIMEOUT 10000
#define DRIVETYPETIMEOUT 300000		// 5 min
#define MENUTIMEOUT 100

#define DEFAULTMENUTOPENTRIES	MENUSYNC|MENUCREATEREPOS|MENUCLONE|MENUCOMMIT
#define DEFAULTMENUEXTENTRIES	MENUSVNIGNORE|MENUSTASHAPPLY|MENUSUBSYNC

typedef CComCritSecLock<CComCriticalSection> Locker;

/**
 * \ingroup TortoiseShell
 * Helper class which caches access to the registry. Also provides helper methods
 * for checks against the settings stored in the registry.
 */
class ShellCache
{
public:
	enum CacheType
	{
		none,
		exe,
		dll,
		dllFull,// same as dll except it uses commandline git tool with all status modes supported
	};
	ShellCache()
	{
		cachetype = CRegStdDWORD(L"Software\\TortoiseGit\\CacheType", GetSystemMetrics(SM_REMOTESESSION) ? dll : exe);
		showrecursive = CRegStdDWORD(L"Software\\TortoiseGit\\RecursiveOverlay", TRUE);
		folderoverlay = CRegStdDWORD(L"Software\\TortoiseGit\\FolderOverlay", TRUE);
		driveremote = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskRemote");
		drivefixed = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskFixed", TRUE);
		drivecdrom = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskCDROM");
		driveremove = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskRemovable");
		drivefloppy = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskFloppy");
		driveram = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskRAM");
		driveunknown = CRegStdDWORD(L"Software\\TortoiseGit\\DriveMaskUnknown");
		shellmenuaccelerators = CRegStdDWORD(L"Software\\TortoiseGit\\ShellMenuAccelerators", TRUE);
		excludelist = CRegStdString(L"Software\\TortoiseGit\\OverlayExcludeList");
		includelist = CRegStdString(L"Software\\TortoiseGit\\OverlayIncludeList");
		simplecontext = CRegStdDWORD(L"Software\\TortoiseGit\\SimpleContext", FALSE);
		unversionedasmodified = CRegStdDWORD(L"Software\\TortoiseGit\\UnversionedAsModified", FALSE);
		recursesubmodules = CRegStdDWORD(L"Software\\TortoiseGit\\TGitCacheRecurseSubmodules", FALSE);
		hidemenusforunversioneditems = CRegStdDWORD(L"Software\\TortoiseGit\\HideMenusForUnversionedItems", FALSE);
		showunversionedoverlay = CRegStdDWORD(L"Software\\TortoiseGit\\ShowUnversionedOverlay", TRUE);
		showignoredoverlay = CRegStdDWORD(L"Software\\TortoiseGit\\ShowIgnoredOverlay", TRUE);
		getlocktop = CRegStdDWORD(L"Software\\TortoiseGit\\GetLockTop", TRUE);
		excludedasnormal = CRegStdDWORD(L"Software\\TortoiseGit\\ShowExcludedAsNormal", TRUE);
		cachetypeticker = GetTickCount64();
		recursiveticker = cachetypeticker;
		folderoverlayticker = cachetypeticker;
		driveticker = cachetypeticker;
		drivetypeticker = cachetypeticker;
		langticker = cachetypeticker;
		excludelistticker = cachetypeticker;
		includelistticker = cachetypeticker;
		simplecontextticker = cachetypeticker;
		shellmenuacceleratorsticker = cachetypeticker;
		unversionedasmodifiedticker = cachetypeticker;
		recursesubmodulesticker = cachetypeticker;
		showunversionedoverlayticker = cachetypeticker;
		showignoredoverlayticker = cachetypeticker;
		admindirticker = cachetypeticker;
		getlocktopticker = cachetypeticker;
		excludedasnormalticker = cachetypeticker;
		hidemenusforunversioneditemsticker = cachetypeticker;
		excontextticker = cachetypeticker;

		unsigned __int64 entries = (DEFAULTMENUTOPENTRIES);
		menulayoutlow = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuEntries",	  entries&0xFFFFFFFF);
		menulayouthigh = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuEntrieshigh", entries>>32);
		layoutticker = cachetypeticker;

		unsigned __int64 ext = (DEFAULTMENUEXTENTRIES);
		menuextlow	= CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuExtEntriesLow", ext&0xFFFFFFFF  );
		menuexthigh = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuExtEntriesHigh",	ext>>32	  );
		exticker = cachetypeticker;

		menumasklow_lm = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuEntriesMaskLow", 0, FALSE, HKEY_LOCAL_MACHINE);
		menumaskhigh_lm = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuEntriesMaskHigh", 0, FALSE, HKEY_LOCAL_MACHINE);
		menumasklow_cu = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuEntriesMaskLow", 0);
		menumaskhigh_cu = CRegStdDWORD(L"Software\\TortoiseGit\\ContextMenuEntriesMaskHigh", 0);
		menumaskticker = cachetypeticker;
		langid = CRegStdDWORD(L"Software\\TortoiseGit\\LanguageID", 1033);
		blockstatus = CRegStdDWORD(L"Software\\TortoiseGit\\BlockStatus", 0);
		blockstatusticker = cachetypeticker;
		for (int i = 0; i < 27; ++i)
		{
			drivetypecache[i] = (UINT)-1;
		}
		// A: and B: are floppy disks
		drivetypecache[0] = DRIVE_REMOVABLE;
		drivetypecache[1] = DRIVE_REMOVABLE;
		drivetypepathcache[0] = 0;
		sAdminDirCacheKey.reserve(MAX_PATH);		// MAX_PATH as buffer reservation ok.
		nocontextpaths = CRegStdString(L"Software\\TortoiseGit\\NoContextPaths", L"");
		m_critSec.Init();
	}
	void ForceRefresh()
	{
		cachetype.read();
		showrecursive.read();
		folderoverlay.read();
		driveremote.read();
		drivefixed.read();
		drivecdrom.read();
		driveremove.read();
		drivefloppy.read();
		driveram.read();
		driveunknown.read();
		excludelist.read();
		includelist.read();
		simplecontext.read();
		shellmenuaccelerators.read();
		unversionedasmodified.read();
		recursesubmodules.read();
		showunversionedoverlay.read();
		showignoredoverlay.read();
		excludedasnormal.read();
		hidemenusforunversioneditems.read();
		menulayoutlow.read();
		menulayouthigh.read();
		langid.read();
		blockstatus.read();
		getlocktop.read();
		menumasklow_lm.read();
		menumaskhigh_lm.read();
		menumasklow_cu.read();
		menumaskhigh_cu.read();
		nocontextpaths.read();
	}
	CacheType GetCacheType()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > cachetypeticker)
		{
			cachetypeticker = GetTickCount64();
			cachetype.read();
		}
		return CacheType(DWORD((cachetype)));
	}
	DWORD BlockStatus()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > blockstatusticker)
		{
			blockstatusticker = GetTickCount64();
			blockstatus.read();
		}
		return (blockstatus);
	}
	unsigned __int64 GetMenuLayout()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > layoutticker)
		{
			layoutticker = GetTickCount64();
			menulayoutlow.read();
			menulayouthigh.read();
		}
		unsigned __int64 temp = unsigned __int64(DWORD(menulayouthigh))<<32;
		temp |= unsigned __int64(DWORD(menulayoutlow));
		return temp;
	}

	unsigned __int64 GetMenuExt()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > exticker)
		{
			exticker = GetTickCount64();
			menuextlow.read();
			menuexthigh.read();
		}
		unsigned __int64 temp = unsigned __int64(DWORD(menuexthigh))<<32;
		temp |= unsigned __int64(DWORD(menuextlow));
		return temp;
	}

	unsigned __int64 GetMenuMask()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > menumaskticker)
		{
			menumaskticker = GetTickCount64();
			menumasklow_lm.read();
			menumaskhigh_lm.read();
			menumasklow_cu.read();
			menumaskhigh_cu.read();
		}
		DWORD low = (DWORD)menumasklow_lm | (DWORD)menumasklow_cu;
		DWORD high = (DWORD)menumaskhigh_lm | (DWORD)menumaskhigh_cu;
		unsigned __int64 temp = unsigned __int64(high)<<32;
		temp |= unsigned __int64(low);
		return temp;
	}
	BOOL IsRecursive()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > recursiveticker)
		{
			recursiveticker = GetTickCount64();
			showrecursive.read();
		}
		return (showrecursive);
	}
	BOOL IsFolderOverlay()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > folderoverlayticker)
		{
			folderoverlayticker = GetTickCount64();
			folderoverlay.read();
		}
		return (folderoverlay);
	}
	BOOL IsSimpleContext()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > simplecontextticker)
		{
			simplecontextticker = GetTickCount64();
			simplecontext.read();
		}
		return (simplecontext!=0);
	}
	BOOL HasShellMenuAccelerators()
	{
		if ((GetTickCount64() - shellmenuacceleratorsticker) > REGISTRYTIMEOUT)
		{
			shellmenuacceleratorsticker = GetTickCount64();
			shellmenuaccelerators.read();
		}
		return (shellmenuaccelerators!=0);
	}
	BOOL IsUnversionedAsModified()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > unversionedasmodifiedticker)
		{
			unversionedasmodifiedticker = GetTickCount64();
			unversionedasmodified.read();
		}
		return (unversionedasmodified);
	}
	BOOL IsRecurseSubmodules()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > recursesubmodulesticker)
		{
			recursesubmodulesticker = GetTickCount64();
			recursesubmodules.read();
		}
		return (recursesubmodules);
	}
	BOOL ShowUnversionedOverlay()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > showunversionedoverlayticker)
		{
			showunversionedoverlayticker = GetTickCount64();
			showunversionedoverlay.read();
		}
		return (showunversionedoverlay);
	}
	BOOL ShowIgnoredOverlay()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > showignoredoverlayticker)
		{
			showignoredoverlayticker = GetTickCount64();
			showignoredoverlay.read();
		}
		return (showignoredoverlay);
	}
	BOOL IsGetLockTop()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > getlocktopticker)
		{
			getlocktopticker = GetTickCount64();
			getlocktop.read();
		}
		return (getlocktop);
	}
	BOOL ShowExcludedAsNormal()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > excludedasnormalticker)
		{
			excludedasnormalticker = GetTickCount64();
			excludedasnormal.read();
		}
		return (excludedasnormal);
	}
	BOOL HideMenusForUnversionedItems()
	{
	if ((GetTickCount64() - hidemenusforunversioneditemsticker) > REGISTRYTIMEOUT)
		{
			hidemenusforunversioneditemsticker = GetTickCount64();
			hidemenusforunversioneditems.read();
		}
		return (hidemenusforunversioneditems);
	}
	BOOL IsRemote()
	{
		DriveValid();
		return (driveremote);
	}
	BOOL IsFixed()
	{
		DriveValid();
		return (drivefixed);
	}
	BOOL IsCDRom()
	{
		DriveValid();
		return (drivecdrom);
	}
	BOOL IsRemovable()
	{
		DriveValid();
		return (driveremove);
	}
	BOOL IsRAM()
	{
		DriveValid();
		return (driveram);
	}
	BOOL IsUnknown()
	{
		DriveValid();
		return (driveunknown);
	}
	BOOL IsContextPathAllowed(LPCWSTR path)
	{
		Locker lock(m_critSec);
		ExcludeContextValid();
		for (const auto& exPath : excontextvector)
		{
			if (exPath.empty())
				continue;
			if (exPath.at(exPath.size() - 1) == '*')
			{
				std::wstring str = exPath.substr(0, exPath.size() - 1);
				if (_wcsnicmp(str.c_str(), path, str.size())==0)
					return FALSE;
			}
			else if (_wcsicmp(exPath.c_str(), path) == 0)
				return FALSE;
		}
		return TRUE;
	}
	BOOL IsPathAllowed(LPCWSTR path)
	{
		Locker lock(m_critSec);
		IncludeListValid();
		for (const auto& pathAllowed : invector)
		{
			if (pathAllowed.empty())
				continue;
			if (pathAllowed.at(pathAllowed.size() - 1) == '*')
			{
				std::wstring str = pathAllowed.substr(0, pathAllowed.size() - 1);
				if (_wcsnicmp(str.c_str(), path, str.size())==0)
					return TRUE;
				if (!str.empty() && (str.at(str.size()-1) == '\\') && (_wcsnicmp(str.c_str(), path, str.size()-1)==0))
					return TRUE;
			}
			else if (_wcsicmp(pathAllowed.c_str(), path) == 0)
				return TRUE;
			else if ((pathAllowed.at(pathAllowed.size() - 1) == '\\') &&
				((_wcsnicmp(pathAllowed.c_str(), path, pathAllowed.size()) == 0) || (_wcsicmp(pathAllowed.c_str(), path) == 0)))
				return TRUE;

		}
		UINT drivetype = 0;
		int drivenumber = PathGetDriveNumber(path);
		if ((drivenumber >=0)&&(drivenumber < 25))
		{
			drivetype = drivetypecache[drivenumber];
			if ((drivetype == -1)|| ((GetTickCount64() - DRIVETYPETIMEOUT) > drivetypeticker))
			{
				if ((drivenumber == 0)||(drivenumber == 1))
					drivetypecache[drivenumber] = DRIVE_REMOVABLE;
				else
				{
					drivetypeticker = GetTickCount64();
					TCHAR pathbuf[MAX_PATH+4] = {0};		// MAX_PATH ok here. PathStripToRoot works with partial paths too.
					wcsncpy_s(pathbuf, MAX_PATH+4, path, MAX_PATH+3);
					PathStripToRoot(pathbuf);
					PathAddBackslash(pathbuf);
					CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": GetDriveType for %s, Drive %d\n", pathbuf, drivenumber);
					drivetype = GetDriveType(pathbuf);
					drivetypecache[drivenumber] = drivetype;
				}
			}
		}
		else
		{
			TCHAR pathbuf[MAX_PATH+4] = {0};		// MAX_PATH ok here. PathIsUNCServer works with partial paths too.
			wcsncpy_s(pathbuf, MAX_PATH+4, path, MAX_PATH+3);
			if (PathIsUNCServer(pathbuf))
				drivetype = DRIVE_REMOTE;
			else
			{
				PathStripToRoot(pathbuf);
				PathAddBackslash(pathbuf);
				if (wcsncmp(pathbuf, drivetypepathcache, MAX_PATH-1)==0)		// MAX_PATH ok.
					drivetype = drivetypecache[26];
				else
				{
					CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L"GetDriveType for %s\n", pathbuf);
					drivetype = GetDriveType(pathbuf);
					drivetypecache[26] = drivetype;
					wcsncpy_s(drivetypepathcache, MAX_PATH, pathbuf, MAX_PATH - 1);			// MAX_PATH ok.
				}
			}
		}
		if ((drivetype == DRIVE_REMOVABLE)&&(!IsRemovable()))
			return FALSE;
		if ((drivetype == DRIVE_REMOVABLE)&&(drivefloppy == 0)&&((drivenumber==0)||(drivenumber==1)))
			return FALSE;
		if ((drivetype == DRIVE_FIXED)&&(!IsFixed()))
			return FALSE;
		if (((drivetype == DRIVE_REMOTE)||(drivetype == DRIVE_NO_ROOT_DIR))&&(!IsRemote()))
			return FALSE;
		if ((drivetype == DRIVE_CDROM)&&(!IsCDRom()))
			return FALSE;
		if ((drivetype == DRIVE_RAMDISK)&&(!IsRAM()))
			return FALSE;
		if ((drivetype == DRIVE_UNKNOWN)&&(IsUnknown()))
			return FALSE;

		ExcludeListValid();
		for (const auto& exPath : exvector)
		{
			if (exPath.empty())
				continue;
			if (exPath.at(exPath.size() - 1) == '*')
			{
				std::wstring str = exPath.substr(0, exPath.size() - 1);
				if (_wcsnicmp(str.c_str(), path, str.size())==0)
					return FALSE;
			}
			else if (_wcsicmp(exPath.c_str(), path) == 0)
				return FALSE;
		}
		return TRUE;
	}
	DWORD GetLangID()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > langticker)
		{
			langticker = GetTickCount64();
			langid.read();
		}
		return (langid);
	}
	BOOL HasGITAdminDir(LPCWSTR path, BOOL bIsDir, CString* ProjectTopDir = nullptr)
	{
		size_t len = wcslen(path);
		auto buf = std::make_unique<TCHAR[]>(len + 1);
		wcscpy_s(buf.get(), len + 1, path);
		if (! bIsDir)
		{
			TCHAR * ptr = wcsrchr(buf.get(), '\\');
			if (ptr != L'\0')
				*ptr = L'\0';
		}
		if ((GetTickCount64() - ADMINDIRTIMEOUT) < admindirticker)
		{
			std::map<std::wstring, AdminDir_s>::iterator iter;
			sAdminDirCacheKey.assign(buf.get());
			if ((iter = admindircache.find(sAdminDirCacheKey)) != admindircache.end())
			{
				if (ProjectTopDir && iter->second.bHasAdminDir)
					*ProjectTopDir = iter->second.sProjectRoot.c_str();
				return iter->second.bHasAdminDir;
			}
		}
		CString sProjectRoot;
		BOOL hasAdminDir = GitAdminDir::HasAdminDir(buf.get(), true, &sProjectRoot);
		admindirticker = GetTickCount64();
		Locker lock(m_critSec);

		AdminDir_s &ad = admindircache[buf.get()];
		ad.bHasAdminDir = hasAdminDir;
		if (hasAdminDir)
		{
			ad.sProjectRoot.assign(sProjectRoot);

			if (ProjectTopDir)
				*ProjectTopDir = sProjectRoot;
		}

		return hasAdminDir;
	}
private:
	void DriveValid()
	{
		if ((GetTickCount64() - REGISTRYTIMEOUT) > driveticker)
		{
			driveticker = GetTickCount64();
			driveremote.read();
			drivefixed.read();
			drivecdrom.read();
			driveremove.read();
			drivefloppy.read();
		}
	}
	void ExcludeContextValid()
	{
		if ((GetTickCount64() - EXCLUDELISTTIMEOUT) > excontextticker)
		{
			Locker lock(m_critSec);
			excontextticker = GetTickCount64();
			nocontextpaths.read();
			if (excludecontextstr.compare((std::wstring)nocontextpaths)==0)
				return;
			excludecontextstr = (std::wstring)nocontextpaths;
			excontextvector.clear();
			size_t pos = 0, pos_ant = 0;
			pos = excludecontextstr.find(L'\n', pos_ant);
			while (pos != std::wstring::npos)
			{
				std::wstring token = excludecontextstr.substr(pos_ant, pos-pos_ant);
				excontextvector.push_back(token);
				pos_ant = pos+1;
				pos = excludecontextstr.find(L'\n', pos_ant);
			}
			if (!excludecontextstr.empty())
			{
				excontextvector.push_back(excludecontextstr.substr(pos_ant, excludecontextstr.size()-1));
			}
			excludecontextstr = (std::wstring)nocontextpaths;
		}
	}
	void ExcludeListValid()
	{
		if ((GetTickCount64() - EXCLUDELISTTIMEOUT) > excludelistticker)
		{
			Locker lock(m_critSec);
			excludelistticker = GetTickCount64();
			excludelist.read();
			if (excludeliststr.compare((std::wstring)excludelist)==0)
				return;
			excludeliststr = (std::wstring)excludelist;
			exvector.clear();
			size_t pos = 0, pos_ant = 0;
			pos = excludeliststr.find(L'\n', pos_ant);
			while (pos != std::wstring::npos)
			{
				std::wstring token = excludeliststr.substr(pos_ant, pos-pos_ant);
				exvector.push_back(token);
				pos_ant = pos+1;
				pos = excludeliststr.find(L'\n', pos_ant);
			}
			if (!excludeliststr.empty())
				exvector.push_back(excludeliststr.substr(pos_ant, excludeliststr.size()-1));
			excludeliststr = (std::wstring)excludelist;
		}
	}
	void IncludeListValid()
	{
		if ((GetTickCount64() - EXCLUDELISTTIMEOUT) > includelistticker)
		{
			Locker lock(m_critSec);
			includelistticker = GetTickCount64();
			includelist.read();
			if (includeliststr.compare((std::wstring)includelist)==0)
				return;
			includeliststr = (std::wstring)includelist;
			invector.clear();
			size_t pos = 0, pos_ant = 0;
			pos = includeliststr.find(L'\n', pos_ant);
			while (pos != std::wstring::npos)
			{
				std::wstring token = includeliststr.substr(pos_ant, pos-pos_ant);
				invector.push_back(token);
				pos_ant = pos+1;
				pos = includeliststr.find(L'\n', pos_ant);
			}
			if (!includeliststr.empty())
				invector.push_back(includeliststr.substr(pos_ant, includeliststr.size()-1));
			includeliststr = (std::wstring)includelist;
		}
	}

	struct AdminDir_s
	{
		BOOL bHasAdminDir;
		std::wstring sProjectRoot;
	};
public:
	CRegStdDWORD cachetype;
	CRegStdDWORD blockstatus;
	CRegStdDWORD langid;
	CRegStdDWORD showrecursive;
	CRegStdDWORD folderoverlay;
	CRegStdDWORD getlocktop;
	CRegStdDWORD driveremote;
	CRegStdDWORD drivefixed;
	CRegStdDWORD drivecdrom;
	CRegStdDWORD driveremove;
	CRegStdDWORD drivefloppy;
	CRegStdDWORD driveram;
	CRegStdDWORD driveunknown;
	CRegStdDWORD menulayoutlow; /* Fist level mask */
	CRegStdDWORD menulayouthigh;
	CRegStdDWORD shellmenuaccelerators;
	CRegStdDWORD menuextlow;	   /* ext menu mask */
	CRegStdDWORD menuexthigh;
	CRegStdDWORD simplecontext;
	CRegStdDWORD menumasklow_lm;
	CRegStdDWORD menumaskhigh_lm;
	CRegStdDWORD menumasklow_cu;
	CRegStdDWORD menumaskhigh_cu;
	CRegStdDWORD unversionedasmodified;
	CRegStdDWORD recursesubmodules;
	CRegStdDWORD showunversionedoverlay;
	CRegStdDWORD showignoredoverlay;
	CRegStdDWORD excludedasnormal;
	CRegStdString excludelist;
	CRegStdDWORD hidemenusforunversioneditems;
	std::wstring excludeliststr;
	std::vector<std::wstring> exvector;
	CRegStdString includelist;
	std::wstring includeliststr;
	std::vector<std::wstring> invector;
	ULONGLONG cachetypeticker;
	ULONGLONG recursiveticker;
	ULONGLONG folderoverlayticker;
	ULONGLONG getlocktopticker;
	ULONGLONG driveticker;
	ULONGLONG drivetypeticker;
	ULONGLONG layoutticker;
	ULONGLONG exticker;
	ULONGLONG menumaskticker;
	ULONGLONG langticker;
	ULONGLONG blockstatusticker;
	ULONGLONG excludelistticker;
	ULONGLONG includelistticker;
	ULONGLONG simplecontextticker;
	ULONGLONG shellmenuacceleratorsticker;
	ULONGLONG unversionedasmodifiedticker;
	ULONGLONG recursesubmodulesticker;
	ULONGLONG showunversionedoverlayticker;
	ULONGLONG showignoredoverlayticker;
	ULONGLONG excludedasnormalticker;
	ULONGLONG hidemenusforunversioneditemsticker;
	UINT  drivetypecache[27];
	TCHAR drivetypepathcache[MAX_PATH];		// MAX_PATH ok.
	TCHAR szDecSep[5];
	TCHAR szThousandsSep[5];
	std::map<std::wstring, AdminDir_s> admindircache;
	std::wstring sAdminDirCacheKey;
	CRegStdString nocontextpaths;
	std::wstring excludecontextstr;
	std::vector<std::wstring> excontextvector;
	ULONGLONG excontextticker;
	ULONGLONG admindirticker;
	CComCriticalSection m_critSec;
};
