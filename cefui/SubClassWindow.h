#pragma once
#include <map>
#include <mutex>

typedef std::map<HWND, WNDPROC> WNCPROC_MAP;
class SubClassWindow
{
public:	
	virtual ~SubClassWindow();
	static SubClassWindow& GetInst();
	void SubWindow(HWND, int);
	void UnSubWIndow(HWND);
	WNDPROC findProc(HWND);
	BOOL IsInFitler(HWND);

	static LRESULT __stdcall WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static SubClassWindow s_inst;
protected:
	SubClassWindow();
	WNCPROC_MAP m_defProcs;
	std::mutex m_MapMutex_;	
};

