#include "stdafx.h"
#include "ResponseRender.h"
#include <boost/format.hpp>
#include <include/cef_v8.h>
//#include "client_app.h"

ResponseRender::ResponseRender()
{
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_invokedJSMethod);
}


ResponseRender::~ResponseRender()
{
}

bool ResponseRender::rsp_invokedJSMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	bool ret = false;
	static std::string _ture("true");
	static std::string _false("false");
	std::string module = req_parm->getList().GetStrVal(0);
	std::string method = req_parm->getList().GetStrVal(1);
	std::string parm = req_parm->getList().GetStrVal(2);
	std::string frameName = req_parm->getList().GetStrVal(3);
	bool bNotifyJson = req_parm->getList().GetBooleanVal(4);
	CefRefPtr<CefFrame> frame;
	if (frameName.empty())
	{
		frame = browser->GetMainFrame();
	}
	else{
		frame = browser->GetFrame(CefString(frameName));
	}
	if ( frame.get() )
	{		
		boost::format fmt("invokeMethod('%1%', '%2%', '%3%', %4%)");
		fmt % module % method % parm % (bNotifyJson ? _ture:_false);
		std::string strJs = fmt.str();
		//frame->ExecuteJavaScript(CefString(strJs), CefString(""), 0);
		CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
		CefRefPtr<CefV8Value> retVal;
		CefRefPtr<CefV8Exception> excp;
		if (v8->Eval(CefString(strJs), retVal, excp)){
			if (retVal->IsString()){
				outVal->getList().AppendVal(retVal->GetStringValue().ToString());
			}
			ret = true;
		}
	}
	
	return ret;
}