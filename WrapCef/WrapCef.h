#ifndef _wrapcef_h
#define _wrapcef_h
#include <windows.h>
#include <string>
#include "cefexprot.h"
#include <atlstr.h>


SHARED_EXPORT_API int InitCef(HINSTANCE hInstance, HACCEL hAccelTable);

SHARED_EXPORT_API int InitBrowser(HINSTANCE hInstance);

SHARED_EXPORT_API int UnInitBrowser();

namespace wrapQweb {

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

	typedef const WCHAR*(__stdcall *call_winProty)(HWND hWnd);

	typedef const WCHAR*(__stdcall *call_softwareAttribute)(unsigned long);

	typedef void (__stdcall *call_NativeComplate)(const HWND&);

	typedef void(__stdcall *call_NativeFrameComplate)(const HWND&, const WCHAR* url, const WCHAR* frameName);

	typedef void(__stdcall *call_NativeFrameBegin)(const HWND&, const WCHAR* url, const WCHAR* frameName);

	typedef void(__stdcall *call_newNativeUrl)(const HWND&, const WCHAR* url, const WCHAR* frameName);

	typedef struct _FunMap{
		call_closeWindow closeWindow;
		call_setWindowPos setWindowPos;
		call_createWindow createWindow;
		call_createModalWindow createModalWindow;
		call_createModalWindow2 createModalWindow2;
		call_invokeMethod invokeMethod;
		call_crossInvokeWebMethod crossInvokeWebMethod;
		call_crossInvokeWebMethod2 crossInvokeWebMethod2;
		call_winProty winProty;
		call_softwareAttribute softAttr;
		call_NativeComplate nativeComplate;
		call_NativeFrameComplate nativeFrameComplate;
		//call_NativeFrameBegin nativeFrameBegin;
		call_newNativeUrl newNativeUrl;
	}FunMap;

	SHARED_EXPORT_API int InitLibrary(HINSTANCE hInstance, WCHAR* lpRender = NULL);

	SHARED_EXPORT_API void FreeLibrary();

	SHARED_EXPORT_API void InitQWeb(FunMap* map);

	SHARED_EXPORT_API void RunLoop();

	SHARED_EXPORT_API void QuitLoop();

	SHARED_EXPORT_API void CloseWebview(const HWND&);

	SHARED_EXPORT_API void CloseAllWebView();

	SHARED_EXPORT_API HWND CreateWebView(const int& x, const int& y, const int& width, const int& height, const WCHAR* lpResource, const int& alpha, const bool& taskbar, const bool& trans);

	SHARED_EXPORT_API bool QueryNodeAttrib(const HWND&, const int& x, const int& y, char* name, WCHAR* outVal, const int& len);

	SHARED_EXPORT_API void SetFouceWebView(const HWND& hWnd, const bool& fouce);

	//操作脚本
	//
	SHARED_EXPORT_API bool callJSMethod(const HWND&, const char* fun_name, const char* utf8_parm,
		const char* utf8_frame_name = 0, CStringW* outstr = 0);

	//软件标准的通信方法(模块，方法 ，参数，返回值， 框架名，前端使用的参数bNoticeJSTrans2JSON）
	SHARED_EXPORT_API bool invokedJSMethod(const HWND&, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm, CStringW* outstr,
		const char* utf8_frame_name = 0, bool bNoticeJSTrans2JSON = true);	

	//这个函数废弃
	SHARED_EXPORT_API bool freeMem(HGLOBAL hMem);

	//注册浏览器插件
	SHARED_EXPORT_API bool RegPlugin(const HWND& hWnd, const WCHAR* szVal, const bool bPPapi, const bool bSandBox);

//---------------------------------------------------------------------------------------------------------------------
//类分割线

	typedef void(__stdcall *call_WebkitAfterCreate)(const HWND&, const HWND&, const int& id);

	typedef void(__stdcall *call_WebkitOpenNewUrl)(const int& id, const WCHAR* url);

	typedef void(__stdcall *call_WebkitLoadingStateChange)(const int& id, const bool& loading, const bool& canBack, const bool& canForward);

	typedef void(__stdcall *call_WebkitChangeUrl)(const int& id, const WCHAR* url);

	typedef void(__stdcall *call_WebkitChangeTitle)(const int& id, const WCHAR* title);

	typedef void(__stdcall *call_WebkitBeginLoad)(const int& id);

	typedef void(__stdcall *call_WebkitEndLoad)(const int& id);

	typedef const WCHAR*(__stdcall *call_WebkitInvokeMethod)(const int& id, std::wstring& modulename, std::wstring& methodname, std::wstring& parm, unsigned long extra);

	typedef struct _EchoMap{
		call_WebkitAfterCreate webkitAfterCreate;
		call_WebkitOpenNewUrl webkitOpenNewUrl;
		call_WebkitLoadingStateChange webkitLoadingStateChange;
		call_WebkitChangeUrl webkitChangeUrl;
		call_WebkitChangeTitle webkitChangeTitle;
		call_WebkitBeginLoad webkitBeginLoad;
		call_WebkitEndLoad webkitEndLoad;
		call_WebkitInvokeMethod webkitInvokeMethod;
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
