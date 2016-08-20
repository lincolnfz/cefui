#include "stdafx.h"
#include "SubClassWindow.h"
#include "detours.h"

extern "C" {
	static BOOL (WINAPI *Real_PostMessageW)(HWND , UINT , WPARAM , LPARAM ) = PostMessageW;
	static BOOL(WINAPI *Real_PostMessageA)(HWND, UINT, WPARAM, LPARAM) = PostMessageA;
	static LRESULT(WINAPI *Real_SendMessageW)(HWND, UINT, WPARAM, LPARAM) = SendMessageW;
	static LRESULT(WINAPI *Real_SendMessageA)(HWND, UINT, WPARAM, LPARAM) = SendMessageA;
	static HWND(WINAPI *Real_SetCapture)(HWND hWnd) = SetCapture;
}

BOOL WINAPI _PostMessageW(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PARENTNOTIFY)
	{
		OutputDebugStringW(L"-----[! Real_PostMessageW WM_PARENTNOTIFY");
	}
	return Real_PostMessageW(hwnd, msg, wp, lp);
}

BOOL WINAPI _PostMessageA(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PARENTNOTIFY)
	{
		OutputDebugStringW(L"-----[! Real_PostMessageA WM_PARENTNOTIFY");
	}
	return Real_PostMessageA(hwnd, msg, wp, lp);
}

LRESULT WINAPI _SendMessageW(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PARENTNOTIFY /*&& SubClassWindow::GetInst().IsInFitler(hwnd)*/)
	{
		OutputDebugStringW(L"-----[! Disable Real_SendMessageW WM_PARENTNOTIFY");
		return FALSE;
	}
	return Real_SendMessageW(hwnd, msg, wp, lp);
}

LRESULT WINAPI _SendMessageA(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PARENTNOTIFY)
	{
		OutputDebugStringW(L"-----[! Real_SendMessageA WM_PARENTNOTIFY");
	}
	return Real_SendMessageA(hwnd, msg, wp, lp);
}

HWND WINAPI _SetCapture(HWND hWnd)
{
	OutputDebugStringW(L"-----[! _SetCapture");
	//return hWnd;
	return Real_SetCapture(hWnd);
}

SubClassWindow SubClassWindow::s_inst;
SubClassWindow::SubClassWindow()
{
	/*DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)Real_PostMessageW, _PostMessageW);
	DetourAttach(&(PVOID&)Real_PostMessageA, _PostMessageA);
	DetourAttach(&(PVOID&)Real_SendMessageW, _SendMessageW);
	DetourAttach(&(PVOID&)Real_SendMessageA, _SendMessageA);
	DetourAttach(&(PVOID&)Real_SetCapture, _SetCapture);
	DetourTransactionCommit();*/
}


SubClassWindow::~SubClassWindow()
{
}

SubClassWindow& SubClassWindow::GetInst()
{
	return s_inst;
}

BOOL SubClassWindow::GetProcInfo(HWND hWnd, WNDPROC& proc, bool& val, bool& send)
{
	BOOL ret = FALSE;
	std::unique_lock<std::mutex> lock(m_MapMutex_);
	WNCPROC_MAP::iterator it = m_defProcs.find(hWnd);
	if (it != m_defProcs.end())
	{
		proc = it->second->m_proc;
		val = it->second->m_bClick;
		send = it->second->m_bSendDown;
		ret = TRUE;
	}
	return ret;
}

void SubClassWindow::SetClickVal(HWND hWnd, bool val, bool send)
{
	std::unique_lock<std::mutex> lock(m_MapMutex_);
	WNCPROC_MAP::iterator it = m_defProcs.find(hWnd);
	if (it != m_defProcs.end())
	{
		it->second->m_bClick = val;
		it->second->m_bSendDown = send;
	}
}

BOOL SubClassWindow::IsInFitler(HWND hWnd)
{
	BOOL bFind = FALSE;
	std::unique_lock<std::mutex> lock(m_MapMutex_);
	WNCPROC_MAP::iterator it = m_defProcs.find(hWnd);
	if (it != m_defProcs.end())
	{
		bFind = TRUE;
	}
	return bFind;
}

void SubClassWindow::SubWindow(HWND hWnd, int)
{
	std::unique_lock<std::mutex> lock(m_MapMutex_);
	OutputDebugStringW(L"-----[! begin SubClassWindow::SubWindow");
	WNCPROC_MAP::iterator it = m_defProcs.find(hWnd);
	if ( it != m_defProcs.end() )
	{
		OutputDebugStringW(L"-----[! begin SubClassWindow::SubWindow return");
		return;
	}
	WNDPROC proc = reinterpret_cast<WNDPROC>(::GetWindowLong(hWnd, GWL_WNDPROC));
	if (::SetWindowLong(hWnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(myWndProc))){
		boost::shared_ptr<InWndProcContext> context(new InWndProcContext);
		context->m_proc = proc;
		context->m_bClick = false;
		m_defProcs.insert(std::make_pair(hWnd, context));
		PostMessageW(hWnd, WM_USER + 828, 0, 0);
	}
	else{
		WCHAR szBuf[512];
		wsprintf(szBuf, L"-----[! SubClassWindow::SubWindow SetWindowLong fail: %d", GetLastError());
		OutputDebugStringW(szBuf);
	}
}

void SubClassWindow::UnSubWIndow(HWND hWnd)
{
	std::unique_lock<std::mutex> lock(m_MapMutex_);
	OutputDebugStringW(L"-----[! begin UnSubWIndow:: UnSubWIndow");
	WNCPROC_MAP::iterator it = m_defProcs.find(hWnd);
	if (it == m_defProcs.end())
	{
		OutputDebugStringW(L"-----[! begin UnSubWIndow::UnSubWIndow return");
		return;
	}
	WNDPROC defProc = it->second->m_proc;
	if (::SetWindowLong(hWnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(defProc))){

	}
	else{
		OutputDebugStringW(L"-----[! begin UnSubWIndow::SetWindowLong fail");
	}
	m_defProcs.erase(hWnd);	
}

bool filterMsg(UINT msg){
	static UINT msg_array[] = {
		WM_MOUSEMOVE,
		WM_MOUSEHOVER,
		WM_SETCURSOR,
		WM_MOUSELEAVE,
	};
	static int size = sizeof(msg_array) / sizeof(msg_array[0]);
	bool hit = false;
	if ( msg > WM_USER )
	{
		return true;
	}
	for (int i = 0; i < size; ++i)
	{
		if ( msg == msg_array[i] )
		{
			hit = true;
			break;
		}
	}
	return hit;
}

LRESULT __stdcall SubClassWindow::myWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = NULL;
	bool bClick = false;
	bool bSend = false;
	if (!SubClassWindow::GetInst().GetProcInfo(hWnd, proc, bClick, bSend) )
	{
		OutputDebugStringW(L"-----[ SubClassWindow::WndProc no frond");
		return 0L;
	}
	/*if ( !filterMsg(message) )
	{
		WCHAR szBuf[512];
		wsprintf(szBuf, L"-----[! SubClassWindow::WndProc msg: 0x%04x", message);
		OutputDebugStringW(szBuf);
	}*/
	if ( message ==WM_LBUTTONDOWN )
	{
		return CallWindowProc(proc, hWnd, message, wParam, lParam);
		//return DefWindowProc(hWnd, message, wParam, lParam);
		/*WCHAR szBuf[128];
		wsprintf(szBuf, L"-----[! DISABLE WM_LBUTTONDOWN hwnd : %08x", hWnd);
		OutputDebugStringW(szBuf);
		if ( bClick == false )
		{
			SubClassWindow::GetInst().SetClickVal(hWnd, true, false);
			PostMessage(hWnd, message, wParam, lParam);
			return 0;
		}
		else{
			
			if ( bSend == false )
			{
				SubClassWindow::GetInst().SetClickVal(hWnd, true, true);
				SendMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
				//Sleep(0);
				SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam);
				SubClassWindow::GetInst().SetClickVal(hWnd, false, false);
			}
			else{
				return CallWindowProc(proc, hWnd, message, wParam, lParam);
				//return DefWindowProc(hWnd, message, wParam, lParam);
			}
			return 0;
		}
		return 0;*/
	}
	return CallWindowProc(proc, hWnd, message, wParam, lParam);
}