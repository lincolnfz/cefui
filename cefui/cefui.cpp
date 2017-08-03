// cefui.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "cefui.h"
#include "WrapCef.h"
#include <comdef.h>
#include <gdiplus.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include "ClientLogic.h"
#include "./pipe/sockCli.h"
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

/*
#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2013. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

#pragma comment(lib, "sandbox.lib")
#pragma comment(lib, "base.lib")
#pragma comment(lib, "base_static.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "dynamic_annotations.lib")
#pragma comment(lib, "libcef.dll.lib")
#pragma comment(lib, "libcef_dll_wrapper.lib")
#pragma comment(lib, "opengl32.lib")*/
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib , "Shlwapi.lib")

#ifdef _DEBUG
#pragma comment(lib, "WrapCef_d.lib")
#else
#pragma comment(lib, "WrapCef.lib")
#endif


#define PORT 22563
#define MAX_LOADSTRING 100

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

ULONG_PTR gdiplusToken;

// �˴���ģ���а����ĺ�����ǰ������:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


ClientLogic* logic = NULL;
cyjh::SockCli* cli = NULL;
unsigned int __stdcall ConectSrvThread(void *){
	HANDLE hMutex = NULL;

	std::wstring directory;
	wchar_t appDataDirectory[MAX_PATH];
	if (FAILED(SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, 0, 0, appDataDirectory)))
		return false;

	directory = std::wstring(appDataDirectory) + L"\\webgameAgent";
	directory.append(L"\\info.ini");

	if (_waccess(directory.c_str(), 0) < 0)
	{
		return 0;
	}
	int port = GetPrivateProfileIntW(L"general", L"port", PORT, directory.c_str());
	logic->setSockCli(cli);
	cli->RegisterConnectFunction(&ClientLogic::Connect, logic);
	cli->RegisterErrorFunction(&ClientLogic::Error, logic);
	cli->RegisterRecvDataFunction(&ClientLogic::RecvSockData, logic);
	cli->Connect("127.0.0.1", port);

	return 0;
}

long __stdcall callback(_EXCEPTION_POINTERS* pExInfo)
{
	//MessageBox(0,"Error","error",MB_OK);
	TCHAR   exeFullPath[MAX_PATH];
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	TCHAR lpszDrive[255], lpszPath[1024], lpszName[255], lpszTemp[255];
	_tsplitpath_s(exeFullPath, lpszDrive, lpszPath, lpszName, lpszTemp);

	TCHAR dumpPath[MAX_PATH];
	_stprintf_s(dumpPath, _T("%s%s%s"), lpszDrive, lpszPath, _T("render_cash.dmp"));

	HANDLE hFile = ::CreateFile(dumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION einfo;
		einfo.ThreadId = ::GetCurrentThreadId();
		einfo.ExceptionPointers = pExInfo;
		einfo.ClientPointers = FALSE;

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, &einfo, NULL, NULL);
		::CloseHandle(hFile);
	}
	TCHAR msg[1024];
	_stprintf_s(msg, _T("carsh, %s"), dumpPath);
	//MessageBox(0, msg, _T("crash"), MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

HHOOK hGetMsg = NULL;
HHOOK hCallMsg = NULL;

LRESULT CALLBACK GetMsgProc(
	_In_ int    code,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	)
{
	if (code == HC_ACTION)
	{
		CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;
		if (cwp->message == WM_SETFOCUS){
			OutputDebugStringW(L"-----[ GetMsgProc WM_SETFOCUS");
			//return 0;
			//cwp->message = WM_KILLFOCUS;
		}
	}
	return CallNextHookEx(hGetMsg, code, wParam, lParam);
}

LRESULT CALLBACK CallWndProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	)
{
	if (nCode == HC_ACTION)
	{
		CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;
		if (cwp->message == WM_SETFOCUS){
			OutputDebugStringW(L"-----[ CallWndProc WM_SETFOCUS");
			//return 0;
		}
	}
	return CallNextHookEx(hCallMsg, nCode, wParam, lParam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
#ifdef _DEBUG1
	OutputDebugString(_T("################################_tWinMain----------------------------"));
#endif
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CEFUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	Gdiplus::GdiplusStartupInput StartupInput;
	GdiplusStartup(&gdiplusToken, &StartupInput, NULL);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFUI));
	//InitCef(hInstance, hAccelTable); //������

	SetUnhandledExceptionFilter(callback);

	WCHAR   exeFullPath[MAX_PATH + 32], PluginDirPath[MAX_PATH], szRenderPath[MAX_PATH];
	GetModuleFileNameW(NULL, exeFullPath, MAX_PATH);
	PathRemoveFileSpec(exeFullPath);

	ClientLogic logic_inst;
	cyjh::SockCli cli_inst;

	logic = &logic_inst;
	cli = &cli_inst;
	bool bconnect = false;
	if (wcsstr(lpCmdLine, L"--type=plugin")/* && wcsstr(lpCmdLine, L"NPSWF32")*/ )
	{
		unsigned int id;
		HANDLE ht = (HANDLE)_beginthreadex(NULL, 0, ConectSrvThread, NULL, 0, &id);
		CloseHandle(ht);
		bconnect = true;
		//hGetMsg = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, 0, GetCurrentThreadId());
		//hCallMsg = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, 0, GetCurrentThreadId());
	}
	
	//PathCombine(szRenderPath, exeFullPath, L"renderx.exe");
	if (wrapQweb::InitLibrary(hInstance, L"render.exe") < 0){
		//wrapQweb::CreateWebView(0, 0, 10, 10, L"about:blank", 0, true, true, WIDGET_NORMAL_SIZE);
		//wrapQweb::CreateWebView(0, 0, 1024, 768, L"http://www.baidu.com", 255, true, true, WIDGET_NORMAL_SIZE);
		//wrapQweb::CreateWebView(0, 0, 1024, 768, L"xpack:///d:/ui.pack/index.html", 255, true, true, WIDGET_NORMAL_SIZE);
		wrapQweb::RunLoop();
		wrapQweb::FreeLibrary();
	}

	// ִ��Ӧ�ó����ʼ��:
	/*(if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFUI));

	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam; */
	Gdiplus::GdiplusShutdown(gdiplusToken);

	if (hGetMsg)
	{
		UnhookWindowsHookEx(hGetMsg);
		hGetMsg = NULL;
	}
	if (hCallMsg)
	{
		UnhookWindowsHookEx(hCallMsg);
		hCallMsg = NULL;
	}
	return 0;
}



//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
//
//    ����ϣ��
//    �˴�������ӵ� Windows 95 �еġ�RegisterClassEx��
//    ����֮ǰ�� Win32 ϵͳ����ʱ������Ҫ�˺��������÷������ô˺���ʮ����Ҫ��
//    ����Ӧ�ó���Ϳ��Ի�ù�����
//    ����ʽ��ȷ�ġ�Сͼ�ꡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFUI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CEFUI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: �ڴ���������ͼ����...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
