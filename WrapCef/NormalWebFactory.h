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

	void CreateNewWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie);

	void CloseWebControl(const HWND& hwnd);

	CefRefPtr<CefBrowser> GetBrowser(int browserID);

	CefRefPtr<WebkitControl> GetWebkitControl(int browserID);

	void CloseAll();

protected:
	NormalWebFactory();
	static NormalWebFactory s_inst;
	NormalWebMap m_map;
};

#endif