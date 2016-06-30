#ifndef _BrowserIdentifier_h
#define _BrowserIdentifier_h
#pragma once

#include "include/cef_browser.h"
#include <map>

struct BrowserProty : public CefBase
{
	CefRefPtr<CefBrowser> m_browser;
	int m_type;
	IMPLEMENT_REFCOUNTING(BrowserProty);
};

class BrowserIdentifier
{
public:
	virtual ~BrowserIdentifier();
	static BrowserIdentifier& GetInst(){
		return s_inst;
	}
	static BrowserIdentifier s_inst;

	CefRefPtr<CefBrowser> GetBrowser(int);
	const int GetType(int);
	bool InsertBrowser(int, CefRefPtr<CefBrowser>);
	bool UpdateBrowserType(int, int);
	bool RemoveBrowser(int);
protected:
	BrowserIdentifier();
	std::map<int, CefRefPtr<BrowserProty> > m_browserMap;
};

#endif