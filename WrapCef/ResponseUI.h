#ifndef _responseui_h_
#define _responseui_h_
#pragma once
#include "ResponseHandle.h"

class ResponseUI : public ResponseHandle
{
public:
	ResponseUI();
	virtual ~ResponseUI();

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
};

#endif