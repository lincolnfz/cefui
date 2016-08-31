#include "stdafx.h"
#include "BridageRender.h"
#include "WebViewFactory.h"
#include "ResponseUI.h"
#include "json/json.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

bool _parseCreateWindowParm(std::string& json, long& x, long& y, long& width, long& height, long& min_cx, long& min_cy, long& max_cx, long& max_cy,
	std::string& skin, long& alpha, unsigned long& ulStyle, unsigned long& extra, unsigned long& parentSign)
{
	bool ret = false;
	Json::Reader read;
	Json::Value root;
	if (read.parse(json, root)){
		x = root.get("x", 0).asInt();
		y = root.get("y", 0).asInt();
		width = root.get("width", 0).asInt();
		height = root.get("height", 0).asInt();
		min_cx = root.get("min_cx", 0).asInt();
		min_cy = root.get("min_cy", 0).asInt();
		max_cx = root.get("max_cx", 0).asInt();
		max_cy = root.get("max_cy", 0).asInt();
		skin = root.get("skin", 0).asString();
		alpha = root.get("alpha", 0).asInt();
		ulStyle = root.get("ulStyle", 0).asUInt();
		extra = root.get("extra", 0).asUInt();
		parentSign = root.get("parentSign", 0).asUInt();
		ret = true;
	}

	return ret;
}

std::wstring _char2wchar(const std::string& str)
{
	int iTextLen = 0;
	iTextLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	WCHAR* wsz = new WCHAR[iTextLen + 1];
	memset((void*)wsz, 0, sizeof(WCHAR) * (iTextLen + 1));
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wsz, iTextLen);
	std::wstring strText(wsz);
	delete[]wsz;
	return strText;
}

BridageRender BridageRender::s_inst;

BridageRender& BridageRender::getInst()
{
	return s_inst;
}

BridageRender::BridageRender()
{
	REGISTER_RESPONSE_ACK_FUNCTION(BridageRender, rsp_getPrivateProfileString);
	REGISTER_RESPONSE_ACK_FUNCTION(BridageRender, rsp_createWindow);
	REGISTER_RESPONSE_ACK_FUNCTION(BridageRender, rsp_createModalWindow);
	REGISTER_RESPONSE_ACK_FUNCTION(BridageRender, rsp_createModalWindow2);
}


BridageRender::~BridageRender()
{
}

bool BridageRender::SendRequest(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue>& val, int timeout/* = REQ_TIME_OUT*/)
{
	return _SendRequest(browser, PID_RENDERER, msg, val, timeout);
}

bool BridageRender::rsp_getPrivateProfileString(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue> response, bool& responseAck)
{
	CefString str = msg->GetArgumentList()->GetString(0);
	response->SetString(0, CefString(L"rcp²âÊÔ"));
	responseAck = false;

	CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("getPrivateProfileString");
	message->GetArgumentList()->SetString(0, CefString(L"ÖÐÄÐis_editable"));
	CefRefPtr<CefListValue> val;
	//SendRequest(browser, message, val, 6000000);
	return true;
}

void myCreateWindow(HWND hWnd, std::string parm)
{
	long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
	std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;

	if (_parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
	{
		//ResponseUI::getFunMap()->createWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, _char2wchar(skin), alpha, ulStyle, true, extra);
	}
}

bool BridageRender::rsp_createWindow(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue>, bool&)
{
	bool ret = true;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (ResponseUI::getFunMap()){
			std::string parm = msg->GetArgumentList()->GetString(0).ToString();
			CefPostTask(TID_UI, base::Bind(&myCreateWindow, hWnd, parm));

			/*long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;
					
			if (_parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
			{
				ResponseUI::getFunMap()->createWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, _char2wchar(skin), alpha, ulStyle, extra);
				ret = true;
			}*/
		}
	}
	return ret;
}

bool BridageRender::rsp_createModalWindow(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue>, bool&)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (ResponseUI::getFunMap()){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;
			std::string parm = msg->GetArgumentList()->GetString(0).ToString();
			if (_parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
			{
				//ResponseUI::getFunMap()->createModalWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, _char2wchar(skin), alpha, ulStyle, true, extra);
				ret = true;
			}
		}
	}
	return ret;
}

bool BridageRender::rsp_createModalWindow2(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue>, bool&)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (ResponseUI::getFunMap()){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;
			std::string parm = msg->GetArgumentList()->GetString(0).ToString();
			if (_parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
			{
				//ResponseUI::getFunMap()->createModalWindow2(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, _char2wchar(skin), alpha, ulStyle, true, extra, parentSign);
				ret = true;
			}
		}
	}
	return ret;
}