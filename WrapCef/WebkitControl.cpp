#include "stdafx.h"
#include "WebkitControl.h"
#include "client_app.h"
#include "cefclient.h"
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_sandbox_win.h"
#include "include/wrapper/cef_closure_task.h"
#include "NormalWebFactory.h"
#include <boost/functional/hash.hpp>

// Set focus to |browser| on the UI thread.
static void SetFocusToBrowserControl(CefRefPtr<CefBrowser> browser) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&SetFocusToBrowserControl, browser));
		return;
	}

	browser->GetHost()->SetFocus(true);
}

typedef std::map<size_t, CefRefPtr<CefCookieManager>> COOKIES_MAN_MAP;

class CookiesManage
{
public:
	static CookiesManage& getInst(){
		return s_inst;
	}
	virtual ~CookiesManage(){}
	CefRefPtr<CefCookieManager> GetCookieManager(const WCHAR* cookie_ctx){
		std::wstring path(cookie_ctx);
		boost::hash<std::wstring> string_hash;
		size_t hash = string_hash(path);
		COOKIES_MAN_MAP::iterator it = map_.find(hash);
		CefRefPtr<CefCookieManager> item;
		if ( it != map_.end() )
		{
			item = it->second;
		}
		else{
			item = CefCookieManager::CreateManager(CefString(path), false);
			map_.insert(std::make_pair(hash, item));
		}

		COOKIES_MAN_MAP::iterator it2 = map_.find(hash);
		CefRefPtr<CefCookieManager> val;
		if (it2 != map_.end())
		{
			val = it2->second;
		}
		return val;
	}
protected:
	CookiesManage(){}
private:
	static CookiesManage s_inst;
	COOKIES_MAN_MAP map_;
};
CookiesManage CookiesManage::s_inst;


CefRefPtr<CefCookieManager> RequestContextHandler::GetCookieManager()
{
	CefRefPtr<CefCookieManager> manager = CookiesManage::getInst().GetCookieManager(cookie_ctx_.c_str());
	if ( manager.get() )
	{
		return manager;
	}
	return NULL;
}

////


HWND ChromeiumBrowserControl::AttachHwnd(HWND hParent, const WCHAR* url, const WCHAR* cookie_context)
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
	//browser_settings.Set()

	RECT rect;

	GetClientRect(hParent, &rect);
	info.SetAsChild(hParent, rect);

	CefRefPtr<CefRequestContext> request_context;
	if (cookie_context && wcslen(cookie_context) > 0){
		m_requestContextHandler = new RequestContextHandler(cookie_context);
		request_context = CefRequestContext::CreateContext(m_requestContextHandler);
	}
	CefBrowserHost::CreateBrowser(info, m_handler.get(),
		url, browser_settings, request_context.get() ? request_context : NULL );

	HWND hWnd = NULL;
	if (m_handler->GetBrowser() && m_handler->GetBrowser()->GetHost()){
		hWnd = m_handler->GetBrowser()->GetHost()->GetWindowHandle();
	}
	return hWnd;
}

void ChromeiumBrowserControl::handle_size(HWND hWnd)
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

void ChromeiumBrowserControl::handle_SetForce()
{
	if (m_handler.get())
	{
		CefRefPtr<CefBrowser> browser = m_handler->GetBrowser();
		if (browser)
		{
			SetFocusToBrowserControl(browser);
		}
	}

}

bool ChromeiumBrowserControl::loadUrl(const WCHAR* url)
{
	bool ret = false;
	if (m_handler->GetBrowser().get() && m_handler->GetBrowser()->GetMainFrame().get() )
	{
		//m_handler->GetBrowser()->GetMainFrame()->LoadURL(CefString(url));
		CefRefPtr<CefRequest> request = CefRequest::Create();
		request->SetURL(CefString(std::wstring(url)));
		m_handler->GetBrowser()->GetMainFrame()->LoadRequest(request);
		ret = true;
	}
	return ret;
}

void ChromeiumBrowserControl::back()
{
	if (m_handler->GetBrowser().get())
	{
		m_handler->GetBrowser()->GoBack();
	}
}

void ChromeiumBrowserControl::forward()
{
	if (m_handler->GetBrowser().get())
	{
		m_handler->GetBrowser()->GoForward();
	}
}

bool ChromeiumBrowserControl::close()
{
	if ( m_bClose )
	{
		return true;
	}
	if (!m_handler->IsClosing() && m_handler->GetBrowser().get() && m_handler->GetBrowser()->GetHost().get() )
	{
		m_handler->GetBrowser()->GetHost()->CloseBrowser(true);
		m_bClose = true;
	}
	return m_bClose;
}

void ChromeiumBrowserControl::reload()
{
	if ( m_handler->GetBrowser().get() )
	{
		m_handler->GetBrowser()->Reload();
	}
}

void ChromeiumBrowserControl::reloadIgnoreCache()
{
	if (m_handler->GetBrowser().get())
	{
		m_handler->GetBrowser()->ReloadIgnoreCache();
	}
}

bool ChromeiumBrowserControl::IsAudioMuted()
{
	if (m_handler->GetBrowser().get() && m_handler->GetBrowser()->GetHost().get()){
		return m_handler->GetBrowser()->GetHost()->IsAudioMuted();
	}
	return false;
}

void ChromeiumBrowserControl::SetAudioMuted(const bool& bEnable)
{
	if (m_handler->GetBrowser().get() && m_handler->GetBrowser()->GetHost().get()){
		m_handler->GetBrowser()->GetHost()->SetAudioMuted(bEnable);
	}
}

void ChromeiumBrowserControl::Stop()
{
	if (m_handler->GetBrowser().get())
	{
		m_handler->GetBrowser()->StopLoad();
	}
}

bool ChromeiumBrowserControl::asyncInvokedJSMethod(const char* utf8_module, const char* utf8_method,
	const char* utf8_parm,
	const char* utf8_frame_name, bool bNoticeJSTrans2JSON)
{
	bool ret = false;
	if (m_handler.get() && m_handler->GetBrowser().get())
	{
		ret = m_handler->asyncInvokedJSMethod(utf8_module, utf8_method, utf8_parm, utf8_frame_name, bNoticeJSTrans2JSON);
	}
	return ret;
}

////////////////////////////////////////////////////

WebkitControl::WebkitControl()
{
	m_ipc_id = 0;
	m_defWinProc = NULL;
	m_browser = new ChromeiumBrowserControl;
}

WebkitControl::~WebkitControl()
{
}

HWND WebkitControl::AttachHwnd(HWND hParentWnd, const WCHAR* url, const WCHAR* cookie_context)
{
#if defined _M_AMD64 || defined _WIN64
	m_defWinProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hParentWnd, GWLP_WNDPROC));
	::SetWindowLongPtr(m_MainHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HostWndProc));
	//::SetWindowLongPtr(hParentWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#else
	m_defWinProc = reinterpret_cast<WNDPROC>(::GetWindowLong(hParentWnd, GWL_WNDPROC));
	//设置自定义窗口过程
	::SetWindowLong(hParentWnd, GWL_WNDPROC, reinterpret_cast<LONG>(HostWndProc));
	//::SetWindowLong(hParentWnd, GWL_USERDATA, reinterpret_cast<LONG>(this));
#endif
	return m_browser->AttachHwnd(hParentWnd, url, cookie_context);
}

void WebkitControl::handle_size(HWND hWnd)
{
	m_browser->handle_size(hWnd);
}

void WebkitControl::handle_SetForce()
{
	m_browser->handle_SetForce();
}

LRESULT WebkitControl::HostWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByHostHwnd(hWnd);
	if ( !IsWindow(hWnd) )
	{
		return 0;
	}

	if ( !control.get() )
	{
		return CallWindowProc(DefWindowProc, hWnd, message, wParam, lParam);
	}
	switch (message)
	{
	case WM_SIZE:
	{
		control->handle_size(hWnd);
		return CallWindowProc(control->m_defWinProc, hWnd, message, wParam, lParam);
		break;
	}
	default:
		return CallWindowProc(control->m_defWinProc, hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}