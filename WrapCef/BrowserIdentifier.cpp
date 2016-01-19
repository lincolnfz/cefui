#include "stdafx.h"
#include "BrowserIdentifier.h"

BrowserIdentifier BrowserIdentifier::s_inst;

BrowserIdentifier::BrowserIdentifier()
{
}


BrowserIdentifier::~BrowserIdentifier()
{
}

CefRefPtr<CefBrowser> BrowserIdentifier::GetBrowser(int id)
{
	CefRefPtr<CefBrowser> ptr;
	std::map<int, CefRefPtr<CefBrowser>>::iterator it = m_browserMap.find(id);
	if ( it != m_browserMap.end() )
	{
		ptr = it->second;
	}
	return ptr;
}

bool BrowserIdentifier::InsertBrowser(int id, CefRefPtr<CefBrowser> browser)
{
	std::pair<std::map<int, CefRefPtr<CefBrowser>>::iterator, bool> ret;
	ret = m_browserMap.insert(std::make_pair(id, browser));
	return ret.second;
}

bool BrowserIdentifier::RemoveBrowser(int id)
{
	bool ret = false;
	std::map<int, CefRefPtr<CefBrowser>>::iterator it = m_browserMap.find(id);
	if (it != m_browserMap.end())
	{
		m_browserMap.erase(it);
		ret = true;
	}
	return ret;
}