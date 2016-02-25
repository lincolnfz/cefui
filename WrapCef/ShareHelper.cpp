#include "stdafx.h"
#include "ShareHelper.h"

namespace cyjh{


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
				dwTimeOut == INFINITE;
			}
			else{
				dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
			}
			// wait for event or message, if it's a message, process it and return to waiting state
			dwRet = MsgWaitForMultipleObjectsEx(nCount, hEvent, dwTimeOut/*dwMaxTick - GetTickCount()*/, QS_ALLINPUT, MWMO_ALERTABLE);
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