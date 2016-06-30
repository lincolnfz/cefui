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
	std::map<int, CefRefPtr<BrowserProty>>::iterator it = m_browserMap.find(id);
	if ( it != m_browserMap.end() )
	{
		ptr = it->second->m_browser;
	}
	return ptr;
}

const int BrowserIdentifier::GetType(int id)
{
	int ret = 0;
	std::map<int, CefRefPtr<BrowserProty>>::iterator it = m_browserMap.find(id);
	if (it != m_browserMap.end())
	{
		ret = it->second->m_type;
	}
	return ret;
}

bool BrowserIdentifier::InsertBrowser(int id, CefRefPtr<CefBrowser> browser)
{
	std::pair<std::map<int, CefRefPtr<BrowserProty>>::iterator, bool> ret;
	CefRefPtr<BrowserProty> proty = new BrowserProty;
	proty->m_browser = browser;
	proty->m_type = 0;
	ret = m_browserMap.insert(std::make_pair(id, proty));
	return ret.second;
}

bool BrowserIdentifier::UpdateBrowserType(int id, int type)
{
	bool ret = false;
	std::map<int, CefRefPtr<BrowserProty>>::iterator it = m_browserMap.find(id);
	if (it != m_browserMap.end())
	{
		it->second->m_type = type;
		ret = true;
	}
	return ret;
}

bool BrowserIdentifier::RemoveBrowser(int id)
{
	bool ret = false;
	std::map<int, CefRefPtr<BrowserProty>>::iterator it = m_browserMap.find(id);
	if (it != m_browserMap.end())
	{
		m_browserMap.erase(it);
		ret = true;
	}
	return ret;
}