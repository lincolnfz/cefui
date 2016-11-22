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
	bool removeHwnd(const HWND& hWnd)
	{
		bool ret = false;
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.find(hWnd);
		if ( it != m_window_map.end() )
		{
			m_window_map.erase(it);
			ret = true;
		}
		return ret;
	}

	bool isHit(const HWND& hWnd)
	{
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.find(hWnd);
		return it != m_window_map.end();
	}

	bool isHitItem(const int browserID)
	{
		bool ret = false;
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.begin();
		for (; it != m_window_map.end(); ++it)
		{
			if (it->second->getProvider().get() && it->second->getProvider()->GetBrowser().get()){
				if (it->second->getProvider()->GetBrowser()->GetIdentifier() == browserID){
					ret = true;
					break;
				}
			}
		}
		return ret;
	}

	int size(){
		return m_window_map.size();
	}

	CefRefPtr<CefBrowser> getBrowser(const int& id)
	{
		CefRefPtr<CefBrowser> ptr;
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.begin();
		for (; it != m_window_map.end(); ++it)
		{
			if (it->second->getProvider().get() && it->second->getProvider()->GetBrowser().get()){
				if (it->second->getProvider()->GetBrowser()->GetIdentifier() == id){
					ptr = it->second->getProvider()->GetBrowser();
					break;
				}
			}
		}
		return ptr;
	}

	CefRefPtr<CefBrowser> getBrowserByHwnd(const HWND& hWnd)
	{
		CefRefPtr<CefBrowser> ptr;
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.find(hWnd);
		if ( it != m_window_map.end() )
		{
			ptr = it->second->getProvider()->GetBrowser();
		}
		return ptr;
	}

	void closeAll(){
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.begin();
		for (; it != m_window_map.end(); ++it)
		{
			if (it->second->getProvider().get() && it->second->getProvider()->GetBrowser().get() &&
				it->second->getProvider()->GetBrowser()->GetHost().get() ){
				it->second->getProvider()->GetBrowser()->GetHost()->CloseBrowser(true);
			}
		}
	}

	CefRefPtr<OSRWindow> getWindowByHwnd(const HWND& hWnd)
	{
		CefRefPtr<OSRWindow> win;
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.find(hWnd);
		if ( it != m_window_map.cend() )
		{
			win = it->second;
		}
		return win;
	}

	CefRefPtr<OSRWindow> getWindowByID(const int& id)
	{
		CefRefPtr<OSRWindow> ptr;
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.begin();
		for (; it != m_window_map.end(); ++it)
		{
			if (it->second->getProvider().get() && it->second->getProvider()->GetBrowser().get()){
				if (it->second->getProvider()->GetBrowser()->GetIdentifier() == id){
					ptr = it->second;
					break;
				}
			}
		}
		return ptr;
	}

	void clearData(int compType){
		std::map<HWND, CefRefPtr<OSRWindow>>::iterator it = m_window_map.begin();
		if ( it != m_window_map.end() )
		{
			it->second->getProvider()->GetBrowser()->GetHost()->CleaarData(compType);
		}
	}

	CefRefPtr<ClientHandler> m_handle;
	std::map<HWND, CefRefPtr<OSRWindow>> m_window_map;
	int m_ipcID;
	IMPLEMENT_REFCOUNTING(WebItem);
};


class BrowserProvider : public OSRBrowserProvider {
public:
	virtual CefRefPtr<CefBrowser> GetBrowser() OVERRIDE{
		if (handle_.get()){
			return handle_->GetBrowserByID(id_);
		}

		return NULL;
	}
	BrowserProvider(CefRefPtr<ClientHandler> handle){
		handle_ = handle;
		id_ = 0;
		hwnd_ = 0;
	}

	virtual CefRefPtr<ClientHandler> GetClientHandler() OVERRIDE{
		return handle_;
	}

	virtual void UpdateBrowserInfo(int id, HWND hwnd) OVERRIDE{
		id_ = id;
		hwnd_ = hwnd;
	}

private:
	CefRefPtr<ClientHandler> handle_;

	int id_;

	HWND hwnd_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(BrowserProvider);
};

typedef std::list<CefRefPtr<WebItem>> WebViewList;

class WebViewFactory : public CefBase
{
public:
	~WebViewFactory();
	static WebViewFactory& getInstance(){
		return s_inst;
	}

	HWND GetWebView(const HWND& hSameProcessWnd, const HINSTANCE& hInstance, const int& x, const int& y, const int& width,
		const int& height, const CefString& url, const int& alpha, const bool& taskbar, const bool& trans, const int& sizetype);

	CefRefPtr<WebItem> FindItem(const HWND& hWnd);

	void RemoveWindow(HWND);

	CefRefPtr<CefBrowser> GetBrowser(int browserID);

	CefRefPtr<WebItem> GetBrowserItem(int browserID);

	HWND GetBrowserHwndByID(int browserID);

	CefRefPtr<CefBrowser> GetBrowserByHwnd(const HWND& hWnd);

	CefRefPtr<OSRWindow> getWindowByHwnd(const HWND& hWnd);

	CefRefPtr<OSRWindow> getWindowByID(const int& browserID);

	void CloseAll();

	void ClearData(int compType);

	bool InjectJS(const HWND& hwnd, const WCHAR* js);

protected:
	WebViewFactory();
	static WebViewFactory s_inst;
	WebViewList m_viewList;
	//std::mutex factoryMutex_;

	IMPLEMENT_REFCOUNTING(WebViewFactory);
};

#endif

