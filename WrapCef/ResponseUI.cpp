#include "stdafx.h"
#include "ResponseUI.h"
#include "IPC.h"
#include "WebViewFactory.h"

enum UI_CONTROL_MSG
{
	UI_CONTROL_MSG_BEGIN = WM_USER + 0x4000, // WM_USER ~ 0x7FFF     私有窗口类用的整数型消息。
	UI_CONTROL_MSG_CREATE_DLG,
	UI_CONTROL_MSG_DOMODAL_DLG,
	UI_CONTROL_MSG_SET_ALPHA,
	UI_CONTROL_MSG_NATIVE_FINISH,
	UI_CONTROL_MSG_SET_WINDOWPOS,
	UI_CONTROL_MSG_NEW_URL,
	UI_CONTROL_MSG_PUSH_MESSAGE,
	UI_CONTROL_MSG_CLOSE,
	UI_CONTROL_MSG_LOAD_FRAME, //每加载完一个frame，即触发消息
	UI_CONTROL_MSG_ZORDER_ADJUST,
	UI_CONTROL_MSG_END,
};

ResponseUI::ResponseUI()
{
	WCHAR szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	wchar_t* lch = wcsrchr(szPath, '\\');
	wcsncpy(m_szName, lch + 1, MAX_PATH);
	*lch = '\0';
	wcsncpy(m_szPath, szPath, MAX_PATH);

	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_RegisterBrowser);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_getPrivateProfileString);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_window_x);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_window_y);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_window_w);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_window_h);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_is_zoomed);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_is_iconic);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_appname);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_appDir);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_minWindow);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_maxWindow);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_restoreWindow);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_closeWindow);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_setWindowText);
}


ResponseUI::~ResponseUI()
{
}

bool ResponseUI::rsp_RegisterBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> reqParm, std::shared_ptr<cyjh::Instruct> )
{
	std::wstring szSrvPipe = reqParm->getList().GetWStrVal(0);
	std::wstring szCliPipe = reqParm->getList().GetWStrVal(1);
	int ipcID = cyjh::IPC_Manager::getInstance().MatchIpc(szSrvPipe.c_str(), szCliPipe.c_str());
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if ( item.get() )
	{
		item->m_ipcID = ipcID;
	}
	else{
		assert(false);
	}
	return true;
}

bool ResponseUI::rsp_getPrivateProfileString(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> reqParm, std::shared_ptr<cyjh::Instruct> out)
{
	std::wstring appName = reqParm->getList().GetWStrVal(0);
	std::wstring keyName = reqParm->getList().GetWStrVal(1);
	std::wstring defVal = reqParm->getList().GetWStrVal(2);
	std::wstring file = reqParm->getList().GetWStrVal(3);
	
	bool bret = false;
	if (_waccess_s(file.c_str(), 0) == 0)
	{
		struct _stat info;
		_wstat(file.c_str(), &info);
		int size = info.st_size;
		if (size > 0)
		{
			WCHAR* buf = new WCHAR[size + 1];
			GetPrivateProfileStringW(appName.c_str(), keyName.c_str(), defVal.c_str(), buf, size, file.c_str());
			out->getList().AppendVal(std::wstring(buf));
			delete []buf;
			bret = true;
		}
	}
	return bret;
}

bool ResponseUI::rsp_window_x(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		RECT rc;
		HWND hWnd = item->m_window->hwnd();
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.left);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_window_y(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		RECT rc;
		HWND hWnd = item->m_window->hwnd();
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.top);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_window_w(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		RECT rc;
		HWND hWnd = item->m_window->hwnd();
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.right - rc.left);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_window_h(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		RECT rc;
		HWND hWnd = item->m_window->hwnd();
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.bottom - rc.top);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_is_zoomed(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		out->getList().AppendVal(IsZoomed(hWnd) ? 1 : 0);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_is_iconic(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		out->getList().AppendVal(IsIconic(hWnd) ? 1 : 0);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_appname(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get())
	{
		out->getList().AppendVal(std::wstring(m_szName));
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_appDir(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get())
	{
		out->getList().AppendVal(std::wstring(m_szPath));
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_minWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_maxWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		PostMessage(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_restoreWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		PostMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_closeWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		PostMessage(hWnd, UI_CONTROL_MSG_CLOSE, 0, 0);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_setWindowText(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		std::wstring title = req_parm->getList().GetWStrVal(0);
		SetWindowText(hWnd, title.c_str());
		ret = true;
	}
	return ret;
}