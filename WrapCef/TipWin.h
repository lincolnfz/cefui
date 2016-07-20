
#ifndef TIPWIN_H
#define TIPWIN_H
#pragma once

#include <Commctrl.h>
#include <string>

class TipWin
{
public:
	TipWin();
	virtual ~TipWin();

	void initializeToolTipWindow(HWND hWnd);
	void setToolTip(const std::wstring& toolTip);

	void SetParentHwnd(HWND hWnd);
	bool HandleNotify(int w_param, NMHDR* l_param, LRESULT* l_result);
	void DestroyTipWin();
private:
	//void DestroyTipWin();
	bool EnsureTooltipWindow();
	void PositionTooltip();
	void Show();
	void Hide();
	bool IsVisible();
	void TipWin::SetText(const std::wstring& tooltip_text,
		const POINT& location);
private:
	// The window |tooltip_hwnd_| is parented to.
	HWND parent_hwnd_;

	// Shows the tooltip.
	HWND tooltip_hwnd_;

	// Used to modify the tooltip.
	TOOLINFO toolinfo_;

	// Is the tooltip showing?
	bool showing_;

	POINT location_;

	std::wstring toolTip_;
};

#endif

