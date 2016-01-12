#include "stdafx.h"
#include "BridageHost.h"

BridageHost BridageHost::s_inst;

BridageHost::BridageHost()
{
	REGISTER_RESPONSE_ACK_FUNCTION(BridageHost, rsp_invokedMethod);
}


BridageHost::~BridageHost()
{
}

BridageHost& BridageHost::getInst()
{
	return s_inst;
}

bool BridageHost::SendRequest(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue>& val, int timeout/* = REQ_TIME_OUT*/)
{
	return _SendRequest(browser, PID_BROWSER, msg, val, timeout);
}

bool BridageHost::rsp_invokedMethod(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue> response, bool& responseAck)
{
	CefString str = msg->GetArgumentList()->GetString(0);
	response->SetString(0, CefString(L"rsp_invokedMethod≤‚ ‘"));
	responseAck = false;
	return true;
}