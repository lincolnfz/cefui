#include "stdafx.h"
#include "ShareHelper.h"
#include <regex>

namespace cyjh{

	std::string x_funName(char* name){
		std::string out;
		std::string mathstr(name);
		std::tr1::regex pattern("::\\S+");
		std::smatch result;
		bool match = std::regex_search(mathstr, result, pattern);
		if (match){
			std::ssub_match base_sub_match = result[0];
			std::string temp = base_sub_match.str();
			std::tr1::regex pattern2("[^:+]\\S+");
			std::smatch result2;
			match = std::regex_search(temp, result2, pattern2);
			if (match){
				std::ssub_match base_sub_match = result2[0];
				out = base_sub_match.str();
			}
		}
		return out;
	}

	static void DoEvents()
	{
		MSG msg;

		// window message         
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DWORD WaitWithMessageLoop(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds)
	{
		DWORD dwRet = WAIT_FAILED;

		DWORD dwMaxTick = (dwMilliseconds == INFINITE) ? 0 : GetTickCount() + dwMilliseconds;

		while (1)
		{
			DWORD dwTimeOut = 0;
			if (dwMilliseconds == INFINITE)
			{
				dwTimeOut = INFINITE;
			}
			else{
				dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
			}
			// wait for event or message, if it's a message, process it and return to waiting state
			//dwRet = MsgWaitForMultipleObjectsEx(nCount, hEvent, dwTimeOut, QS_ALLINPUT, MWMO_ALERTABLE);
			dwRet = MsgWaitForMultipleObjectsEx(nCount, hEvent, dwTimeOut, QS_PAINT | QS_TIMER|QS_SENDMESSAGE , MWMO_ALERTABLE);
			if (dwRet < WAIT_OBJECT_0 + nCount)
			{
				//OutputDebugStringA("WaitWithMessageLoop() event triggered.\n");
				return dwRet;
			}
			else if (dwRet == WAIT_OBJECT_0 + nCount)
			{
				// process window messages
				//OutputDebugStringA("DoEvents.\n");
				DoEvents();
			}
			else if (dwRet == WAIT_TIMEOUT)
			{
				// timed out !
				//OutputDebugStringA("timed out!\n");
				return dwRet;
			}
			else if (dwRet == WAIT_FAILED)
			{
				//OutputDebugStringA("wait failed!\n");
				return dwRet;
			}
			else{
				return dwRet;
			}

		}
	}

	DWORD WaitForMultiEvent(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds)
	{
		DWORD dwRet = WAIT_FAILED;

		//DWORD dwMaxTick = (dwMilliseconds == INFINITE) ? INFINITE : GetTickCount() + dwMilliseconds;

		//while (1)
		{
			//DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
			// wait for event or message, if it's a message, process it and return to waiting state
			dwRet = WaitForMultipleObjects(nCount, hEvent, FALSE, dwMilliseconds);
			if (dwRet < WAIT_OBJECT_0 + nCount )
			{
				//OutputDebugStringA("WaitWithMessageLoop() event triggered.\n");
				return dwRet;
			}
			else if (dwRet == WAIT_TIMEOUT)
			{
				// timed out !
				//OutputDebugStringA("timed out!\n");
				return dwRet;
			}
			else if (dwRet == WAIT_FAILED)
			{
				//OutputDebugStringA("wait failed!\n");
				return dwRet;
			}
			else{
				return dwRet;
			}

		}
	}

}