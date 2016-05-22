#include "stdafx.h"
#include "NormalWebFactory.h"

NormalWebFactory NormalWebFactory::s_inst;

NormalWebFactory::NormalWebFactory()
{
	
}


NormalWebFactory::~NormalWebFactory()
{
}

void NormalWebFactory::CreateNewWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie_ctx)
{
	CefRefPtr<WebkitControl> item = new  WebkitControl;
	item->AttachHwnd(hwnd, url, cookie_ctx);
	m_map.insert(std::make_pair(hwnd, item));
}

bool NormalWebFactory::CloseWebControl(const HWND& hwnd)
{
	bool ret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if ( it != m_map.end() )
	{
		if (it->second->getBrowser().get() )
		{
			if (it->second->getBrowser()->close())
			{
				m_map.erase(it);
				ret = true;
			}
		}		
	}
	return ret;
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

CefRefPtr<WebkitControl> NormalWebFactory::GetWebkitControlByID(int browserID)
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

CefRefPtr<WebkitControl> NormalWebFactory::GetWebkitControlByHostHwnd(HWND hWnd)
{
	CefRefPtr<WebkitControl> control;
	NormalWebMap::iterator it = m_map.find(hWnd);
	if ( it != m_map.end() )
	{
		control = it->second;
	}
	return control;
}

void NormalWebFactory::CloseAll()
{
	NormalWebMap::iterator it = m_map.begin();
	for (; it != m_map.end(); ++it)
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->close();
		}
	}
}

bool NormalWebFactory::Loadurl(const HWND& hwnd, const WCHAR* url)
{
	bool bret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if ( it->second->getBrowser().get() )
		{
			it->second->getBrowser()->loadUrl(url);
			bret = true;
		}
	}
	return bret;
}

bool NormalWebFactory::GoBack(const HWND& hwnd)
{
	bool bret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->back();
			bret = true;
		}
	}
	return bret;
}

bool NormalWebFactory::GoForward(const HWND& hwnd)
{
	bool bret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->forward();
			bret = true;
		}
	}
	return bret;
}

bool NormalWebFactory::Reload(const HWND& hwnd)
{
	bool bret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->reload();
			bret = true;
		}
	}
	return bret;
}

bool NormalWebFactory::ReloadIgnoreCache(const HWND& hwnd)
{
	bool bret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->reloadIgnoreCache();
			bret = true;
		}
	}
	return bret;
}

bool NormalWebFactory::IsAudioMuted(const HWND& hwnd)
{
	bool bret = false;
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->IsAudioMuted();
			bret = true;
		}
	}
	return bret;
}

void NormalWebFactory::SetAudioMuted(const HWND& hwnd, const bool& bEnable)
{
	NormalWebMap::iterator it = m_map.find(hwnd);
	if (it != m_map.end())
	{
		if (it->second->getBrowser().get())
		{
			it->second->getBrowser()->SetAudioMuted(bEnable);
		}
	}
}