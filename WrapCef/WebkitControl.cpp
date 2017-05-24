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
			item = CefCookieManager::CreateManager(CefString(path), false, NULL);
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

bool ChromeiumBrowserControl::setBrowser(CefRefPtr<CefBrowser> browser)
{
	if (m_browser.get())
	{
		assert(m_browser.get() == nullptr);
		return false;
	}
	m_browser = browser;
	return true;
}

bool ChromeiumBrowserControl::setClientHandler(CefRefPtr<ClientHandler> clientHandler)
{
	if (m_handler.get())
	{
		assert(m_handler.get() == nullptr);
		return false;
	}
	m_handler = clientHandler;
	return true;
}

HWND ChromeiumBrowserControl::AttachHwnd(HWND hParent, const WCHAR* url, const WCHAR* cookie_context, const bool skipcache)
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
	//browser_settings.universal_access_from_file_urls = STATE_ENABLED; //让xpack访问本地文件
	//browser_settings.Set()

	RECT rect;

	GetClientRect(hParent, &rect);
	info.SetAsChild(hParent, rect);

	CefRefPtr<CefRequestContext> request_context;
	if (cookie_context && wcslen(cookie_context) > 0){
		m_requestContextHandler = new RequestContextHandler(cookie_context);
		CefRequestContextSettings settings;
		CefString(&settings.cache_path) = CefString(cookie_context);
		request_context = CefRequestContext::CreateContext(settings, m_requestContextHandler);
	}
	CefBrowserHost::CreateBrowser(info, m_handler.get(),
		skipcache ? L"about:blank" : url, browser_settings, request_context.get() ? request_context : NULL);

	HWND hWnd = NULL;
	if (m_browser.get() && m_browser->GetHost()){
		hWnd = m_browser->GetHost()->GetWindowHandle();
		//loadUrl(url);
		if ( skipcache )
		{
			m_strInitUrl = url;
		}
	}
	return hWnd;
}

void ChromeiumBrowserControl::handle_size(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	if (m_browser.get())
	{
		CefWindowHandle hBrowser = m_browser->GetHost()->GetWindowHandle();
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

bool ChromeiumBrowserControl::loadUrl(const WCHAR* url, const bool skipCache /*= false*/)
{
	bool ret = false;
	if (m_browser.get() && m_browser->GetMainFrame().get())
	{
		//m_browser->GetMainFrame()->LoadURL(CefString(url));
		CefRefPtr<CefRequest> request = CefRequest::Create();
		if ( !skipCache )
		{
			int flag = request->GetFlags();
			flag |= UR_FLAG_SKIP_CACHE;
			request->SetFlags(flag);
		}
		request->SetURL(CefString(std::wstring(url)));
		m_browser->GetMainFrame()->LoadRequest(request);
		ret = true;
	}
	return ret;
}

void ChromeiumBrowserControl::back()
{
	if (m_browser.get())
	{
		m_browser->GoBack();
	}
}

void ChromeiumBrowserControl::forward()
{
	if (m_browser.get())
	{
		m_browser->GoForward();
	}
}

bool ChromeiumBrowserControl::close()
{
	if ( m_bClose )
	{
		return true;
	}
	if (!m_handler->IsClosing() && m_browser.get() && m_browser->GetHost().get())
	{
		m_browser->GetHost()->CloseBrowser(true);
		m_bClose = true;
	}
	return m_bClose;
}

void ChromeiumBrowserControl::reload()
{
	if (m_browser.get())
	{
		m_browser->Reload();
	}
}

void ChromeiumBrowserControl::reloadIgnoreCache()
{
	if (m_browser.get())
	{
		m_browser->ReloadIgnoreCache();
	}
}

void ChromeiumBrowserControl::InitLoadUrl()
{
	if (!m_strInitUrl.empty()){
		loadUrl(m_strInitUrl.c_str(), true);
		m_strInitUrl.clear();
	}
}

bool ChromeiumBrowserControl::IsAudioMuted()
{
	if (m_browser.get() && m_browser->GetHost().get()){
		return m_browser->GetHost()->IsAudioMuted();
	}
	return false;
}

void ChromeiumBrowserControl::SetAudioMuted(const bool& bEnable)
{
	if (m_browser.get() && m_browser->GetHost().get()){
		m_browser->GetHost()->SetAudioMuted(bEnable);
	}
}

void ChromeiumBrowserControl::Stop()
{
	if (m_browser.get())
	{
		m_browser->StopLoad();
	}
}

bool ChromeiumBrowserControl::asyncInvokedJSMethod(const char* utf8_module, const char* utf8_method,
	const char* utf8_parm,
	const char* utf8_frame_name, bool bNoticeJSTrans2JSON)
{
	bool ret = false;
	if (m_handler.get() && m_browser.get())
	{
		ret = m_handler->asyncInvokedJSMethod(m_browser, utf8_module, utf8_method, utf8_parm, utf8_frame_name, bNoticeJSTrans2JSON);
	}
	return ret;
}

bool ChromeiumBrowserControl::InjectJS(const WCHAR* js)
{
	bool ret = false;
	if (m_handler.get() && m_browser.get())
	{
		ret = m_handler->initiativeInjectJS(m_browser, js);
	}
	return ret;
}

void ChromeiumBrowserControl::AdjustRenderSpeed(const double& dbSpeed)
{
	if (m_handler.get() && m_browser.get())
	{
		m_handler->AdjustRenderSpeed(m_browser, dbSpeed);
	}
}

void ChromeiumBrowserControl::SendMouseClickEvent(const unsigned int& msg, const long& wp, const long& lp)
{
	if (m_handler.get() && m_browser.get())
	{
		m_handler->SendMouseClickEvent(m_browser, msg, wp, lp);
	}
}

////////////////////////////////////////////////////

WebkitControl::WebkitControl()
{
	m_ipc_id = 0;
	m_defWinProc = NULL;
	m_browser = new ChromeiumBrowserControl();
}

WebkitControl::~WebkitControl()
{
}

void WebkitControl::SubWindow_Proc(const HWND& hParentWnd)
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
}

HWND WebkitControl::AttachHwnd(const HWND& hParentWnd, const WCHAR* url, const WCHAR* cookie_context, const bool skipcache)
{
	SubWindow_Proc(hParentWnd);
	return m_browser->AttachHwnd(hParentWnd, url, cookie_context, skipcache);
}

void WebkitControl::handle_size(HWND hWnd)
{
	m_browser->handle_size(hWnd);
}

void WebkitControl::handle_SetForce()
{
	m_browser->handle_SetForce();
}

void WebkitControl::InitLoadUrl()
{
	m_browser->InitLoadUrl();
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