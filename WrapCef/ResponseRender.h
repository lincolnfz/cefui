#ifndef _responserender_h
#define _responserender_h
#pragma once
#include "ResponseHandle.h"

class ResponseRender : public ResponseHandle
{
public:
	ResponseRender();
	virtual ~ResponseRender();
	
	bool rsp_invokedJSMethod(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_callJSMethod(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_queryElementAttrib(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);
};

#endif