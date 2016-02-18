#ifndef _wrapcef_h
#define _wrapcef_h
#include <windows.h>
#include <string>
#include "cefexprot.h"
//#include <atlstr.h>


SHARED_EXPORT_API int InitCef(HINSTANCE hInstance, HACCEL hAccelTable);

SHARED_EXPORT_API int InitBrowser(HINSTANCE hInstance);

SHARED_EXPORT_API int UnInitBrowser();

namespace wrapQweb {

	typedef long(__stdcall *call_closeWindow)(HWND hWnd);

	typedef long(__stdcall *call_setWindowPos)(HWND hWnd, long order, long x, long y, long cx, long cy , long flag);

	typedef long(__stdcall *call_createWindow)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, unsigned long extra);

	typedef long(__stdcall *call_createModalWindow)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, unsigned long extra);

	typedef long(__stdcall *call_createModalWindow2)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, unsigned long extra, unsigned long parentSign);

	typedef const WCHAR*(__stdcall *call_invokeMethod)(HWND hWnd, std::wstring& modulename, std::wstring& methodname, std::wstring& parm, unsigned long extra);

	typedef const WCHAR*(__stdcall *call_crossInvokeWebMethod)(HWND hWnd, long winSign, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef const WCHAR*(__stdcall *call_crossInvokeWebMethod2)(HWND hWnd, long winSign, std::wstring& framename, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef const WCHAR*(__stdcall *call_winProty)(HWND hWnd);

	typedef const WCHAR*(__stdcall *call_softwareAttribute)(unsigned long);

	typedef void (__stdcall *call_NativeComplate)(const HWND&);

	typedef void(__stdcall *call_NativeFrameComplate)(const HWND&, const WCHAR* url, const WCHAR* frameName);

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
		call_newNativeUrl newNativeUrl;
	}FunMap;

	SHARED_EXPORT_API int InitLibrary(HINSTANCE hInstance, WCHAR* lpRender = NULL);

	SHARED_EXPORT_API void FreeLibrary();

	SHARED_EXPORT_API void InitQWeb(FunMap* map);

	SHARED_EXPORT_API void RunLoop();

	SHARED_EXPORT_API void QuitLoop();

	SHARED_EXPORT_API void CloseWebview(const HWND&);

	SHARED_EXPORT_API void CloseAllWebView();

	SHARED_EXPORT_API HWND CreateWebView(const int& x, const int& y, const int& width, const int& height, const WCHAR* lpResource, const int& alpha, const bool& taskbar);

	SHARED_EXPORT_API bool QueryNodeAttrib(const HWND&, const int& x, const int& y, char* name, WCHAR* outVal, const int& len);

	SHARED_EXPORT_API void SetFouceWebView(const HWND& hWnd, const bool& fouce);

	//操作脚本
	//
	SHARED_EXPORT_API bool callJSMethod(const HWND&, const char* fun_name, const char* utf8_parm,
		const char* utf8_frame_name = 0, HGLOBAL* outstr = 0);

	//软件标准的通信方法(模块，方法 ，参数，返回值， 框架名，前端使用的参数bNoticeJSTrans2JSON）
	SHARED_EXPORT_API bool invokedJSMethod(const HWND&, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm, HGLOBAL* outstr,
		const char* utf8_frame_name = 0, bool bNoticeJSTrans2JSON = true);	

	SHARED_EXPORT_API bool freeMem(HGLOBAL hMem);

}


class CChromeiumBrowserControl;

class SHARED_EXPORT_CLASS CBrowserControl
{
public:
	CBrowserControl();
	virtual ~CBrowserControl();
	void AttachHwnd(HWND, const WCHAR*);
	void handle_size(HWND);
	void handle_SetForce();

private:
	 CChromeiumBrowserControl* m_browser;
};


#endif
