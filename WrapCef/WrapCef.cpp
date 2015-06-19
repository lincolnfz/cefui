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

#if defined(CEF_USE_SANDBOX)
CefScopedSandboxInfo scoped_sandbox;
#endif

char szWorkingDir[MAX_PATH];  // The current working directory

// The global ClientHandler reference.
extern CefRefPtr<ClientHandler> g_handler;

WCHAR szCefWindowClass[] = {L"Direct_UI"};

class MainBrowserProvider : public OSRBrowserProvider {
	virtual CefRefPtr<CefBrowser> GetBrowser() {
		if (g_handler.get())
			return g_handler->GetBrowser();

		return NULL;
	}
} g_main_browser_provider;

// Set focus to |browser| on the UI thread.
static void SetFocusToBrowser(CefRefPtr<CefBrowser> browser) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&SetFocusToBrowser, browser));
		return;
	}

	if (!g_handler)
		return;

	if (AppIsOffScreenRenderingEnabled()) {
		// Give focus to the OSR window.
		CefRefPtr<OSRWindow> osr_window =
			static_cast<OSRWindow*>(g_handler->GetOSRHandler().get());
		if (osr_window)
			::SetFocus(osr_window->hwnd());
	}
	else {
		// Give focus to the browser.
		browser->GetHost()->SetFocus(true);
	}
}

LRESULT CALLBACK cefWndProc(HWND hWnd, UINT message, WPARAM wParam,
	LPARAM lParam) {
	static HWND backWnd = NULL, forwardWnd = NULL, reloadWnd = NULL,
		stopWnd = NULL, editWnd = NULL;
	static WNDPROC editWndOldProc = NULL;

	// Static members used for the find dialog.
	static FINDREPLACE fr;
	static WCHAR szFindWhat[80] = { 0 };
	static WCHAR szLastFindWhat[80] = { 0 };
	static bool findNext = false;
	static bool lastMatchCase = false;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

		// Callback for the main window
		switch (message) {
		case WM_CREATE: {
			// Create the single static handler class instance
			g_handler = new ClientHandler();
			g_handler->SetMainWindowHandle(hWnd);

			// Create the child windows used for navigation
			RECT rect;
			int x = 0;

			GetClientRect(hWnd, &rect);

			CefWindowInfo info;
			CefBrowserSettings settings;

			// Populate the browser settings based on command line arguments.
			AppGetBrowserSettings(settings);

			{
				// Initialize window info to the defaults for a child window.
				info.SetAsChild(hWnd, rect);
			}

			// Creat the new child browser window
			CefBrowserHost::CreateBrowser(info, g_handler.get(),
				g_handler->GetStartupURL(), settings, NULL);

			return 0;
		}

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_SETFOCUS:
			if (g_handler.get()) {
				CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
				if (browser)
					SetFocusToBrowser(browser);
			}
			return 0;

		case WM_SIZE: {
			if (!g_handler.get())
				break;

			if (g_handler->GetBrowser()) {
				// Retrieve the window handle (parent window with off-screen rendering).
				CefWindowHandle hwnd =
					g_handler->GetBrowser()->GetHost()->GetWindowHandle();
				if (hwnd) {
					if (wParam == SIZE_MINIMIZED) {
						// For windowed browsers when the frame window is minimized set the
						// browser window size to 0x0 to reduce resource usage.
						if ( true /*!offscreen*/) {
							SetWindowPos(hwnd, NULL,
								0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
						}
					}
					else {
						// Resize the window and address bar to match the new frame size.
						RECT rect;
						GetClientRect(hWnd, &rect);
						HDWP hdwp = BeginDeferWindowPos(1);
						hdwp = DeferWindowPos(hdwp, hwnd, NULL,
							rect.left, rect.top, rect.right - rect.left,
							rect.bottom - rect.top, SWP_NOZORDER);
						EndDeferWindowPos(hdwp);
					}
				}
			}
		} break;

		case WM_MOVING:
		case WM_MOVE:
			// Notify the browser of move events so that popup windows are displayed
			// in the correct location and dismissed when the window moves.
			if (g_handler.get() && g_handler->GetBrowser())
				g_handler->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
			return 0;

		case WM_ERASEBKGND:
			if (g_handler.get() && g_handler->GetBrowser()) {
				CefWindowHandle hwnd =
					g_handler->GetBrowser()->GetHost()->GetWindowHandle();
				if (hwnd) {
					// Dont erase the background if the browser window has been loaded
					// (this avoids flashing)
					return 0;
				}
			}
			break;

		case WM_ENTERMENULOOP:
			if (!wParam) {
				// Entering the menu loop for the application menu.
				CefSetOSModalLoop(true);
			}
			break;

		case WM_EXITMENULOOP:
			if (!wParam) {
				// Exiting the menu loop for the application menu.
				CefSetOSModalLoop(false);
			}
			break;

		case WM_CLOSE:
			if (g_handler.get() && !g_handler->IsClosing()) {
				CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
				if (browser.get()) {
					// Notify the browser window that we would like to close it. This
					// will result in a call to ClientHandler::DoClose() if the
					// JavaScript 'onbeforeunload' event handler allows it.
					browser->GetHost()->CloseBrowser(false);

					// Cancel the close.
					return 0;
				}
			}

			// Allow the close.
			break;

		case WM_DESTROY:
			// Quitting CEF is handled in ClientHandler::OnBeforeClose().
			return 0;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	
}


ATOM MyCefCefRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = cefWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szCefWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	return RegisterClassEx(&wcex);
}

int InitCef(HINSTANCE hInstance){
#if defined(CEF_USE_SANDBOX)
	g_sandbox_info = scoped_sandbox.sandbox_info();
#endif
	CefMainArgs main_args(hInstance);
	CefRefPtr<ClientApp> app(new ClientApp);
	int exit_code = CefExecuteProcess(main_args, app.get(), g_sandbox_info);

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

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), g_sandbox_info);

	// Register the scheme handler.
	scheme_test::InitTest();

	g_handler = new ClientHandler();
	g_handler->SetMainWindowHandle(NULL);

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
			OSRWindow::Create(&g_main_browser_provider, transparent,
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
		g_handler->SetOSRHandler(osr_window.get());
	}

	// Creat the new child browser window
	CefBrowserHost::CreateBrowser(info, g_handler.get(),
		g_handler->GetStartupURL(), browser_settings, NULL);

	/*MyCefCefRegisterClass(hInstance);*/

	CefRunMessageLoop();

	CefShutdown();

	return 0;
}