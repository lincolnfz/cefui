#ifndef _wrapcef_h
#define _wrapcef_h
#include <windows.h>
#include <string>
#include "cefexprot.h"
#include <atlstr.h>
#include <vector>


SHARED_EXPORT_API int InitCef(HINSTANCE hInstance, HACCEL hAccelTable);

SHARED_EXPORT_API int InitBrowser(HINSTANCE hInstance);

SHARED_EXPORT_API int UnInitBrowser();

struct WRAP_CEF_MENU_COMMAND
{
	WCHAR szTxt[256];
	int command;
	bool bEnable;
	bool top;
};

namespace wrapQweb {

	#define REMOVE_DATA_MASK_APPCACHE  1 << 0
	#define REMOVE_DATA_MASK_COOKIES 1 << 1
	#define REMOVE_DATA_MASK_FILE_SYSTEMS 1 << 2
	#define REMOVE_DATA_MASK_INDEXEDDB  1 << 3
	#define REMOVE_DATA_MASK_LOCAL_STORAGE 1 << 4
	#define REMOVE_DATA_MASK_SHADER_CACHE 1 << 5
	#define REMOVE_DATA_MASK_WEBSQL 1 << 6
	#define REMOVE_DATA_MASK_WEBRTC_IDENTITY 1 << 7
	#define REMOVE_DATA_MASK_SERVICE_WORKERS 1 << 8
	#define REMOVE_DATA_MASK_ALL  0xFFFFFFFF


//创建窗口时默认大小
#define WIDGET_NORMAL_SIZE 1 << 0

//创建窗口时最小化
#define WIDGET_MIN_SIZE 1 << 1

//创建窗口时最大化
#define WIDGET_MAX_SIZE 1 << 2

	//允许拖拽
#define ENABLE_DRAG_DROP 1 << 4

	typedef long(__stdcall *call_closeWindow)(HWND hWnd);

	typedef long(__stdcall *call_setWindowPos)(HWND hWnd, long order, long x, long y, long cx, long cy , long flag);

	typedef long(__stdcall *call_createWindow)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, bool bTrans, unsigned long extra);

	typedef long(__stdcall *call_createModalWindow)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, bool bTrans, unsigned long extra);

	typedef long(__stdcall *call_createModalWindow2)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, bool bTrans, unsigned long extra, unsigned long parentSign);

	typedef const WCHAR*(__stdcall *call_invokeMethod)(HWND hWnd, std::wstring& modulename, std::wstring& methodname, std::wstring& parm, unsigned long extra);

	typedef const WCHAR*(__stdcall *call_crossInvokeWebMethod)(HWND hWnd, long winSign, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef const WCHAR*(__stdcall *call_crossInvokeWebMethod2)(HWND hWnd, long winSign, std::wstring& framename, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef void(__stdcall *call_asyncCrossInvokeWebMethod)(HWND hWnd, long winSign, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef void(__stdcall *call_asyncCrossInvokeWebMethod2)(HWND hWnd, long winSign, std::wstring& framename, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef const WCHAR*(__stdcall *call_winProty)(HWND hWnd);

	typedef const WCHAR*(__stdcall *call_softwareAttribute)(unsigned long);

	typedef void (__stdcall *call_NativeComplate)(const HWND&);

	typedef void(__stdcall *call_NativeFrameComplate)(const HWND&, const WCHAR* url, const WCHAR* frameName);

	typedef void(__stdcall *call_NativeFrameBegin)(const HWND&, const WCHAR* url, const WCHAR* frameName);

	typedef void(__stdcall *call_newNativeUrl)(const HWND&, const WCHAR* url, const WCHAR* frameName);

	typedef bool(__stdcall *call_doMenuCommand)(const HWND&, const int& id);

	typedef const WCHAR*(__stdcall *call_InjectJS)(const HWND&, const WCHAR* url, const WCHAR* mainurl, const WCHAR* frameName);

	typedef void(__stdcall *call_InertMenu)(const HWND&, const WCHAR* attribName, WRAP_CEF_MENU_COMMAND[]);

	typedef void(__stdcall *call_LoadError)(const HWND&, const int& errcode, const WCHAR* frameName, const WCHAR* url);

	typedef void(__stdcall *call_onDocLoaded)(const HWND&, const WCHAR* url, const WCHAR* frameName, const bool& bMainFrame);

	typedef struct _FunMap{
		call_closeWindow closeWindow;
		call_setWindowPos setWindowPos;
		call_createWindow createWindow;
		call_createModalWindow createModalWindow;
		call_createModalWindow2 createModalWindow2;		
		call_invokeMethod invokeMethod;
		call_crossInvokeWebMethod crossInvokeWebMethod;
		call_crossInvokeWebMethod2 crossInvokeWebMethod2;
		call_asyncCrossInvokeWebMethod asyncCrossInvokeWebMethod;
		call_asyncCrossInvokeWebMethod2 asyncCrossInvokeWebMethod2;
		call_winProty winProty;
		call_softwareAttribute softAttr;
		call_NativeComplate nativeComplate;
		call_NativeFrameComplate nativeFrameComplate;
		//call_NativeFrameBegin nativeFrameBegin;
		call_newNativeUrl newNativeUrl;
		call_doMenuCommand doMenuCommand;
		call_InjectJS injectJS;
		call_InertMenu insertMenu;
		call_LoadError loadError;
	}FunMap;

	SHARED_EXPORT_API void SetUserAgent(WCHAR* ua);

	SHARED_EXPORT_API int InitLibrary(HINSTANCE hInstance, WCHAR* lpRender = NULL, WCHAR* szLocal = L"zh-CN", bool bShareNPPlugin = true);

	SHARED_EXPORT_API void FreeLibrary();

	SHARED_EXPORT_API void InitQWeb(FunMap* map);

	SHARED_EXPORT_API void RunLoop();

	SHARED_EXPORT_API void QuitLoop();

	SHARED_EXPORT_API void CloseWebview(const HWND&);

	SHARED_EXPORT_API void CloseAllWebView();

	SHARED_EXPORT_API HWND CreateWebView(const int& x, const int& y, const int& width, const int& height, const WCHAR* lpResource, const int& alpha, const bool& taskbar, const bool& trans, const int& winCombination);

	SHARED_EXPORT_API HWND CreateInheritWebView(const HWND&, const int& x, const int& y, const int& width, const int& height, const WCHAR* lpResource, const int& alpha, const bool& taskbar, const bool& trans, const int& winCombination);

	SHARED_EXPORT_API bool QueryNodeAttrib(const HWND&, const int& x, const int& y, char* name, WCHAR* outVal, const int& len);

	SHARED_EXPORT_API void SetFouceWebView(const HWND& hWnd, const bool& fouce);

	//操作脚本
	//
	SHARED_EXPORT_API bool callJSMethod(const HWND&, const char* fun_name, const char* utf8_parm,
		const char* utf8_frame_name = 0, CStringW* outstr = 0);

	//软件同步通信方法(模块，方法 ，参数，返回值， 框架名，前端使用的参数bNoticeJSTrans2JSON）
	SHARED_EXPORT_API bool invokedJSMethod(const HWND&, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm, CStringW* outstr,
		const char* utf8_frame_name = 0, bool bNoticeJSTrans2JSON = true);

	//软件异步通信方法(模块，方法 ，参数，返回值， 框架名，前端使用的参数bNoticeJSTrans2JSON）
	SHARED_EXPORT_API bool asyncInvokedJSMethod(const HWND&, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm,
		const char* utf8_frame_name = 0, bool bNoticeJSTrans2JSON = true);

	//这个函数废弃
	SHARED_EXPORT_API bool freeMem(HGLOBAL hMem);

	//注册浏览器插件
	//SHARED_EXPORT_API bool RegPlugin(const HWND& hWnd, const WCHAR* szVal, const bool bPPapi, const bool bSandBox);

	//改变dns解析
	//SHARED_EXPORT_API void setResolveHost(const char* host, const char* ip);

	//清理解析
	SHARED_EXPORT_API void clearResolveHost();

	SHARED_EXPORT_API bool QueryRenderProcessID(const HWND&, int& pid);

	//
	SHARED_EXPORT_API bool QueryPluginsProcessID(const HWND&, std::vector<DWORD>& plugins_process_ids);

	SHARED_EXPORT_API bool GetViewZoomLevel(const HWND&, double& level);

	SHARED_EXPORT_API bool SetViewZoomLevel(const HWND&, const double& level);

//---------------------------------------------------------------------------------------------------------------------
//类分割线

	typedef void(__stdcall *call_WebkitAfterCreate)(const HWND&, const HWND&, const HWND&, const int& id);

	typedef void(__stdcall *call_WebkitOpenNewUrl)(const int& id, const WCHAR* url, const WCHAR* cookie_ctx);

	typedef void(__stdcall *call_WebkitLoadingStateChange)(const int& id, const bool& loading, const bool& canBack, const bool& canForward);

	typedef void(__stdcall *call_WebkitChangeUrl)(const int& id, const WCHAR* url);

	typedef void(__stdcall *call_WebkitChangeTitle)(const int& id, const WCHAR* title);

	typedef void(__stdcall *call_WebkitBeginLoad)(const int& id, const WCHAR* url, bool* cancel);

	typedef void(__stdcall *call_WebkitEndLoad)(const int& id);

	typedef const WCHAR*(__stdcall *call_WebkitInvokeMethod)(const int& id, std::wstring& modulename, std::wstring& methodname, std::wstring& parm, unsigned long extra);

	typedef const WCHAR*(__stdcall *call_WebkitInjectJS)(const int& id, const WCHAR* url, const WCHAR* mainurl, const WCHAR* title);

	typedef void(__stdcall *call_WebkitDownFileUrl)(const int& id, const WCHAR* url, const WCHAR* suggestFileName);

	typedef void(__stdcall *call_WebkitAsyncCallMethod)(const int& id, std::wstring& modulename, std::wstring& methodname, std::wstring& parm, unsigned long extra);

	typedef void(__stdcall *call_WebkitPluginCrash)(const int& id, const WCHAR* path);

	typedef void(__stdcall *call_WebkitBeforeClose)(const int& id);

	typedef void(__stdcall *call_WebkitDocLoaded)(const int& id, const WCHAR* url, const WCHAR* frameName, const bool& bMainFrame);

	typedef void(__stdcall *call_WebkitSiteIcon)(const int& id, const WCHAR* main_url, const WCHAR* icon_url);

	typedef bool(__stdcall *call_WebkitNewTab)(const int& id, const WCHAR* main_url, HWND* parent);

	typedef struct _EchoMap{
		call_WebkitAfterCreate webkitAfterCreate;
		call_WebkitOpenNewUrl webkitOpenNewUrl;
		call_WebkitLoadingStateChange webkitLoadingStateChange;
		call_WebkitChangeUrl webkitChangeUrl;
		call_WebkitChangeTitle webkitChangeTitle;
		call_WebkitBeginLoad webkitBeginLoad;
		call_WebkitEndLoad webkitEndLoad;
		call_WebkitInvokeMethod webkitInvokeMethod;
		call_WebkitInjectJS webkitInjectJS;
		call_WebkitDownFileUrl webkitDownFileUrl;
		call_WebkitAsyncCallMethod webkitAsyncCallMethod;
		call_WebkitPluginCrash webkitPluginCrash;
		call_WebkitBeforeClose webkitBeforeClose;
		call_WebkitDocLoaded webkitDocLoaded;
		call_WebkitSiteIcon webkitSiteIcon;
		call_WebkitNewTab webkitNewTab;
	}EchoMap;

	//初始化浏览器控件响应函数
	SHARED_EXPORT_API void InitEchoFn(EchoMap* map);

	SHARED_EXPORT_API void CreateWebControl(const HWND& hwnd, const WCHAR* url, const WCHAR* cookie = NULL);

	SHARED_EXPORT_API bool CloseWebControl(const HWND& hwnd);

	SHARED_EXPORT_API bool LoadUrl(const HWND& hwnd, const WCHAR* url);

	SHARED_EXPORT_API bool GoBack(const HWND& hwnd);

	SHARED_EXPORT_API bool GoForward(const HWND& hwnd);

	SHARED_EXPORT_API bool Reload(const HWND& hwnd);

	SHARED_EXPORT_API bool ReloadIgnoreCache(const HWND& hwnd);

	SHARED_EXPORT_API bool IsAudioMuted(const HWND& hwnd);

	SHARED_EXPORT_API void SetAudioMuted(const HWND& hwnd, const bool& bEnable);

	SHARED_EXPORT_API bool Stop(const HWND& hwnd);

	//SHARED_EXPORT_API bool asyncInvokedJSMethod(const HWND& hWnd, const char* utf8_module, const char* utf8_method,
	//	const char* utf8_parm,
	//	const char* utf8_frame_name, bool bNoticeJSTrans2JSON = true);

	SHARED_EXPORT_API void AdjustRenderSpeed(const HWND& hWnd, const double& dbSpeed);

	SHARED_EXPORT_API void ClearBrowserData(int combType);

	SHARED_EXPORT_API void SendMouseClickEvent(const HWND& hWnd, const unsigned int& msg, const long& wp, const long& lp);

	//注入脚本
	SHARED_EXPORT_API void InjectJS(const HWND& hwnd, const WCHAR* js);

	class CChromeiumBrowserControl;

	class SHARED_EXPORT_CLASS CWebkitControl
	{
	public:
		CWebkitControl();
		virtual ~CWebkitControl();
		HWND AttachHwnd(HWND, const WCHAR*);
		void handle_size(HWND);
		void handle_SetForce();

	private:
		CChromeiumBrowserControl* m_browser;
	};

}

namespace cefControl{


}

#endif
