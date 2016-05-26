#include "stdafx.h"
#include "TipWin.h"
#include <windef.h>

#pragma comment(lib , "Comctl32.lib")

TipWin::TipWin()
{
	tooltip_hwnd_ = NULL;
	showing_ = false;
	parent_hwnd_ = NULL;
	memset(&toolinfo_, 0, sizeof(toolinfo_));
	toolinfo_.cbSize = sizeof(toolinfo_);
	toolinfo_.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	toolinfo_.uId = reinterpret_cast<UINT_PTR>(parent_hwnd_);
	toolinfo_.hwnd = parent_hwnd_;
	toolinfo_.lpszText = NULL;
	toolinfo_.lpReserved = NULL;
	SetRectEmpty(&toolinfo_.rect);
}


TipWin::~TipWin()
{
	//if (tooltip_hwnd_)
	//	DestroyWindow(tooltip_hwnd_);
	DestroyTipWin();
}

void TipWin::SetParentHwnd(HWND hWnd)
{
	parent_hwnd_ = hWnd;
	toolinfo_.uId = reinterpret_cast<UINT_PTR>(parent_hwnd_);
	toolinfo_.hwnd = parent_hwnd_;
}

bool TipWin::HandleNotify(int w_param, NMHDR* l_param, LRESULT* l_result) {
	if (tooltip_hwnd_ == NULL)
		return false;

	switch (l_param->code) {
	case TTN_POP:
		showing_ = false;
		return true;
	case TTN_SHOW:
		*l_result = TRUE;
		PositionTooltip();
		showing_ = true;
		return true;
	default:
		break;
	}
	return false;
}

bool TipWin::EnsureTooltipWindow() {
	if (tooltip_hwnd_)
		return true;

	tooltip_hwnd_ = CreateWindowEx(
		WS_EX_TRANSPARENT /*| WS_EX_LAYOUTRTL*/,
		TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | WS_POPUP, 0, 0, 0, 0,
		parent_hwnd_, NULL, NULL, NULL);
	if (!tooltip_hwnd_) {
		//PLOG(WARNING) << "tooltip creation failed, disabling tooltips";
		return false;
	}

	//l10n_util::AdjustUIFontForWindow(tooltip_hwnd_); //ÉèÖÃ×ÖÌå

	SendMessage(tooltip_hwnd_, TTM_ADDTOOL, 0,
		reinterpret_cast<LPARAM>(&toolinfo_));
	return true;
}

RECT getNearWork_area(POINT initial_loc){
	RECT rcWork;
	SetRectEmpty(&rcWork);
	HMONITOR monitor = MonitorFromPoint(initial_loc, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX mi;
	ZeroMemory(&mi, sizeof(MONITORINFOEX));
	mi.cbSize = sizeof(mi);
	if (monitor && GetMonitorInfo(monitor, &mi)){
		rcWork = mi.rcWork;
	}
	return rcWork;
}

void TipWin::PositionTooltip() {
	// This code only runs for non-metro, so GetNativeScreen() is fine.
	POINT screen_point = location_;
	ClientToScreen(parent_hwnd_, &screen_point);
	const int cursoroffset = 16;
	screen_point.y += cursoroffset;


	DWORD tooltip_size = SendMessage(tooltip_hwnd_, TTM_GETBUBBLESIZE, 0,
		reinterpret_cast<LPARAM>(&toolinfo_));
	
	SIZE size;
	size.cx = LOWORD(tooltip_size);
	size.cy = HIWORD(tooltip_size);

	//const gfx::Display display(
	//	gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(screen_point));

	RECT tooltip_bounds;
	tooltip_bounds.left = screen_point.x;
	tooltip_bounds.top = screen_point.y;
	tooltip_bounds.right = screen_point.x + size.cx;
	tooltip_bounds.bottom = screen_point.y + size.cy;
	//tooltip_bounds.AdjustToFit(gfx::win::DIPToScreenRect(display.work_area()));
	SetWindowPos(tooltip_hwnd_, NULL, tooltip_bounds.left, tooltip_bounds.top, 0,
		0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void TipWin::SetText(const std::wstring& tooltip_text,
	const POINT& location) {
	if (!EnsureTooltipWindow())
		return;

	// See comment in header for details on why |location_| is needed.
	location_ = location;

	// Without this we get a flicker of the tooltip appearing at 0x0. Not sure
	// why.
	SetWindowPos(tooltip_hwnd_, NULL, 0, 0, 0, 0,
		SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOMOVE |
		SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);

	toolinfo_.lpszText = const_cast<WCHAR*>(tooltip_text.c_str());
	SendMessage(tooltip_hwnd_, TTM_SETTOOLINFO, 0,
		reinterpret_cast<LPARAM>(&toolinfo_));

	// This code only runs for non-metro, so GetNativeScreen() is fine.
	POINT screen_point = location_;
	ClientToScreen(parent_hwnd_, &screen_point);
	RECT rcWork = getNearWork_area(screen_point);
	int max_width = (rcWork.right - rcWork.left + 1) / 2;
	SendMessage(tooltip_hwnd_, TTM_SETMAXTIPWIDTH, 0, max_width);
}

void TipWin::Show() {
	if (!EnsureTooltipWindow())
		return;

	SendMessage(tooltip_hwnd_, TTM_TRACKACTIVATE,
		TRUE, reinterpret_cast<LPARAM>(&toolinfo_));
	SetWindowPos(tooltip_hwnd_, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE);
}

void TipWin::Hide() {
	if (!tooltip_hwnd_)
		return;

	SendMessage(tooltip_hwnd_, TTM_TRACKACTIVATE, FALSE,
		reinterpret_cast<LPARAM>(&toolinfo_));
}

bool TipWin::IsVisible() {
	return showing_;
}


static bool initCommonControls()
{
	static bool haveInitialized = false;
	if (haveInitialized)
		return true;

	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof(init);
	init.dwICC = ICC_TREEVIEW_CLASSES;
	haveInitialized = !!::InitCommonControlsEx(&init);
	return haveInitialized;
}

static const int maxToolTipWidth = 250;

void TipWin::initializeToolTipWindow(HWND hWnd)
{
	if (!initCommonControls())
		return;

	if (!tooltip_hwnd_ || !IsWindow(tooltip_hwnd_))
	{
		parent_hwnd_ = hWnd;
		tooltip_hwnd_ = CreateWindowEx(WS_EX_TRANSPARENT, TOOLTIPS_CLASS, 0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			parent_hwnd_, 0, 0, 0);
		if (!tooltip_hwnd_)
			return;

		TOOLINFO info = { 0 };
		info.cbSize = sizeof(info);
		info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		info.uId = reinterpret_cast<UINT_PTR>(parent_hwnd_);

		::SendMessage(tooltip_hwnd_, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&info));
		::SendMessage(tooltip_hwnd_, TTM_SETMAXTIPWIDTH, 0, maxToolTipWidth);
	}
}

void TipWin::DestroyTipWin()
{
	if (tooltip_hwnd_ && IsWindow(tooltip_hwnd_)){
		DestroyWindow(tooltip_hwnd_);
		tooltip_hwnd_ = NULL;
	}

}

void TipWin::setToolTip(const std::wstring& toolTip)
{
	//if (!tooltip_hwnd_)
	//	return;

	if (toolTip == toolTip_)
		return;

	toolTip_ = toolTip;
	if ( toolTip_.empty() ){
		DestroyTipWin();
		return;
	}

	initializeToolTipWindow(parent_hwnd_);	

	if (!tooltip_hwnd_)
		return;

	if (!toolTip_.empty()) {
		TOOLINFO info = { 0 };
		info.cbSize = sizeof(info);
		info.uFlags = TTF_IDISHWND;
		info.uId = reinterpret_cast<UINT_PTR>(parent_hwnd_);
		//Vector<UChar> toolTipCharacters = m_toolTip.charactersWithNullTermination(); // Retain buffer long enough to make the SendMessage call
		//info.lpszText = const_cast<UChar*>(toolTipCharacters.data());
		info.lpszText = const_cast<WCHAR*>(toolTip_.c_str());
		::SendMessage(tooltip_hwnd_, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&info));
	}

	::SendMessage(tooltip_hwnd_, TTM_ACTIVATE, !toolTip_.empty(), 0);
}