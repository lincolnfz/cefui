#ifndef _normalwebfactory_h
#define _normalwebfactory_h
#pragma once
#include <map>

#include "include/base/cef_lock.h"
#include "include/cef_client.h"
#include "include/wrapper/cef_helpers.h"
#include "WebkitControl.h"

typedef std::map<HWND, CefRefPtr<WebkitControl>> NormalWebMap;

class NormalWebFactory
{
public:
	~NormalWebFactory();
	static NormalWebFactory& getInstance(){
		return s_inst;
	}

	void CreateNewWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie_ctx, const bool skipcache);

	void CreateNewWebControl(const HWND& hwnd, CefRefPtr<ClientHandler> clientHandle);

	bool CloseWebControl(const HWND& hwnd);

	CefRefPtr<CefBrowser> GetBrowser(int browserID);

	CefRefPtr<WebkitControl> GetWebkitControlByID(int browserID);

	CefRefPtr<WebkitControl> GetWebkitControlByHostHwnd(HWND hWnd);

	bool SetBrowser(HWND hWnd, CefRefPtr<CefBrowser> browser);

	bool Loadurl(const HWND& hwnd, const WCHAR* url);

	void CloseAll();

	bool GoBack(const HWND& hwnd);

	bool GoForward(const HWND& hwnd);

	bool Reload(const HWND& hwnd);

	bool ReloadIgnoreCache(const HWND& hwnd);

	bool IsAudioMuted(const HWND& hwnd);

	void SetAudioMuted(const HWND& hwnd, const bool& bEnable);

	bool Stop(const HWND& hwnd);

	bool asyncInvokedJSMethod(const HWND& hwnd, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm, 
		const char* utf8_frame_name, bool bNoticeJSTrans2JSON);

	void AdjustRenderSpeed(const HWND& hWnd, const double& dbSpeed);

	void SendMouseClickEvent(const HWND& hWnd, const unsigned int& msg, const long& wp, const long& lp);

	bool InjectJS(const HWND& hwnd, const WCHAR* js);

	bool QueryRenderProcessID(const HWND&, int& pid);

	bool QueryPluginsProcessID(const HWND&, std::vector<DWORD>& plugins_process_ids);

	bool GetViewZoomLevel(const HWND&, double& level);

	bool SetViewZoomLevel(const HWND&, const double& level);

protected:
	NormalWebFactory();
	static NormalWebFactory s_inst;
	NormalWebMap m_map;
};

#endif