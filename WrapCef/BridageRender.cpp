#include "stdafx.h"
#include "BridageRender.h"

BridageRender BridageRender::s_inst;

BridageRender& BridageRender::getInst()
{
	return s_inst;
}

BridageRender::BridageRender()
{
	REGISTER_RESPONSE_ACK_FUNCTION(BridageRender, rsp_getPrivateProfileString);
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
	response->SetString(0, CefString(L"rcp≤‚ ‘"));
	responseAck = false;
	return true;
}