// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <Shlwapi.h>
#include <tchar.h>

extern WCHAR g_szLocalPath[MAX_PATH];

extern bool bDebug_Dev;

//#pragma comment(lib, "libcef.dll.lib")
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:{
		GetModuleFileName(hModule, g_szLocalPath, MAX_PATH);
		PathRemoveFileSpec(g_szLocalPath);
		WCHAR szDebugFlag[512];
		swprintf_s(szDebugFlag, L"%s\\debug.dbg", g_szLocalPath);
		bDebug_Dev = (_taccess_s(szDebugFlag, 0) == 0);
	}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

