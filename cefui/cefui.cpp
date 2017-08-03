// cefui.cpp : 定义应用程序的入口点。
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

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

ULONG_PTR gdiplusToken;

// 此代码模块中包含的函数的前向声明:
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

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CEFUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	Gdiplus::GdiplusStartupInput StartupInput;
	GdiplusStartup(&gdiplusToken, &StartupInput, NULL);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFUI));
	//InitCef(hInstance, hAccelTable); //不测试

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

	// 执行应用程序初始化:
	/*(if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFUI));

	// 主消息循环:
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
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
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
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

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
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
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
		// 分析菜单选择:
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
		// TODO: 在此添加任意绘图代码...
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

// “关于”框的消息处理程序。
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
