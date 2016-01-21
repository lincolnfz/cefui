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
	bool rsp_RegisterBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);
};

#endif