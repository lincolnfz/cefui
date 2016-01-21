#include "stdafx.h"
#include "ResponseRender.h"


ResponseRender::ResponseRender()
{
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_invokedJSMethod);
}


ResponseRender::~ResponseRender()
{
}

bool ResponseRender::rsp_invokedJSMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	outVal->getList().AppendVal(std::wstring(L"fdseea99"));
	return true;
}