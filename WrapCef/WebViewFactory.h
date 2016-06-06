#ifndef _WEBVIEWFACTORY_H_
#define _WEBVIEWFACTORY_H_
#pragma once

#include "cefclient_osr_widget_win.h"
//#include <mutex>

class BrowserProvider;
class OSRWindow;
class WebItem : public CefBase
{
public:
	WebItem(){
		m_ipcID = 0;
	}
	~WebItem(){

	}
	CefRefPtr<ClientHandler> m_handle;
	CefRefPtr<BrowserProvider> m_provider;
	CefRefPtr<OSRWindow> m_window;
	int m_ipcID;
	IMPLEMENT_REFCOUNTING(WebItem);
};


class BrowserProvider : public OSRBrowserProvider {
public:
	virtual CefRefPtr<CefBrowser> GetBrowser() OVERRIDE{
		if (handle_.get())
		return handle_->GetBrowser();

		return NULL;
	}
		BrowserProvider(CefRefPtr<ClientHandler> handle){
		handle_ = handle;
	}

	virtual CefRefPtr<ClientHandler> GetClientHandler() OVERRIDE{
		return handle_;
	}

private:
	CefRefPtr<ClientHandler> handle_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(BrowserProvider);
};

typedef std::map<HWND, CefRefPtr<WebItem>> WebViewMap;

class WebViewFactory
{
public:
	~WebViewFactory();
	static WebViewFactory& getInstance(){
		return s_inst;
	}

	HWND GetWebView(const HINSTANCE& hInstance, const int& x, const int& y, const int& width,
		const int& height, const CefString& url, const int& alpha, const bool& taskbar, const bool& trans);

	CefRefPtr<WebItem> FindItem(const HWND hWnd);

	void RemoveWindow(HWND);

	CefRefPtr<CefBrowser> GetBrowser(int browserID);

	CefRefPtr<WebItem> GetBrowserItem(int browserID);

	HWND GetBrowserHwnd(int);

	void CloseAll();

	void ClearData(int compType);

protected:
	WebViewFactory();
	static WebViewFactory s_inst;
	WebViewMap m_viewMap;
	//std::mutex factoryMutex_;
};

#endif

