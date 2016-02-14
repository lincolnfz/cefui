// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include <Shlwapi.h>

extern WCHAR g_szLocalPath[MAX_PATH];

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
	}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

