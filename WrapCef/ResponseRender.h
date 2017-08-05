#ifndef _responserender_h
#define _responserender_h
#pragma once
#include "ResponseHandle.h"

struct DocLoadComplate 
{
	int browsr_;
	bool bComp_;
	DocLoadComplate(int id, bool comp){
		browsr_ = id;
		bComp_ = comp;
	}
};

//指示主框架dom加载完毕（仅用在ui界面）
class DocComplate
{
public:
	static DocComplate s_inst;

	static DocComplate& getInst(){
		return s_inst;
	}

	bool setBrowsr(int id, bool comp);

	bool hitBrowser(int id);

protected:
	DocComplate(){

	}

	virtual ~DocComplate(){

	}

protected:
	std::map<int, DocLoadComplate> docLoadMap_;
};

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

	bool rsp_injectJS(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_asyncInvokedJSMethod(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_AdjustRenderSpeed(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_initiativeInjectJS(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_ackSendData(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);
};

#endif