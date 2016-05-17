#include "stdafx.h"
#include "NormalWebFactory.h"

NormalWebFactory NormalWebFactory::s_inst;

NormalWebFactory::NormalWebFactory()
{
	
}


NormalWebFactory::~NormalWebFactory()
{
}

void NormalWebFactory::CreateNewWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie)
{
	CefRefPtr<WebkitControl> item = new  WebkitControl;
	item->AttachHwnd(hwnd, url);
	m_map.insert(std::make_pair(hwnd, item));
}

void NormalWebFactory::CloseWebControl(const HWND& hwnd)
{
	NormalWebMap::iterator it = m_map.find(hwnd);
	if ( it != m_map.end() )
	{
		if (it->second->getBrowser())
		{
			it->second->getBrowser()->getClientHandler()->GetBrowser()->GetHost()->CloseBrowser(true);
		}
	}
}

CefRefPtr<CefBrowser> NormalWebFactory::GetBrowser(int browserID)
{
	CefRefPtr<CefBrowser> browser;
	NormalWebMap::iterator it = m_map.begin();
	for (; it != m_map.end(); ++it)
	{
		CefRefPtr<CefBrowser> visit = it->second->getBrowser()->getClientHandler()->GetBrowser();
		if ( visit && visit->GetIdentifier() == browserID )
		{
			browser = visit;
			break;
		}
	}
	return browser;
}

CefRefPtr<WebkitControl> NormalWebFactory::GetWebkitControl(int browserID)
{
	CefRefPtr<WebkitControl> control;
	NormalWebMap::iterator it = m_map.begin();
	for (; it != m_map.end(); ++it)
	{
		CefRefPtr<CefBrowser> visit = it->second->getBrowser()->getClientHandler()->GetBrowser();
		if (visit && visit->GetIdentifier() == browserID)
		{
			control = it->second;
			break;
		}
	}
	return control;
}

void NormalWebFactory::CloseAll()
{
	NormalWebMap::iterator it = m_map.begin();
	for (; it != m_map.end(); ++it)
	{
		if (it->second->getBrowser())
		{
			it->second->getBrowser()->getClientHandler()->GetBrowser()->GetHost()->CloseBrowser(true);
		}
	}
}