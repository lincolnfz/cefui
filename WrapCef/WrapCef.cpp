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

private:
	CefRefPtr<ClientHandler> handle_;
}; //g_main_browser_provider;




int InitCef(HINSTANCE hInstance, HACCEL hAccelTable){
#if defined(CEF_USE_SANDBOX)
	g_sandbox_info = scoped_sandbox.sandbox_info();
#endif
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
	settings.multi_threaded_message_loop = bMultiThreadedMessageLoop;

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), g_sandbox_info);

	// Register the scheme handler.
	scheme_test::InitTest();

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
		RECT rect{ x, y, width, height };
		osr_window->CreateWidget(NULL, rect, hInstance, szOSRWindowClass);
		info.SetAsWindowless(osr_window->hwnd(), transparent);
		info.transparent_painting_enabled = true;
		info.windowless_rendering_enabled = true;
		provider_1.GetClientHandler()->SetOSRHandler(osr_window.get());
	}

	// Creat the new child browser window
	CefBrowserHost::CreateBrowser(info, provider_1.GetClientHandler().get(),
		provider_1.GetClientHandler()->GetStartupURL(), browser_settings, NULL);

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