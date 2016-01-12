#pragma once
#include "Bridage.h"

class BridageRender :
	public Bridage
{
public:	
	~BridageRender();

	static BridageRender& getInst();

	virtual bool SendRequest(CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>&, int timeout = REQ_TIME_OUT) OVERRIDE;

	bool rsp_getPrivateProfileString(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>, bool&);

protected:
	BridageRender();

	static BridageRender s_inst;
};

