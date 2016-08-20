#pragma once
#include <map>
#include <mutex>
#include <boost/shared_ptr.hpp>

struct InWndProcContext 
{
	WNDPROC m_proc;
	bool m_bClick;
	bool m_bSendDown;
	InWndProcContext()
	{
		m_proc = NULL;
		m_bClick = false;
		m_bSendDown = false;
	}
};
typedef std::map<HWND, boost::shared_ptr<InWndProcContext>> WNCPROC_MAP;
class SubClassWindow
{
public:	
	virtual ~SubClassWindow();
	static SubClassWindow& GetInst();
	void SubWindow(HWND, int);
	void UnSubWIndow(HWND);
	//boost::shared_ptr<InWndProcContext> findProc(HWND);
	BOOL IsInFitler(HWND);
	BOOL GetProcInfo(HWND hWnd, WNDPROC&, bool&, bool&);
	void SetClickVal(HWND hWnd, bool val, bool send);

	static LRESULT __stdcall myWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static SubClassWindow s_inst;
protected:
	SubClassWindow();
	WNCPROC_MAP m_defProcs;
	std::mutex m_MapMutex_;	
};

