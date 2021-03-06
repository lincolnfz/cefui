#include "stdafx.h"
#include "ResponseUI.h"
#include "IPC.h"
#include "WebViewFactory.h"
#include <boost/functional/hash.hpp>
#include <shlobj.h> 
#include "json/json.h"
#include "NormalWebFactory.h"
#include "WebkitEcho.h"
//#include "HttpTrans.h"

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
	wcsncpy_s(m_szName, lch + 1, MAX_PATH);
	*lch = '\0';
	wcsncpy_s(m_szPath, szPath, MAX_PATH);

	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_RegisterBrowser); //注册浏览器 (渲染进程自已弹出的窗口放不放在框架中处理,如chromedev://)
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
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_asyncCrossInvokeWebMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_asyncCrossInvokeWebMethod2);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_fullScreen);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_appDataPath);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_asyncCallMethod);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_onDocLoaded);
	//REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_closeBrowser);
	//REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_siteicon);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_launchServerData);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_abortServerData);
}


ResponseUI::~ResponseUI()
{
}

void ResponseUI::SetFunMap(wrapQweb::FunMap* fnMap)
{
	s_fnMap = fnMap;
}

bool ResponseUI::rsp_RegisterBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> reqParm, std::shared_ptr<cyjh::Instruct> out )
{
	std::wstring szSrvPipe = reqParm->getList().GetWStrVal(0);
	std::wstring szCliPipe = reqParm->getList().GetWStrVal(1);
	int ipcID = cyjh::IPC_Manager::getInstance().MatchIpc(szSrvPipe.c_str(), szCliPipe.c_str());
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	//OutputDebugString(L"---- rsp_RegisterBrowser succ");
	if ( item.get() )
	{
		item->m_ipcID = ipcID;
		std::shared_ptr<cyjh::IPCUnit> unit = cyjh::IPC_Manager::getInstance().GetIpc(ipcID);
		if ( unit.get() )
		{
			unit->Attach();
		}
		out->getList().AppendVal((int)1);
	}
	else{
		//assert(false);
		//OutputDebugString(L"----------rsp_RegisterBrowser fail");
		CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
		if ( control.get() )
		{
			control->setIpcID(ipcID);
			std::shared_ptr<cyjh::IPCUnit> unit = cyjh::IPC_Manager::getInstance().GetIpc(ipcID);
			if (unit.get())
			{
				unit->Attach();
			}
			out->getList().AppendVal((int)2);
		}
		else{
			out->getList().AppendVal((int)0);
			assert(false);			
		}
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.left);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_window_y(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.top);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_window_w(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.right - rc.left);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_window_h(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		out->getList().AppendVal(rc.bottom - rc.top);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_is_zoomed(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		out->getList().AppendVal(IsZoomed(hWnd) ? 1 : 0);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_is_iconic(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_maxWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		PostMessage(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_restoreWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		PostMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_closeWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
	ret = !!WritePrivateProfileString(appName.c_str(), keyName.c_str(), val.c_str(), file.c_str());
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if ( IsWindow(hWnd) )
	{
		CefRefPtr<OSRWindow> window = WebViewFactory::getInstance().getWindowByHwnd(hWnd);
		if ( window.get() )
		{
			window->SetAlpha(alpha);
		}
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			out->getList().AppendVal(std::wstring(s_fnMap->winProty(hWnd)));
			ret = true;
		}
	}
	return ret;
}

bool parseCreateWindowParm(std::string& json, long& x, long& y, long& width, long& height, long& min_cx, long& min_cy, long& max_cx, long& max_cy,
	std::string& skin, long& alpha, unsigned long& ulStyle, unsigned long& extra, unsigned long& parentSign, bool& trans)
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
		trans = root.get("trans", true).asBool();
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign; bool trans;
			std::string parm = req_parm->getList().GetStrVal(0);
			if (parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign, trans))
			{
				s_fnMap->createWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, char2wchar(skin), alpha, ulStyle, trans, extra);
				ret = true;
			}			
		}
	}
	return ret;
}

bool ResponseUI::rsp_createModalWindow(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign; bool trans;
			std::string parm = req_parm->getList().GetStrVal(0);
			if (parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign, trans))
			{
				s_fnMap->createModalWindow(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, char2wchar(skin), alpha, ulStyle, trans, extra);
				ret = true;
			}
		}
	}
	return ret;
}

bool ResponseUI::rsp_createModalWindow2(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			long x; long y; long width; long height; long min_cx; long min_cy; long max_cx; long max_cy;
			std::string skin; long alpha; unsigned long ulStyle; unsigned long extra; unsigned long parentSign; bool trans;
			std::string parm = req_parm->getList().GetStrVal(0);
			if (parseCreateWindowParm(parm, x, y, width, height, min_cx, min_cy, max_cx, max_cy, skin, alpha, ulStyle, extra, parentSign, trans))
			{
				s_fnMap->createModalWindow2(hWnd, x, y, width, height, min_cx, min_cy, max_cx, max_cy, char2wchar(skin), alpha, ulStyle, trans, extra, parentSign);
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			out->getList().AppendVal( std::wstring(s_fnMap->invokeMethod(hWnd, module, method, parm, extra)));
			ret = true;
		}
	}
	else{
		CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
		if ( control.get() )
		{
			if (WebkitEcho::getFunMap()){
				out->getList().AppendVal( std::wstring(WebkitEcho::getFunMap()->webkitInvokeMethod(browser->GetIdentifier(), module, method, parm, extra)) );
				ret = true;
			}
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			out->getList().AppendVal(std::wstring(s_fnMap->crossInvokeWebMethod2(hWnd, sign, frame, module, method, parm, json)));
			ret = true;
		}
	}
	return ret;
}

bool ResponseUI::rsp_asyncCrossInvokeWebMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	int sign = req_parm->getList().GetIntVal(0);
	std::wstring module = req_parm->getList().GetWStrVal(1);
	std::wstring method = req_parm->getList().GetWStrVal(2);
	std::wstring parm = req_parm->getList().GetWStrVal(3);
	bool json = req_parm->getList().GetBooleanVal(4);
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			s_fnMap->asyncCrossInvokeWebMethod(hWnd, sign, module, method, parm, json);
			ret = true;
		}
	}
	return ret;
}

bool ResponseUI::rsp_asyncCrossInvokeWebMethod2(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	int sign = req_parm->getList().GetIntVal(0);
	std::wstring frame = req_parm->getList().GetWStrVal(1);
	std::wstring module = req_parm->getList().GetWStrVal(2);
	std::wstring method = req_parm->getList().GetWStrVal(3);
	std::wstring parm = req_parm->getList().GetWStrVal(4);
	bool json = req_parm->getList().GetBooleanVal(5);
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			s_fnMap->asyncCrossInvokeWebMethod2(hWnd, sign, frame, module, method, parm, json);
			ret = true;
		}
	}
	return ret;
}

bool ResponseUI::rsp_fullScreen(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	bool bFull = req_parm->getList().GetBooleanVal(0);
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
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
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_asyncCallMethod(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	std::wstring module = req_parm->getList().GetWStrVal(0);
	std::wstring method = req_parm->getList().GetWStrVal(1);
	std::wstring parm = req_parm->getList().GetWStrVal(2);
	int extra = req_parm->getList().GetIntVal(3);
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){
			s_fnMap->invokeMethod(hWnd, module, method, parm, extra);
			ret = true;
		}
	}
	else{
		CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
		if (control.get())
		{
			if (WebkitEcho::getFunMap()){
				WebkitEcho::getFunMap()->webkitAsyncCallMethod(browser->GetIdentifier(), module, method, parm, extra);
				ret = true;
			}
		}
	}
	return ret;
}

bool  ResponseUI::rsp_onDocLoaded(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out)
{
	bool ret = false;
	std::wstring strjs;
	std::wstring url = req_parm->getList().GetWStrVal(0);
	std::wstring frameName = req_parm->getList().GetWStrVal(1);
	std::wstring mainurl = req_parm->getList().GetWStrVal(2);
	int64  frameid = req_parm->getList().GetInt64Val(3);
	bool isMainFrame = req_parm->getList().GetBooleanVal(4);
	std::wstring icon_url = req_parm->getList().GetWStrVal(5);
	bool bAsync = req_parm->getAsync();
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		if (s_fnMap){		
#ifdef _DEBUG1
			//WCHAR szBuf[2048] = { 0 };
			//wsprintf(szBuf, L"-----[1 rsp_onDocLoaded name: %s url: %s", frameName.c_str(), url.c_str());
			//OutputDebugStringW(szBuf);
#endif
			const WCHAR* js = s_fnMap->injectJS(hWnd, url.c_str(), mainurl.c_str(), frameName.c_str());
			if (js && wcslen(js) > 0)
			{
				if ( !bAsync )
				{
					out->getList().AppendVal(std::wstring(js));
				}				
				strjs = js;				
			}
			ret = true;
		}
	}
	else{
		CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
		if (control.get())
		{
			if (WebkitEcho::getFunMap()){
				if ( isMainFrame )
				{
					WebkitEcho::getFunMap()->webkitSiteIcon(browser->GetIdentifier(), url.c_str(), icon_url.c_str());
				}
				WebkitEcho::getFunMap()->webkitDocLoaded(browser->GetIdentifier(), url.c_str(), frameName.c_str(), isMainFrame);
				const WCHAR* js = WebkitEcho::getFunMap()->webkitInjectJS(browser->GetIdentifier(), url.c_str(), mainurl.c_str(), frameName.c_str());
				if (js && wcslen(js) > 0)
				{
					if ( !bAsync )
					{
						out->getList().AppendVal(std::wstring(js));
					}
					strjs = js;
					
				}
				ret = true;
			}
		}
	}

	if ( ret && bAsync && !strjs.empty() )
	{
		cyjh::Instruct parm;
		parm.setName("injectJS");
		parm.getList().AppendVal(frameid);
		parm.getList().AppendVal(strjs);

		CefRefPtr<cyjh::UIThreadCombin> ipc = ClientApp::getGlobalApp()->getUIThreadCombin();
		ipc->AsyncRequest(browser, parm);
	}
	return ret;
}

bool ResponseUI::rsp_closeBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = true;
#ifdef _DEBUG1
	OutputDebugStringA("-----[ rsp_closeBrowser");
#endif
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		CefRefPtr<OSRWindow> window = WebViewFactory::getInstance().getWindowByHwnd(hWnd);
		if ( window.get() )
		{
			window->m_bPrepareClose = true;
		}
#ifdef _DEBUG1
		WCHAR szbuf[128] = { 0 };
		wsprintfW(szbuf, L"-----[ rsp_closeBrowser in ui, id : %d, hwnd: %d", browser->GetIdentifier(), hWnd);
		OutputDebugStringW(szbuf);
#endif
		PostMessage(hWnd, WM_CLOSE, NULL, NULL);
	}
	return ret;
}

bool ResponseUI::rsp_siteicon(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
	if (control.get())
	{
		if (WebkitEcho::getFunMap())
		{
			std::wstring icon_url = req_parm->getList().GetWStrVal(0);
			std::wstring url = req_parm->getList().GetWStrVal(1);
			WebkitEcho::getFunMap()->webkitSiteIcon(browser->GetIdentifier(), url.c_str(), icon_url.c_str());
		}
		ret = true;
	}
	return ret;
}

bool ResponseUI::rsp_launchServerData(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		std::string id = req_parm->getList().GetStrVal(0);
		std::string url = req_parm->getList().GetStrVal(1);
		std::string method = req_parm->getList().GetStrVal(2);
		std::string head = req_parm->getList().GetStrVal(3);
		std::string data = req_parm->getList().GetStrVal(4);
		bool bPost = _stricmp(method.c_str(), "post") == 0;
		//ret = HttpTrans::getInstance().sendData(browser->GetIdentifier(), id.c_str(),
		//	url.c_str(), "", data.c_str(), bPost, head.c_str());
	}
	return ret;
}

bool ResponseUI::rsp_abortServerData(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct>)
{
	bool ret = false;
	HWND hWnd = WebViewFactory::getInstance().GetBrowserHwndByID(browser->GetIdentifier());
	if (IsWindow(hWnd))
	{
		std::string id = req_parm->getList().GetStrVal(0);
		//HttpTrans::getInstance().abort(browser->GetIdentifier(), id.c_str());
		//ret = true;
	}
	return ret;
}