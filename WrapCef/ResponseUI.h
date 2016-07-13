#ifndef _responseui_h_
#define _responseui_h_
#pragma once
#include "ResponseHandle.h"
#include <map>
#include "WrapCef.h"

class ResponseUI : public ResponseHandle
{
public:
	ResponseUI();
	virtual ~ResponseUI();
	static void SetFunMap(wrapQweb::FunMap*);
	static const wrapQweb::FunMap* getFunMap(){
		return s_fnMap;
	}
protected:
	WCHAR m_szName[MAX_PATH];
	WCHAR m_szPath[MAX_PATH];

protected:	
	bool rsp_RegisterBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_getPrivateProfileString(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_window_x(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_window_y(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_window_w(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_window_h(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_is_zoomed(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_is_iconic(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_appname(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_appDir(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_minWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_maxWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_restoreWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_closeWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_setWindowText(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_writePrivateProfileString(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_getPrivateProfileInt(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_setProfile(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_getProfile(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_setWindowSize(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_setWindowPos(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_setAlpha(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_getSoftwareAttribute(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_winProty(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_createWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_createModalWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_createModalWindow2(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_invokeMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_crossInvokeWebMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_crossInvokeWebMethod2(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_fullScreen(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_appDataPath(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_asyncCallMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
	bool rsp_getInjectJS(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
protected:
	std::map<unsigned int, std::wstring> m_profile;
	static wrapQweb::FunMap* s_fnMap;
};

#endif