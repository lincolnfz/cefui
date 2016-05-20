#ifndef _webkitcontrol_h
#define _webkitcontrol_h
#pragma once
#include "cefclient.h"

class ChromeiumBrowserControl : public CefBase
{
public:
	ChromeiumBrowserControl(){
		m_bClose = false;
	}
	virtual ~ChromeiumBrowserControl(){}
	HWND AttachHwnd(HWND, const WCHAR*);
	void handle_size(HWND);
	void handle_SetForce();
	CefRefPtr<ClientHandler> getClientHandler(){
		return m_handler;
	}

	bool close();
	bool loadUrl(const WCHAR* url);
	void back();
	void forward();
	IMPLEMENT_REFCOUNTING(ChromeiumBrowserControl);
private:
	CefRefPtr<ClientHandler> m_handler;
	bool m_bClose;

};

class WebkitControl : public CefBase
{
public:
	WebkitControl();
	virtual ~WebkitControl();

	HWND AttachHwnd(HWND, const WCHAR*);
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
	IMPLEMENT_REFCOUNTING(WebkitControl);
protected:
	static LRESULT __stdcall HostWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	CefRefPtr<ChromeiumBrowserControl> m_browser;
	WNDPROC m_defWinProc;
	int m_ipc_id;
};

#endif