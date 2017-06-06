#ifndef _webkitcontrol_h
#define _webkitcontrol_h
#pragma once
#include "cefclient.h"
#include "cookie_impl.h"

class ChromeiumBrowserControl : public CefBase
{
public:
	ChromeiumBrowserControl(){
		m_bClose = false;
	}
	virtual ~ChromeiumBrowserControl(){}
	HWND AttachHwnd(HWND, const WCHAR*, const WCHAR* cookie_context, const bool skipcache);

	void handle_size(HWND);
	void handle_SetForce();

	CefRefPtr<CefBrowser> getBrowser(){
		return m_browser;
	}

	bool setBrowser(CefRefPtr<CefBrowser> browser);

	bool setClientHandler(CefRefPtr<ClientHandler> clientHandler);

	bool close();
	bool loadUrl(const WCHAR* url, const bool skipCache = false);
	void back();
	void forward();
	void reload();
	void reloadIgnoreCache();
	bool IsAudioMuted();
	void SetAudioMuted(const bool& bEnable);
	void Stop();
	bool asyncInvokedJSMethod(const char* utf8_module, const char* utf8_method,
		const char* utf8_parm,
		const char* utf8_frame_name, bool bNoticeJSTrans2JSON);
	void AdjustRenderSpeed( const double& dbSpeed);
	void SendMouseClickEvent(const unsigned int& msg, const long& wp, const long& lp);
	void InitLoadUrl();
	bool InjectJS(const WCHAR* js);
	IMPLEMENT_REFCOUNTING(ChromeiumBrowserControl);
private:
	CefRefPtr<ClientHandler> m_handler;
	CefRefPtr<CefBrowser> m_browser;
	bool m_bClose;
	CefRefPtr<RequestContextHandlerPath> m_requestContextHandler;
	std::wstring m_strInitUrl;

};

class WebkitControl : public CefBase
{
public:
	WebkitControl();
	virtual ~WebkitControl();

	void SubWindow_Proc(const HWND&);
	HWND AttachHwnd(const HWND&, const WCHAR*, const WCHAR* cookie_context, const bool skipcache);
	void handle_size(HWND);
	void handle_SetForce();
	CefRefPtr<ChromeiumBrowserControl> getBrowser(){
		return m_browser;
	}
	const int& getIpcID(){
		return m_ipc_id;
	}

	void setIpcID(const int& id){
		m_ipc_id = id;
	}
	void InitLoadUrl();
	IMPLEMENT_REFCOUNTING(WebkitControl);
protected:
	static LRESULT __stdcall HostWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	CefRefPtr<ChromeiumBrowserControl> m_browser;
	WNDPROC m_defWinProc;
	int m_ipc_id;
};

#endif