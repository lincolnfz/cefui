#ifndef _BrowserIdentifier_h
#define _BrowserIdentifier_h
#pragma once

#include "include/cef_browser.h"
#include <map>

class BrowserIdentifier
{
public:
	virtual ~BrowserIdentifier();
	static BrowserIdentifier& GetInst(){
		return s_inst;
	}
	static BrowserIdentifier s_inst;

	CefRefPtr<CefBrowser> GetBrowser(int);
	bool InsertBrowser(int, CefRefPtr<CefBrowser>);
	bool RemoveBrowser(int);
protected:
	BrowserIdentifier();
	std::map<int, CefRefPtr<CefBrowser> > m_browserMap;
};

#endif