#include "stdafx.h"
#include "WebViewFactory.h"
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_sandbox_win.h"
#include "include/wrapper/cef_closure_task.h"

#include "client_app.h"
#include "cefclient.h"


WebViewFactory WebViewFactory::s_inst;

WebViewFactory::WebViewFactory()
{
}


WebViewFactory::~WebViewFactory()
{
}

// Set focus to |browser| on the UI thread.
static void SetFocusBrowser(CefRefPtr<CefBrowser> browser) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&SetFocusBrowser, browser));
		return;
	}

	browser->GetHost()->SetFocus(true);
}

HWND WebViewFactory::GetWebView(const HINSTANCE& hInstance, const int& x, const int& y, const int& width,
	const int& height, const CefString& url, const int& alpha, const bool& taskbar)
{
	CefRefPtr<WebItem> item = new  WebItem;
	item->m_handle = new ClientHandler();
	item->m_handle->SetMainWindowHandle(NULL);
	item->m_provider = new BrowserProvider(item->m_handle);
	CefWindowInfo info;
	CefBrowserSettings browser_settings;
	const bool transparent = false;
	//cmd_line->HasSwitch(cefclient::kTransparentPaintingEnabled);
	const bool show_update_rect = false;
	RECT rect;

	item->m_window =
		OSRWindow::Create(item->m_provider, transparent,
		show_update_rect);
	WCHAR szOSRWindowClass[] = L"clientOSR";
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	item->m_window->CreateWidget(NULL, rect, hInstance, szOSRWindowClass);
	info.SetAsWindowless(item->m_window->hwnd(), transparent);
	info.transparent_painting_enabled = true;
	info.windowless_rendering_enabled = true;
	item->m_provider->GetClientHandler()->SetOSRHandler(item->m_window.get());
	if ( !taskbar )
	{
		DWORD exStyle = GetWindowLong(item->m_window->hwnd(), GWL_EXSTYLE);
		exStyle &= ~WS_EX_APPWINDOW;
		exStyle |= WS_EX_TOOLWINDOW;
		SetWindowLong(item->m_window->hwnd(), GWL_EXSTYLE, exStyle);
		SetWindowPos(item->m_window->hwnd(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
	item->m_window->SetAlpha(alpha);
	ShowWindow(item->m_window->hwnd(), SW_SHOW);

	//browser_settings.web_security = STATE_DISABLED;
	//browser_settings.file_access_from_file_urls = STATE_ENABLED;
	browser_settings.universal_access_from_file_urls = STATE_ENABLED; //让xpack访问本地文件
	browser_settings.webgl = STATE_DISABLED;
	browser_settings.plugins = STATE_DISABLED;
	browser_settings.java = STATE_DISABLED;
	// Creat the new child browser window
	CefBrowserHost::CreateBrowser(info, item->m_provider->GetClientHandler().get(),
		url, browser_settings, NULL);
	m_viewMap.insert(std::make_pair(item->m_window->hwnd(), item));
	return item->m_window->hwnd();
}

void WebViewFactory::RemoveWindow(HWND hWnd)
{
	WebViewMap::iterator it = m_viewMap.find(hWnd);
	if ( it != m_viewMap.end() )
	{
		m_viewMap.erase(it);
	}
}

CefRefPtr<WebItem> WebViewFactory::FindItem(const HWND hWnd){
	CefRefPtr<WebItem> ptr;
	WebViewMap::iterator it = m_viewMap.find(hWnd);
	if ( it != m_viewMap.end() )
	{
		ptr = it->second;
	}
	return ptr;
}

CefRefPtr<CefBrowser> WebViewFactory::GetBrowser(int browserID)
{
	CefRefPtr<CefBrowser> ptr;
	WebViewMap::iterator it = m_viewMap.begin();
	for (; it != m_viewMap.end(); ++it)
	{
		ptr = it->second->m_provider->GetBrowser();
		if (ptr.get() && ptr.get()->GetIdentifier() == browserID)
		{
			break;
		}
	}
	return ptr;
}

CefRefPtr<WebItem> WebViewFactory::GetBrowserItem(int browserID)
{
	CefRefPtr<WebItem> ptr;
	WebViewMap::iterator it = m_viewMap.begin();
	for (; it != m_viewMap.end(); ++it)
	{
		CefRefPtr<CefBrowser> browser = it->second->m_provider->GetBrowser();
		if (browser.get() && browser.get()->GetIdentifier() == browserID)
		{
			ptr = it->second;
			break;
		}
	}
	return ptr;
}

HWND WebViewFactory::GetBrowserHwnd(int browserID)
{
	HWND hWnd = NULL;
	CefRefPtr<CefBrowser> ptr;
	WebViewMap::iterator it = m_viewMap.begin();
	for (; it != m_viewMap.end(); ++it)
	{
		if (ptr.get() && ptr.get()->GetIdentifier() == browserID)
		{
			hWnd = it->first;
		}
	}

	return hWnd;
}