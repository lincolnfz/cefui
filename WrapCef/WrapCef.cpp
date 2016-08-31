// WrapCef.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "WrapCef.h"
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <direct.h>
#include <sstream>
#include <string>
#include <Shlwapi.h>
#include <stdio.h>
#include <Shlobj.h>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_sandbox_win.h"
#include "include/wrapper/cef_closure_task.h"

#include "client_app.h"
#include "cefclient.h"
#include "scheme_test.h"
#include "cefclient_osr_widget_win.h"
#include "WebViewFactory.h"
#include "ResponseUI.h"
#include "NormalWebFactory.h"
#include "WebkitEcho.h"

#pragma comment(lib , "Shlwapi.lib")

#define  CEF_USE_SANDBOX 1

#ifdef _WINDLL
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
//#pragma comment(lib, "libcef.dll.lib")
#pragma comment(lib, "libWBX.dll.lib")
#pragma comment(lib, "libcef_dll_wrapper.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "packlib.lib")
//#pragma comment(lib, "gdiplus.lib")
#endif



void* g_sandbox_info = NULL;
#define ID_QUIT WM_USER+1

#if defined(CEF_USE_SANDBOX)
CefScopedSandboxInfo scoped_sandbox;
#endif

char szWorkingDir[MAX_PATH];  // The current working directory

// The global ClientHandler reference.
//CefRefPtr<ClientHandler> g_handler;

WCHAR szCefWindowClass[] = {L"Direct_UI"};

bool bMultiThreadedMessageLoop = false;

static void SetFocusToBrowser(CefRefPtr<CefBrowser> browser);

HWND hMessageWnd;

LRESULT CALLBACK MessageWndProc(HWND hWnd, UINT message, WPARAM wParam,
	LPARAM lParam) {
	switch (message) {
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		switch (wmId) {
		case ID_QUIT:
			PostQuitMessage(0);
			return 0;
		}
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateMessageWindow(HINSTANCE hInstance) {
	static const wchar_t kWndClass[] = L"ClientMessageWindow";

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = MessageWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = kWndClass;
	RegisterClassEx(&wc);

	return CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0,
		hInstance, 0);
}


class MainBrowserProvider : public OSRBrowserProvider {
public:
	virtual CefRefPtr<CefBrowser> GetBrowser() OVERRIDE {
		if (handle_.get())
			return handle_->GetBrowser();

		return NULL;
	}
	MainBrowserProvider(CefRefPtr<ClientHandler> handle){
		handle_ = handle;
	}

	virtual CefRefPtr<ClientHandler> GetClientHandler() OVERRIDE {
		return handle_;
	}

	virtual CefRefPtr<CefBrowser> GetBrowserByWnd(HWND hWnd) OVERRIDE{
		return NULL;
	}

private:
	CefRefPtr<ClientHandler> handle_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(MainBrowserProvider);
}; //g_main_browser_provider;




int InitCef(HINSTANCE hInstance, HACCEL hAccelTable){
#if defined(CEF_USE_SANDBOX)
	g_sandbox_info = scoped_sandbox.sandbox_info();
#endif
	hInstance = GetModuleHandle(NULL);
	CefMainArgs main_args(hInstance);
	CefRefPtr<ClientApp> app(new ClientApp);
	int exit_code = CefExecuteProcess(main_args, app.get(), g_sandbox_info);
	int result = 0;
	if (exit_code >= 0)
		return exit_code;

	// Retrieve the current working directory.
	if (_getcwd(szWorkingDir, MAX_PATH) == NULL)
		szWorkingDir[0] = 0;

	// Parse command line arguments. The passed in values are ignored on Windows.
	AppInitCommandLine(0, NULL);

	CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif

	// Populate the settings based on command line arguments.
	AppGetSettings(settings);
	//bMultiThreadedMessageLoop = true;
	settings.multi_threaded_message_loop = bMultiThreadedMessageLoop;
	//settings.single_process = true; // --single-process

	
	//实现渲染进程分离
	WCHAR szFile[MAX_PATH] = { 0 };
	WCHAR szRender[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szFile, MAX_PATH);
	PathRemoveFileSpec(szFile);
	PathCombine(szRender, szFile, L"render.exe");
	cef_string_set(szRender, wcslen(szRender), &settings.browser_subprocess_path, true); //设置渲染进程exe
	WCHAR szCache[MAX_PATH] = { 0 };
	PathCombine(szCache, szFile, L"cache");
	cef_string_set(szCache, wcslen(szCache), &settings.cache_path, true);
	
	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), g_sandbox_info);

	// Register the scheme handler.
	scheme_test::InitTest();
	{
		std::map<int, CefRefPtr<ClientHandler>> amap;
		CefRefPtr<ClientHandler> a = new ClientHandler();
		amap.insert(std::make_pair(0, a));
		amap.clear();
		int tt = 0;
	}

	CefRefPtr<ClientHandler> handler_1 = new ClientHandler();
	handler_1->SetMainWindowHandle(NULL);
	MainBrowserProvider provider_1(handler_1);

	CefWindowInfo info;
	CefBrowserSettings browser_settings;

	// Populate the browser settings based on command line arguments.
	AppGetBrowserSettings(browser_settings);
	CefRefPtr<CefCommandLine> cmd_line = AppGetCommandLine();
	const bool transparent = false;
		//cmd_line->HasSwitch(cefclient::kTransparentPaintingEnabled);
	const bool show_update_rect = false;
	RECT rect;
	//cmd_line->HasSwitch(cefclient::kShowUpdateRect);
	{
		CefRefPtr<OSRWindow> osr_window =
			OSRWindow::Create(&provider_1, transparent,
			show_update_rect);
		WCHAR szOSRWindowClass[] = L"clientOSR";
		int x = 300;
		int y = 80;
		int width = x + 1024;
		int height = y + 768;
		//RECT rect{ x, y, width, height };
		rect.left = x;
		rect.top = y;
		rect.right = width;
		rect.bottom = height;
		osr_window->CreateWidget(NULL, rect, hInstance, szOSRWindowClass, true);
		info.SetAsWindowless(osr_window->hwnd(), transparent);
		info.transparent_painting_enabled = true;
		info.windowless_rendering_enabled = true;
		provider_1.GetClientHandler()->SetOSRHandler(osr_window.get());
		ShowWindow(osr_window->hwnd(), SW_SHOW);
	}

	//browser_settings.web_security = STATE_DISABLED;
	//browser_settings.file_access_from_file_urls = STATE_ENABLED;
	browser_settings.universal_access_from_file_urls = STATE_ENABLED; //让xpack访问本地文件
	browser_settings.webgl = STATE_DISABLED;
	browser_settings.plugins = STATE_DISABLED;
	browser_settings.java = STATE_DISABLED;
	// Creat the new child browser window
	CefBrowserHost::CreateBrowser(info, provider_1.GetClientHandler().get(),
		provider_1.GetClientHandler()->GetStartupURL(), browser_settings, NULL);

	{
		if (handler_1.get())
		{
			CefRefPtr<CefBrowser> browser = handler_1->GetBrowser();
			if (browser)
			{
				SetFocusToBrowser(browser);
			}
		}
	}


	CefRefPtr<ClientHandler> handler_2 = new ClientHandler();
	handler_2->SetMainWindowHandle(NULL);
	MainBrowserProvider provider_2(handler_2);
	//multi window test code
	/*{
		CefRefPtr<OSRWindow> osr_window =
			OSRWindow::Create(&provider_2, transparent,
			show_update_rect);
		WCHAR szOSRWindowClass[] = L"clientOSR";
		int x = 500;
		int y = 400;
		int width = x + 768;
		int height = y + 600;
		RECT rect{ x, y, width, height };
		osr_window->CreateWidget(NULL, rect, hInstance, szOSRWindowClass);
		info.SetAsWindowless(osr_window->hwnd(), transparent);
		info.transparent_painting_enabled = true;
		info.windowless_rendering_enabled = true;
		provider_2.GetClientHandler()->SetOSRHandler(osr_window.get());

		CefBrowserHost::CreateBrowser(info, provider_2.GetClientHandler().get(),
			provider_2.GetClientHandler()->GetStartupURL(), browser_settings, NULL);
	}*/

	/*MyCefCefRegisterClass(hInstance);*/

	if (settings.multi_threaded_message_loop == false)
	{
		CefRunMessageLoop();
	}
	else{
		// Create a hidden window for message processing.
		hMessageWnd = CreateMessageWindow(hInstance);
		DCHECK(hMessageWnd);

		MSG msg;

		// Run the application message loop.
		while (GetMessage(&msg, NULL, 0, 0)) {
			// Allow processing of find dialog messages.

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DestroyWindow(hMessageWnd);
		hMessageWnd = NULL;

		result = static_cast<int>(msg.wParam);
	}

	CefShutdown();

	return result;
}

void AppQuitMessageLoop() {
	if (bMultiThreadedMessageLoop) {
		// Running in multi-threaded message loop mode. Need to execute
		// PostQuitMessage on the main application thread.
		DCHECK(hMessageWnd);
		PostMessage(hMessageWnd, WM_COMMAND, ID_QUIT, 0);
	}
	else {
		CefQuitMessageLoop();
	}
}

int InitBrowser(HINSTANCE hInstance)
{
#if defined(CEF_USE_SANDBOX)
	g_sandbox_info = scoped_sandbox.sandbox_info();
#endif
	CefMainArgs main_args(hInstance);
	CefRefPtr<ClientApp> app(new ClientApp);
	int exit_code = CefExecuteProcess(main_args, app.get(), g_sandbox_info);
	int result = -1;
	if (exit_code >= 0)
		return exit_code;

	// Retrieve the current working directory.
	if (_getcwd(szWorkingDir, MAX_PATH) == NULL)
		szWorkingDir[0] = 0;

	// Parse command line arguments. The passed in values are ignored on Windows.
	AppInitCommandLine(0, NULL);

	CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif

	// Populate the settings based on command line arguments.
	AppGetSettings(settings);
	//bMultiThreadedMessageLoop = true;
	settings.multi_threaded_message_loop = bMultiThreadedMessageLoop;

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), g_sandbox_info);

	// Register the scheme handler.
	scheme_test::InitTest();

	return -1;
}

int UnInitBrowser()
{
	CefShutdown();
	return 0;
}


// Set focus to |browser| on the UI thread.
static void SetFocusToBrowser(CefRefPtr<CefBrowser> browser) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&SetFocusToBrowser, browser));
		return;
	}

	browser->GetHost()->SetFocus(true);
}

namespace cefControl{

}

WCHAR g_szLocalPath[MAX_PATH];
std::wstring g_strAppDataPath;

namespace wrapQweb{

	static bool getAppDataFolder(std::wstring& directory)
	{
		wchar_t appDataDirectory[MAX_PATH];
		if (FAILED(SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, 0, 0, appDataDirectory)))
			return false;

		wchar_t executablePath[MAX_PATH];
		if (!::GetModuleFileNameW(0, executablePath, MAX_PATH))
			return false;

		::PathRemoveExtensionW(executablePath);

		directory = std::wstring(appDataDirectory) + L"\\" + ::PathFindFileNameW(executablePath);
		directory.append(L"\\");
		return true;
	}

	static bool g_init = false;
	static bool g_multi_thread = false;
	CefRefPtr<ClientApp> g_app;
	HINSTANCE g_hInstance = 0;
	int InitLibrary(HINSTANCE hInstance, WCHAR* lpRender)
	{
		g_hInstance = hInstance;
#if defined(CEF_USE_SANDBOX)
		g_sandbox_info = scoped_sandbox.sandbox_info();
#endif
		//HINSTANCE hInstance = GetModuleHandle(NULL);
		CefMainArgs main_args(g_hInstance);
		g_app = new ClientApp;
		int exit_code = CefExecuteProcess(main_args, g_app.get(), g_sandbox_info);
		if (exit_code >= 0)
			return exit_code;

		CefSettings settings;
#if !defined(CEF_USE_SANDBOX)
		settings.no_sandbox = true;
#endif

		settings.multi_threaded_message_loop = g_multi_thread;
		settings.single_process = true;
		std::wstring cachePath;
		if (getAppDataFolder(cachePath)){
			g_strAppDataPath = cachePath;
			cachePath.append(L"cache");
			cef_string_set(cachePath.c_str(), wcslen(cachePath.c_str()), &settings.cache_path, true);
		}
		WCHAR szLocal[] = L"zh-CN";
		cef_string_set(szLocal, wcslen(szLocal), &settings.locale, true);

		WCHAR szLogFile[512] = { 0 };
		wsprintfW(szLogFile, L"%s\\debug.log", cachePath.c_str());
		cef_string_set(szLogFile, wcslen(szLogFile), &settings.log_file, true);

		//实现渲染进程分离
		
		WCHAR szRender[MAX_PATH] = { 0 };
		PathCombine(szRender, g_szLocalPath, lpRender);
		//OutputDebugString(szRender);
		if (szRender && wcslen(szRender) > 0 && _waccess(szRender, 0) == 0)
		{
			cef_string_set(szRender, wcslen(szRender), &settings.browser_subprocess_path, true); //设置渲染进程exe
			settings.single_process = false;
		}
		OSRWindow::s_singleProcess = settings.single_process;
		//WCHAR szFile[MAX_PATH] = { 0 };
		//WCHAR szCache[MAX_PATH] = { 0 };
		//GetModuleFileName(0, szFile, MAX_PATH);
		//PathRemoveFileSpec(szFile);
		//PathCombine(szCache, szFile, L"cache");
		//cef_string_set(szCache, wcslen(szCache), &settings.cache_path, true);

		// Initialize CEF.
		CefInitialize(main_args, settings, g_app.get(), g_sandbox_info);

		// Register the scheme handler.
		scheme_test::InitTest();

		return exit_code;
	}

	HWND CreateWebView(const int& x, const int& y, const int& width, const int& height, const WCHAR* lpResource, const int& alpha, const bool& taskbar, const bool& trans)
	{
		return WebViewFactory::getInstance().GetWebView(g_hInstance, x, y, width, height, CefString(lpResource), alpha, taskbar, trans);
	}

	void InitQWeb(FunMap* map){
		ResponseUI::SetFunMap(map);
	}

	void RunLoop(){
		if ( g_multi_thread )
		{
			MSG msg;

			// Run the application message loop.
			while (GetMessage(&msg, NULL, 0, 0)) {
				// Allow processing of find dialog messages.

				if (!TranslateAccelerator(msg.hwnd, /*hAccelTable*/ NULL, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		else{
			CefRunMessageLoop();
		}
	}

	void FreeLibrary(){
		CefShutdown();
	}

	bool QueryNodeAttrib(const HWND& hWnd, const int& x, const int& y, char* name, WCHAR* outVal, const int& len)
	{
		bool bret = false;
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().FindItem(hWnd);
		if ( item )
		{
			//CefString val(L"data-nc");
			//item->m_provider->GetBrowser()->GetHost()->SendQueryElement(x, y, val);
			POINT pt;
			pt.x = x;
			pt.y = y;
			ClientToScreen(hWnd, &pt);
			int g_x = pt.x;
			int g_y = pt.y;
			std::wstring val;
			item->m_handle->queryElementAttrib(x, y, g_x, g_y, val);
			if ( len > 0 )
			{
				//wcscpy_s(outVal, len, val.ToWString().c_str());
				wcscpy_s(outVal, len, val.c_str());
				bret = true;
			}
		}		

		return bret;
	}

	bool callJSMethod(const HWND& hWnd, const char* fun_name, const char* utf8_parm,
		const char* utf8_frame_name/* = 0*/, CStringW* outstr/* = 0*/)
	{
		bool bret = false;
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().FindItem(hWnd);
		if ( item )
		{
			bret = item->m_handle->callJSMethod(fun_name, utf8_parm, utf8_frame_name, outstr);
		}

		return bret;
	}

	bool invokedJSMethod(const HWND& hWnd, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm, CStringW* outstr,
		const char* utf8_frame_name /*= 0*/, bool bNoticeJSTrans2JSON /*= true*/)
	{
		bool bret = false;
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().FindItem(hWnd);
		if ( item.get() && item->m_handle.get() )
		{
			bret = item->m_handle->invokedJSMethod(utf8_module, utf8_method, utf8_parm, outstr, utf8_frame_name, bNoticeJSTrans2JSON);
		}

		return bret;
	}

	bool asyncInvokedJSMethod(const HWND& hWnd, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm,
		const char* utf8_frame_name /*= 0*/, bool bNoticeJSTrans2JSON /*= true*/)
	{
		bool bret = false;
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().FindItem(hWnd);
		if (item.get() && item->m_handle.get())
		{
			bret = item->m_handle->asyncInvokedJSMethod(utf8_module, utf8_method, utf8_parm, utf8_frame_name, bNoticeJSTrans2JSON);
		}
		else{
			bret = NormalWebFactory::getInstance().asyncInvokedJSMethod(hWnd, utf8_module,
				utf8_method, utf8_parm,
				utf8_frame_name, bNoticeJSTrans2JSON);
		}

		return bret;
	}

	bool freeMem(HGLOBAL hMem)
	{
		return GlobalFree(hMem) == NULL;
	}

	void CloseWebview(const HWND& hWnd)
	{
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowserByHwnd(hWnd);
		if (browser.get())
		{
			browser->GetHost()->CloseBrowser(true);
		}
	}

	/*bool RegPlugin(const HWND& hWnd, const WCHAR* szVal, const bool bPPapi, const bool bSandBox)
	{
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().FindItem(hWnd);
		if (item)
		{
			bool snadbox = false;
			if (bPPapi){
				snadbox = bSandBox;
			}
#ifdef _DEBUG1
			WCHAR szBuf[512] = { L"" };
			swprintf_s(szBuf, L"--------------------val = %s, ppapi = %d", szVal, bPPapi);
			OutputDebugStringW(szBuf);
#endif
			//WCHAR plugin[] = { L"d:\\plugins\\NPSWF32_21_0_0_182.dll" };

			item->m_provider->GetBrowser()->RegPlugin(std::wstring(szVal), bPPapi);
		}
		return true;
	}*/

	void CloseAllWebView()
	{
		/*CefRefPtr<WebItem> item = WebViewFactory::getInstance().FindItem(hWnd);
		if (item)
		{
			item->m_handle->CloseAllBrowsers(true);
		}*/

		WebViewFactory::getInstance().CloseAll();
	}

	void QuitLoop()
	{
		CefQuitMessageLoop();
	}

	void SetFouceWebView(const HWND& hWnd, const bool& fouce)
	{
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowserByHwnd(hWnd);
		if (browser.get())
		{
			browser->GetHost()->SendFocusEvent(fouce);
		}
	}


	void InitEchoFn(EchoMap* map)
	{
		WebkitEcho::SetFunMap(map);
	}

	void CreateWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie/* = NULL*/)
	{
		NormalWebFactory::getInstance().CreateNewWebControl(hwnd, url, cookie);
	}

	bool CloseWebControl(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().CloseWebControl(hwnd);
	}

	bool LoadUrl(const HWND& hwnd, const WCHAR* url)
	{
		return NormalWebFactory::getInstance().Loadurl(hwnd, url);
	}

	bool GoBack(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().GoBack(hwnd);
	}

	bool GoForward(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().GoForward(hwnd);
	}

	bool Reload(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().Reload(hwnd);
	}

	bool ReloadIgnoreCache(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().ReloadIgnoreCache(hwnd);
	}

	bool IsAudioMuted(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().IsAudioMuted(hwnd);
	}

	void SetAudioMuted(const HWND& hwnd, const bool& bEnable)
	{
		NormalWebFactory::getInstance().SetAudioMuted(hwnd, bEnable);
	}

	bool Stop(const HWND& hwnd)
	{
		return NormalWebFactory::getInstance().Stop(hwnd);
	}

// 	bool asyncInvokedJSMethod(const HWND& hWnd, const char* utf8_module, const char* utf8_method,
// 		const char* utf8_parm,
// 		const char* utf8_frame_name, bool bNoticeJSTrans2JSON /*= true*/)
// 	{
// 		return NormalWebFactory::getInstance().asyncInvokedJSMethod(hWnd, utf8_module,
// 			utf8_method, utf8_parm,
// 			utf8_frame_name, bNoticeJSTrans2JSON);
// 	}

	void AdjustRenderSpeed(const HWND& hWnd, const double& dbSpeed)
	{
		NormalWebFactory::getInstance().AdjustRenderSpeed(hWnd, dbSpeed);
	}

	void ClearBrowserData(int combType)
	{
		WebViewFactory::getInstance().ClearData(combType);
	}

	void SendMouseClickEvent(const HWND& hWnd, const unsigned int& msg, const long& wp, const long& lp)
	{
		NormalWebFactory::getInstance().SendMouseClickEvent(hWnd, msg, wp, lp);
	}

	////
	class CChromeiumBrowserControl
	{
	public:
		CChromeiumBrowserControl(){}
		virtual ~CChromeiumBrowserControl(){}
		HWND AttachHwnd(HWND, const WCHAR*);
		void handle_size(HWND);
		void handle_SetForce();

	private:
		CefRefPtr<ClientHandler> m_handler;

	};

	HWND CChromeiumBrowserControl::AttachHwnd(HWND hParent, const WCHAR* url)
	{
		if (!IsWindow(hParent))
		{
			return NULL;
		}
		m_handler = new ClientHandler();
		CefWindowInfo info;
		CefBrowserSettings browser_settings;

		// Populate the browser settings based on command line arguments.
		//AppGetBrowserSettings(browser_settings);
		browser_settings.universal_access_from_file_urls = STATE_ENABLED; //让xpack访问本地文件

		RECT rect;

		GetClientRect(hParent, &rect);
		info.SetAsChild(hParent, rect);

		CefBrowserHost::CreateBrowser(info, m_handler.get(),
			url, browser_settings, NULL);
		
		HWND hWnd = NULL;
		if (m_handler->GetBrowser() && m_handler->GetBrowser()->GetHost()){
			hWnd = m_handler->GetBrowser()->GetHost()->GetWindowHandle();
		}
		return hWnd;
	}

	void CChromeiumBrowserControl::handle_size(HWND hWnd)
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		if (m_handler.get())
		{
			CefWindowHandle hBrowser = m_handler->GetBrowser()->GetHost()->GetWindowHandle();
			HDWP hdwp = BeginDeferWindowPos(1);
			hdwp = DeferWindowPos(hdwp, hBrowser, NULL,
				rect.left, rect.top, rect.right - rect.left,
				rect.bottom - rect.top, SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
		}
	}

	void CChromeiumBrowserControl::handle_SetForce()
	{
		if (m_handler.get())
		{
			CefRefPtr<CefBrowser> browser = m_handler->GetBrowser();
			if (browser)
			{
				SetFocusToBrowser(browser);
			}
		}

	}

	CWebkitControl::CWebkitControl()
	{
		m_browser = new CChromeiumBrowserControl;
	}

	CWebkitControl::~CWebkitControl()
	{
		delete m_browser;
	}

	HWND CWebkitControl::AttachHwnd(HWND hParentWnd, const WCHAR* url)
	{
		return m_browser->AttachHwnd(hParentWnd, url);
	}

	void CWebkitControl::handle_size(HWND hWnd)
	{
		m_browser->handle_size(hWnd);
	}

	void CWebkitControl::handle_SetForce()
	{
		m_browser->handle_SetForce();
	}
}