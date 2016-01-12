#pragma once
#include "Bridage.h"
class BridageHost :
	public Bridage
{
public:
	~BridageHost();
	static BridageHost& getInst();

	virtual bool SendRequest(CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>&, int timeout = REQ_TIME_OUT) OVERRIDE;

	bool rsp_invokedMethod(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>, bool&);

protected:
	BridageHost();

	static BridageHost s_inst;
};

