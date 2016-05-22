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

	void CreateNewWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie_ctx);

	bool CloseWebControl(const HWND& hwnd);

	CefRefPtr<CefBrowser> GetBrowser(int browserID);

	CefRefPtr<WebkitControl> GetWebkitControlByID(int browserID);

	CefRefPtr<WebkitControl> GetWebkitControlByHostHwnd(HWND hWnd);

	bool Loadurl(const HWND& hwnd, const WCHAR* url);

	void CloseAll();

	bool GoBack(const HWND& hwnd);

	bool GoForward(const HWND& hwnd);

	bool Reload(const HWND& hwnd);

	bool ReloadIgnoreCache(const HWND& hwnd);

	bool IsAudioMuted(const HWND& hwnd);

	void SetAudioMuted(const HWND& hwnd, const bool& bEnable);

protected:
	NormalWebFactory();
	static NormalWebFactory s_inst;
	NormalWebMap m_map;
};

#endif