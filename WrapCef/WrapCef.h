#ifndef _wrapcef_h
#define _wrapcef_h
#include <windows.h>
#include "defexport.h"


MIRAGE_API int InitCef(HINSTANCE hInstance, HACCEL hAccelTable);

MIRAGE_API int InitBrowser(HINSTANCE hInstance);

MIRAGE_API int UnInitBrowser();


class CChromeiumBrowserControl;

class MIRAGE_CLASS CBrowserControl
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
