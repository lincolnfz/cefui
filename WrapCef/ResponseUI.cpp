#include "stdafx.h"
#include "ResponseUI.h"
#include "IPC.h"
#include "WebViewFactory.h"
#include <boost/functional/hash.hpp>
#include <shlobj.h> 
#include "json/json.h"

/*enum UI_CONTROL_MSG
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
};*/

wrapQweb::FunMap* ResponseUI::s_fnMap = NULL;
extern std::wstring g_strAppDataPath;

ResponseUI::ResponseUI()
{
	WCHAR szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	wchar_t* lch = wcsrchr(szPath, '\\');
	wcsncpy(m_szName, lch + 1, MAX_PATH);
	*lch = '\0';
	wcsncpy(m_szPath, szPath, MAX_PATH);

	//REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_RegisterBrowser); //注册浏览器放不放在框架中处理
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
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_writePrivateProfileString);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_getPrivateProfileInt);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_setProfile);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_getProfile);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_setWindowSize);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_setWindowPos);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_setAlpha);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_getSoftwareAttribute);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_winProty);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_createWindow);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_createModalWindow);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_createModalWindow2);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_invokeMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_crossInvokeWebMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_crossInvokeWebMethod2);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_fullScreen);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_appDataPath);
}


ResponseUI::~ResponseUI()
{
}

void ResponseUI::SetFunMap(wrapQweb::FunMap* fnMap)
{
	s_fnMap = fnMap;
}

bool ResponseUI::rsp_RegisterBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> reqParm, std::shared_ptr<cyjh::Instruct> )
{
	std::wstring szSrvPipe = reqParm->getList().GetWStrVal(0);
	std::wstring szCliPipe = reqParm->getList().GetWStrVal(1);
	int ipcID = cyjh::IPC_Manager::getInstance().MatchIpc(szSrvPipe.c_str(), szCliPipe.c_str());
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	//OutputDebugString(L"---- rsp_RegisterBrowser succ");
	if ( item.get() )
	{
		item->m_ipcID = ipcID;
	}
	else{
		assert(false);
		//OutputDebugString(L"----------rsp_RegisterBrowser fail");
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

bool ResponseUI::rsp_appDataPath(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get())
	{
		out->getList().AppendVal(g_strAppDataPath);
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
		//PostMessage(hWnd, UI_CONTROL_MSG_CLOSE, 0, 0);
		if ( s_fnMap )
		{
			s_fnMap->closeWindow(hWnd);
			ret = true;
		}		
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

static BOOL mirage_CreateDirectory(WCHAR* directory)
{
	BOOL bRet = TRUE;
	if (-1 == _waccess(directory, 0)) //判断目标文件夹是否存在
	{
		if (ERROR_SUCCESS != SHCreateDirectoryExW(NULL, directory, NULL))
		{
			return S_FALSE;
		}
	}
	return bRet;
}

bool ResponseUI::rsp_writePrivateProfileString(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	std::wstring appName = req_parm->getList().GetWStrVal(0);
	std::wstring keyName = req_parm->getList().GetWStrVal(1);
	std::wstring val = req_parm->getList().GetWStrVal(2);
	std::wstring file = req_parm->getList().GetWStrVal(3);

	WCHAR path[MAX_PATH];
	wcscpy_s(path, file.c_str());
	wchar_t* pos = wcsrchr(path, '\\');
	*pos = '\0';
	mirage_CreateDirectory(path);
	ret = WritePrivateProfileString(appName.c_str(), keyName.c_str(), val.c_str(), file.c_str());
	//assert(ret);
	return ret;
}

bool ResponseUI::rsp_getPrivateProfileInt(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = true;
	std::wstring appName = req_parm->getList().GetWStrVal(0);
	std::wstring keyName = req_parm->getList().GetWStrVal(1);
	int val = req_parm->getList().GetIntVal(2);
	std::wstring file = req_parm->getList().GetWStrVal(3);
	int outval = ::GetPrivateProfileInt(appName.c_str(), keyName.c_str(), val, file.c_str());
	out->getList().AppendVal(outval);
	return ret;
}

bool ResponseUI::rsp_setProfile(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = true;
	boost::hash<std::wstring> string_hash;
	std::wstring keyName = req_parm->getList().GetWStrVal(0);
	size_t key = string_hash(keyName);
	std::wstring val = req_parm->getList().GetWStrVal(1);
	std::map<unsigned int, std::wstring>::iterator it = m_profile.find(key);
	if ( it != m_profile.end() )
	{
		it->second = val;
	}
	else{
		std::pair<std::map<unsigned int, std::wstring>::iterator, bool> insret = m_profile.insert(std::make_pair(key, val));
		//ret = insret.second;
	}	
	//assert(ret);
	return ret;
}

bool ResponseUI::rsp_getProfile(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = true;
	boost::hash<std::wstring> string_hash;
	std::wstring keyName = req_parm->getList().GetWStrVal(0);
	size_t key = string_hash(keyName);
	std::map<unsigned int, std::wstring>::iterator it = m_profile.find(key);
	if ( it != m_profile.end() )
	{
		out->getList().AppendVal(it->second);
		//ret = true;
	}
	else{
		out->getList().AppendVal(L"");
	}
	//assert(ret);
	return ret;
}

bool ResponseUI::rsp_setWindowSize(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	int x = req_parm->getList().GetIntVal(0);
	int y = req_parm->getList().GetIntVal(1);
	int width = req_parm->getList().GetIntVal(2);
	int height = req_parm->getList().GetIntVal(3);
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		SetWindowPos(hWnd, HWND_NOTOPMOST, x, y, width, height, SWP_NOZORDER | SWP_FRAMECHANGED);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_setWindowPos(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	int order = req_parm->getList().GetIntVal(0);
	int x = req_parm->getList().GetIntVal(1);
	int y = req_parm->getList().GetIntVal(2);
	int width = req_parm->getList().GetIntVal(3);
	int height = req_parm->getList().GetIntVal(4);
	int flag = req_parm->getList().GetIntVal(5);
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if ( s_fnMap )
		{
			s_fnMap->setWindowPos(hWnd, order, x, y, width, height, flag);
		}
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_setAlpha(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	int alpha = req_parm->getList().GetIntVal(0);
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		item->m_window->SetAlpha(alpha);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_getSoftwareAttribute(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	int attidx = req_parm->getList().GetIntVal(0);
	if ( s_fnMap )
	{
		out->getList().AppendVal(std::wstring(s_fnMap->softAttr(attidx)));
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_winProty(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			out->getList().AppendVal(std::wstring(s_fnMap->winProty(hWnd)));
			ret = true;
		}
	}
	return ret;
}

bool parseCreateWindowParm(std::string& json, long& x, long& y, long& width, long& height, long& min_cx, long& min_cy, long& max_cx, long& max_cy,
	std::string& skin, long& alpha, unsigned long& ulStyle, unsigned long& extra, unsigned long& parentSign)
{
	bool ret = false;
	Json::Reader read;
	Json::Value root;
	if (read.parse(json, root)){
		x = root.get("x", 0).asInt();
		y = root.get("y", 0).asInt();
		width = root.get("width", 0).asInt();
		height = root.get("height", 0).asInt();
		min_cx = root.get("min_cx", 0).asInt();
		min_cy = root.get("min_cy", 0).asInt();
		max_cx = root.get("max_cx", 0).asInt();
		max_cy = root.get("max_cy", 0).asInt();
		skin = root.get("skin", 0).asString();
		alpha = root.get("alpha", 0).asInt();
		ulStyle = root.get("ulStyle", 0).asUInt();
		extra = root.get("extra", 0).asUInt();
		parentSign = root.get("parentSign", 0).asUInt();
		ret = true;
	}

	return ret;
}

std::wstring char2wchar(const std::string& str)
{
	int iTextLen = 0;
	iTextLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	WCHAR* wsz = new WCHAR[iTextLen + 1];
	memset((void*)wsz, 0, sizeof(WCHAR) * (iTextLen + 1));
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wsz, iTextLen);
	std::wstring strText(wsz);
	delete[]wsz;
	return strText;
}

bool ResponseUI::rsp_createWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;
			std::string parm = req_parm->getList().GetStrVal(0);
			if (parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
			{
				s_fnMap->createWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, char2wchar(skin), alpha, ulStyle, extra);
				ret = true;
			}			
		}
	}
	return ret;
}

bool ResponseUI::rsp_createModalWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;
			std::string parm = req_parm->getList().GetStrVal(0);
			if (parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
			{
				s_fnMap->createModalWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, char2wchar(skin), alpha, ulStyle, extra);
				ret = true;
			}
		}
	}
	return ret;
}

bool ResponseUI::rsp_createModalWindow2(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign;
			std::string parm = req_parm->getList().GetStrVal(0);
			if (parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign))
			{
				s_fnMap->createModalWindow2(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, char2wchar(skin), alpha, ulStyle, extra, parentSign);
				ret = true;
			}
		}
	}
	return ret;
}

bool ResponseUI::rsp_invokeMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	std::wstring module = req_parm->getList().GetWStrVal(0);
	std::wstring method = req_parm->getList().GetWStrVal(1);
	std::wstring parm = req_parm->getList().GetWStrVal(2);
	int extra = req_parm->getList().GetIntVal(3);
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			out->getList().AppendVal( std::wstring(s_fnMap->invokeMethod(hWnd, module, method, parm, extra)));
			ret = true;
		}
	}
	return ret;
}

bool ResponseUI::rsp_crossInvokeWebMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	int sign = req_parm->getList().GetIntVal(0);
	std::wstring module = req_parm->getList().GetWStrVal(1);
	std::wstring method = req_parm->getList().GetWStrVal(2);
	std::wstring parm = req_parm->getList().GetWStrVal(3);
	bool json = req_parm->getList().GetBooleanVal(4);
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			out->getList().AppendVal(std::wstring(s_fnMap->crossInvokeWebMethod(hWnd, sign,  module, method, parm, json)));
			ret = true;
		}
	}
	return ret;
}

bool ResponseUI::rsp_crossInvokeWebMethod2(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	int sign = req_parm->getList().GetIntVal(0);
	std::wstring frame = req_parm->getList().GetWStrVal(1);
	std::wstring module = req_parm->getList().GetWStrVal(2);
	std::wstring method = req_parm->getList().GetWStrVal(3);
	std::wstring parm = req_parm->getList().GetWStrVal(4);
	bool json = req_parm->getList().GetBooleanVal(5);
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		if (s_fnMap){
			out->getList().AppendVal(std::wstring(s_fnMap->crossInvokeWebMethod2(hWnd, sign, frame, module, method, parm, json)));
			ret = true;
		}
	}
	return ret;
}

bool ResponseUI::rsp_fullScreen(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	bool bFull = req_parm->getList().GetBooleanVal(0);
	if (item.get() && IsWindow(item->m_window->hwnd()))
	{
		HWND hWnd = item->m_window->hwnd();
		LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
		if (bFull)
		{
			style &= ~(WS_DLGFRAME | WS_THICKFRAME);
			SetWindowLong(hWnd, GWL_STYLE, style);
			ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			::SetWindowPos(hWnd, HWND_NOTOPMOST, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_FRAMECHANGED);
		}
		else{
			style |= WS_DLGFRAME | WS_THICKFRAME;
			SetWindowLong(hWnd, GWL_STYLE, style);
			ShowWindow(hWnd, SW_NORMAL);
		}
	}
	return ret;
}