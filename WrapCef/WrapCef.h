#ifndef _wrapcef_h
#define _wrapcef_h
#include <windows.h>
#include <string>
#include "defexport.h"


SHARED_EXPORT_API int InitCef(HINSTANCE hInstance, HACCEL hAccelTable);

SHARED_EXPORT_API int InitBrowser(HINSTANCE hInstance);

SHARED_EXPORT_API int UnInitBrowser();

namespace wrapQweb {

	typedef long(__stdcall *call_createWindow)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, unsigned long extra);

	typedef long(__stdcall *call_createModalWindow)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, unsigned long extra);

	typedef long(__stdcall *call_createModalWindow2)(HWND hWnd, long x, long y, long width, long height, long min_cx, long min_cy, long max_cx, long max_cy,
		std::wstring& skin, long alpha, unsigned long ulStyle, unsigned long extra, unsigned long parentSign);

	typedef std::wstring(__stdcall *call_invokeMethod)(HWND hWnd, std::wstring& modulename, std::wstring& methodname, std::wstring& parm, unsigned long extra);

	typedef std::wstring(__stdcall *call_crossInvokeWebMethod)(HWND hWnd, long winSign, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef std::wstring(__stdcall *call_crossInvokeWebMethod2)(HWND hWnd, long winSign, std::wstring& framename, std::wstring& modulename,
		std::wstring& methodname, std::wstring& parm, bool bNoticeJSTrans2JSON);

	typedef struct _FunMap{
		call_createWindow createWindow;
		call_createModalWindow createModalWindow;
		call_createModalWindow2 createModalWindow2;
		call_invokeMethod invokeMethod;
		call_crossInvokeWebMethod crossInvokeWebMethod;
		call_crossInvokeWebMethod2 crossInvokeWebMethod2;
	}FunMap;

	SHARED_EXPORT_API void InitQWeb(FunMap* map);

	SHARED_EXPORT_API HWND CreateWebView(int x, int y, int width, int height, const WCHAR* lpResource, int alpha, bool taskbar);

	SHARED_EXPORT_API bool QueryNodeAttrib(HWND hWnd, int x, int y, char* name, WCHAR** outVal, int& len);

	//操作脚本
	//
	SHARED_EXPORT_API bool callJSMethod(HWND hWnd, const char* fun_name, const char* utf8_parm,
		const char* utf8_frame_name = 0, std::wstring* outstr = 0);

	//软件标准的通信方法(模块，方法 ，参数，返回值， 框架名，前端使用的参数bNoticeJSTrans2JSON）
	SHARED_EXPORT_API bool invokedJSMethod(HWND hWnd, const char* utf8_module, const char* utf8_method,
		const char* utf8_parm, std::wstring* outstr,
		const char* utf8_frame_name = 0, bool bNoticeJSTrans2JSON = true);

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
