#include "stdafx.h"
#include "ResponseRender.h"
#include <boost/format.hpp>
#include <include/cef_v8.h>
//#include <iostream>
#include <sstream>
#include "speedbox.h"
//#include "client_app.h"


DocComplate DocComplate::s_inst;

bool DocComplate::setBrowsr(int id, bool comp)
{
	std::map<int, DocLoadComplate>::iterator it = docLoadMap_.find(id);
	if (it != docLoadMap_.end())
	{
		it->second.bComp_ = comp;
	}
	else{
		DocLoadComplate item(id, comp);
		docLoadMap_.insert(std::make_pair(id, item));
	}
	return true;
}

bool DocComplate::hitBrowser(int id)
{
	bool bFind = false;
	std::map<int, DocLoadComplate>::iterator it = docLoadMap_.find(id);
	if ( it != docLoadMap_.end() )
	{
		bFind = it->second.bComp_;
	}
	return bFind;
}

struct _HitTest
{
	WORD wHitCode;
	wchar_t szHitTag[16];
	bool bZoom;
};

_HitTest hitTestData[] = {
	{ HTCAPTION, L"caption", true },
	{ HTTOP, L"top", false },
	{ HTTOPLEFT, L"topleft", false },
	{ HTTOPRIGHT, L"topright", false },
	{ HTLEFT, L"left", false },
	{ HTRIGHT, L"right", false },
	{ HTBOTTOM, L"bottom", false },
	{ HTBOTTOMLEFT, L"bottomleft", false },
	{ HTBOTTOMRIGHT, L"bottomright", false }
};

ResponseRender::ResponseRender()
{
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_invokedJSMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_callJSMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_queryElementAttrib);
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_injectJS);
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_asyncInvokedJSMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseRender, rsp_AdjustRenderSpeed);
}


ResponseRender::~ResponseRender()
{
}

template <class T>
std::wstring ConvertToString(T value) {
	std::wstringstream ss;
	ss << value;
	return ss.str();
}

std::wstring V8ValtoWstring(CefRefPtr<CefV8Value> val)
{
	std::wstring strVal;
	if ( val.get() )
	{
		if ( val->IsBool() )
		{
			strVal = val->GetBoolValue() ? L"true" : L"false";
		}
		else if ( val->IsDouble() )
		{
			strVal = ConvertToString(val->GetDoubleValue());
		}
		else if ( val->IsInt() )
		{
			strVal = ConvertToString(val->GetIntValue());
		}
		else if ( val->IsUInt() )
		{
			strVal = ConvertToString(val->GetUIntValue());
		}
		else if ( val->IsNull() )
		{
			strVal = L"null";
		}
		else if ( val->IsString() )
		{
			strVal = val->GetStringValue().ToWString();
		}
		else if ( val->IsUndefined() )
		{
			strVal = L"undefined";
		}
		else{
			strVal = L"undefined";
		}
	}
	return strVal;
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
		boost::format fmt("window.invokeMethod('%1%', '%2%', '%3%', %4%)");
		fmt % module % method % parm % (bNotifyJson ? _ture:_false);
		std::string strJs = fmt.str();
		//frame->ExecuteJavaScript(CefString(strJs), CefString(""), 0);
		CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
		CefRefPtr<CefV8Value> retVal;
		CefRefPtr<CefV8Exception> excp;
		CefString cefjs(strJs);
#ifdef _DEBUG1
		char szTmp[512] = {0};
		char jspart[256] = { 0 };
		strncpy_s(jspart, strJs.c_str(), 255);
		sprintf_s(szTmp, "------js invoke in render  %d ;  %s", GetCurrentThreadId(),
			jspart);
		OutputDebugStringA(szTmp);
#endif
		bool bEval = false;
		if ( parm.empty())
		{
			bEval = v8->Eval(cefjs, retVal, excp);
		}
		else{
			bEval = v8->CallInvokeMethod(CefString("invokeMethod"), CefString(module), CefString(method), CefString(parm), bNotifyJson, retVal, excp);
		}
		
		if( bEval ){
			if (retVal.get()){
				outVal->getList().AppendVal(V8ValtoWstring(retVal));
			}
			ret = true;
		}else{
#ifdef _DEBUG1
			/*std::string err;
			if ( excp.get() )
			{
				int end = excp->GetEndPosition();
				int col = excp->GetEndColumn();
				int line = excp->GetLineNumber();
				//const char* p = excp->GetScriptResourceName().ToString().c_str();
				//excp->
				int startcol = excp->GetStartColumn();
				//OutputDebugStringA(excp->GetMessageW().ToString().c_str());
				err = excp->GetMessageW().ToString();
			}*/

			char szTmp[512] = { 0 };
			sprintf_s(szTmp, "------invokejs Fail! render  %d ;  %s", GetCurrentThreadId(),
				jspart);
			OutputDebugStringA(szTmp);
#endif
		}
	}

#ifdef _DEBUG1
	char szTmp[256] = { 0 };
	sprintf_s(szTmp, "------js invoke finish!!!  %d ; ", GetCurrentThreadId());
	OutputDebugStringA(szTmp);
#endif
	//assert(ret);
	return ret;
}

bool ResponseRender::rsp_callJSMethod(const CefRefPtr<CefBrowser> browser,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	bool ret = false;
	std::string funname = req_parm->getList().GetStrVal(0);
	std::string parm = req_parm->getList().GetStrVal(1);
	std::string frameName = req_parm->getList().GetStrVal(2);
	CefRefPtr<CefFrame> frame;
	if (frameName.empty())
	{
		frame = browser->GetMainFrame();
	}
	else{
		frame = browser->GetFrame(CefString(frameName));
	}
	if (frame.get())
	{
		boost::format fmt("%1%('%2%')");
		fmt % funname % parm;
		std::string strJs = fmt.str();
		CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
		CefRefPtr<CefV8Value> retVal;
		CefRefPtr<CefV8Exception> excp;
		if (v8->Eval(CefString(strJs), retVal, excp)){
			if (retVal.get() && retVal.get()->IsString()){
				outVal->getList().AppendVal(retVal->GetStringValue().ToWString());
			}
			ret = true;
		}
	}

	return ret;
}

bool ResponseRender::rsp_queryElementAttrib(const CefRefPtr<CefBrowser> browser,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	bool ret = false;
	int x = req_parm->getList().GetIntVal(0);
	int y = req_parm->getList().GetIntVal(1);
	int g_x = req_parm->getList().GetIntVal(2);
	int g_y = req_parm->getList().GetIntVal(3);
	double dt = req_parm->getList().GetDoubleVal(4);
	CefString val(L"");
	std::wstring attrib;
	if ( DocComplate::getInst().hitBrowser(browser->GetIdentifier()) )
	{
		browser->Query_xy_Element(x, y, g_x, g_y, dt, CefString(L"data-nc"), val);
		attrib = val.ToWString();
	}
	outVal->getList().AppendVal(attrib);
	bool bHandle = false;
	if (wcscmp(attrib.c_str(), L""))
	{
		int j = sizeof(hitTestData) / sizeof(hitTestData[0]);
		for (int i = 0; i < j; ++i)
		{
			if (_wcsicmp(attrib.c_str(), hitTestData[i].szHitTag) == 0){
				bHandle = true;
				break;
			}
		}
	}
	if ( bHandle )
	{
		CefRefPtr<CefFrame> frame;
		frame = browser->GetMainFrame();
		if ( frame.get() )
		{
			static std::string _ture("true");
			static std::string _false("false");
			boost::format fmt("window.invokeMethod('%1%', '%2%', '%3%', %4%)");
			fmt % "UI" % "NCMouseDown" % "" % (true ? _ture : _false);
			std::string strJs = fmt.str();
			//frame->ExecuteJavaScript(CefString(strJs), CefString(""), 0);
			CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
			CefRefPtr<CefV8Value> retVal;
			CefRefPtr<CefV8Exception> excp;
			CefString cefjs(strJs);
			bool bEval = false;
			bEval = v8->Eval(cefjs, retVal, excp);
		}
	}

	ret = true;
	return ret;
}

bool ResponseRender::rsp_injectJS(const CefRefPtr<CefBrowser> browser,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	bool ret = false;
	int64 frameID = req_parm->getList().GetInt64Val(0);
	CefString cefjs(req_parm->getList().GetWStrVal(1));
	CefRefPtr<CefFrame> frame = browser->GetFrame(frameID);
	if (frame.get())
	{
		CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
		CefRefPtr<CefV8Value> retVal;
		CefRefPtr<CefV8Exception> excp;
		v8->Eval(cefjs, retVal, excp);
		ret = true;
	}	
	return ret;
}

bool ResponseRender::rsp_asyncInvokedJSMethod(const CefRefPtr<CefBrowser> browser,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
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
	if (frame.get())
	{
		boost::format fmt("window.invokeMethod('%1%', '%2%', '%3%', %4%)");
		fmt % module % method % parm % (bNotifyJson ? _ture : _false);
		std::string strJs = fmt.str();
		CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
		CefRefPtr<CefV8Value> retVal;
		CefRefPtr<CefV8Exception> excp;
		CefString cefjs(strJs);
#ifdef _DEBUG1
		char szTmp[512] = { 0 };
		char jspart[256] = { 0 };
		strncpy_s(jspart, strJs.c_str(), 255);
		sprintf_s(szTmp, "------async js invoke in render  %d ;  %s", GetCurrentThreadId(),
			jspart);
		OutputDebugStringA(szTmp);
#endif
		bool bEval = false;
		if (parm.empty())
		{
			bEval = v8->Eval(cefjs, retVal, excp);
		}
		else{
			bEval = v8->CallInvokeMethod(CefString("invokeMethod"), CefString(module), CefString(method), CefString(parm), bNotifyJson, retVal, excp);
		}
		ret = bEval;
	}

#ifdef _DEBUG1
	char szTmp[256] = { 0 };
	sprintf_s(szTmp, "------async js invoke finish!!!  %d ; ", GetCurrentThreadId());
	OutputDebugStringA(szTmp);
#endif
	//assert(ret);
	return ret;
}

bool speed_adjust = false;
bool ResponseRender::rsp_AdjustRenderSpeed(const CefRefPtr<CefBrowser> browser,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	double dt = req_parm->getList().GetDoubleVal(0);
	if (fabs(dt - 1.0) < 0.01)
	{
		if (!speed_adjust)
		{
			EnableSpeedControl(FALSE);
		}
		else{
			EnableSpeedControl(TRUE);
			SetSpeed(1.001);
		}
		//OutputDebugString(_T("------------------ClientResponse::rsp_AdjustFlashSpeed disable "));
	}
	else{
		EnableSpeedControl(TRUE);
		SetSpeed(dt);
		speed_adjust = true;
		//OutputDebugString(_T("------------------ClientResponse::rsp_AdjustFlashSpeed enable "));
	}
	return true;
}