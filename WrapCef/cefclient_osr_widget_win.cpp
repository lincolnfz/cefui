// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include <comdef.h>
#include <gdiplus.h>
#include "cefclient_osr_widget_win.h"

#include <windowsx.h>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "client_app.h"
#include "WebViewFactory.h"
#include "IPC.h"

#ifdef _D3DX
#pragma comment(lib,"d3d9.lib")  
#pragma comment(lib,"d3dx9.lib")
#endif
//#include <windows.h>
//#include "cefclient/resource.h"

namespace {

class ScopedGLContext {
 public:
  ScopedGLContext(HDC hdc, HGLRC hglrc, bool swap_buffers)
    : hdc_(hdc),
      swap_buffers_(swap_buffers) {
    BOOL result = wglMakeCurrent(hdc, hglrc);
    DCHECK(result);
  }
  ~ScopedGLContext() {
    BOOL result = wglMakeCurrent(NULL, NULL);
    DCHECK(result);
    if (swap_buffers_) {
      result = SwapBuffers(hdc_);
      DCHECK(result);
    }
  }

 private:
  const HDC hdc_;
  const bool swap_buffers_;
};

}  // namespace


bool OSRWindow::s_singleProcess = true;
// static
CefRefPtr<OSRWindow> OSRWindow::Create(
	CefRefPtr<OSRBrowserProvider> browser_provider,
    bool transparent,
    bool show_update_rect) {
  DCHECK(browser_provider);
  if (!browser_provider)
    return NULL;

  return new OSRWindow(browser_provider, transparent, show_update_rect);
}

// static
CefRefPtr<OSRWindow> OSRWindow::From(
    CefRefPtr<ClientHandler::RenderHandler> renderHandler) {
  return static_cast<OSRWindow*>(renderHandler.get());
}

bool OSRWindow::CreateWidget(HWND hWndParent, const RECT& rect,
		HINSTANCE hInst, LPCTSTR className, const bool& trans, const int& sizetype) {
  DCHECK(hWnd_ == NULL && hDC_ == NULL && hRC_ == NULL);

  WNDCLASSEXW wndClass;
  wndClass.cbSize = sizeof(wndClass);
  if(!GetClassInfoExW(hInst, className, &wndClass))
	  RegisterOSRClass(hInst, className);

  bTrans_ = trans;
  //DWORD dwExStyle = WS_EX_LTRREADING|WS_EX_LEFT|WS_EX_RIGHTSCROLLBAR;
  DWORD dwExStyle = WS_EX_APPWINDOW;
  //DWORD dwExStyle = 0;
  DWORD dwStyle = WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;// | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  if ( bTrans_ )
  {
	  dwExStyle |= (WS_EX_LAYERED);
	  //dwExStyle &= ~WS_EX_APPWINDOW;	  
  }
  else{
	  //dwExStyle = WS_EX_APPWINDOW;
	  dwStyle |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
  }
  if ( sizetype == WIDGET_MIN_SIZE )
  {
	  dwStyle |= WS_MINIMIZE;
  }else if ( sizetype == WIDGET_MAX_SIZE )
  {
	  dwStyle |= WS_MAXIMIZE;
  }
  hWnd_ = ::CreateWindowEx(dwExStyle/*WS_EX_LAYERED*/ /*|WS_EX_APPWINDOW*/, className, 0,
	  dwStyle /*WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX*/,
	  rect.left,
	  rect.top,
	  rect.right - rect.left,
	  rect.bottom - rect.top,
      hWndParent, 0, hInst, 0);


  if (!hWnd_)
    return false;

  if (!bTrans_)
  {
	  bRenderDX_ = dx_Init(hWnd_, rect.right - rect.left, rect.bottom - rect.top);
  }  
  SetWindowLongPtr(hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  /*long styleEx = GetWindowLong(hWnd_, GWL_EXSTYLE);
  styleEx &= ~WS_EX_APPWINDOW;
  styleEx |= WS_EX_TOOLWINDOW;
  SetWindowLongPtr(hWnd_, GWL_EXSTYLE, styleEx);
  ShowWindow(hWnd_, SW_SHOW);*/

  // Reference released in OnDestroyed().
  AddRef();

#if defined(CEF_USE_ATL)
  drop_target_ = DropTargetWin::Create(this, hWnd_);
  HRESULT register_res = RegisterDragDrop(hWnd_, drop_target_);
  DCHECK_EQ(register_res, S_OK);
#endif
  tipinfo_.SetParentHwnd(hWnd_);
  return true;
}

void OSRWindow::DestroyWidget() {
  if (IsWindow(hWnd_))
    DestroyWindow(hWnd_);
}

void OSRWindow::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
#if defined(CEF_USE_ATL)
  RevokeDragDrop(hWnd_);
  drop_target_ = NULL;
#endif

  DisableGL();
  ::DestroyWindow(hWnd_);
}

bool OSRWindow::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                  CefRect& rect) {
  RECT window_rect = {0};
  HWND root_window = GetAncestor(hWnd_, GA_ROOT);
  if (::GetWindowRect(root_window, &window_rect)) {
    rect = CefRect(window_rect.left,
                   window_rect.top,
                   window_rect.right - window_rect.left,
                   window_rect.bottom - window_rect.top);
    return true;
  }
  return false;
}

bool OSRWindow::GetViewRect(CefRefPtr<CefBrowser> browser,
                            CefRect& rect) {
  RECT clientRect;
  if (!::GetClientRect(hWnd_, &clientRect))
    return false;
  rect.x = rect.y = 0;
  rect.width = clientRect.right;
  rect.height = clientRect.bottom;
  return true;
}

bool OSRWindow::GetScreenPoint(CefRefPtr<CefBrowser> browser,
                               int viewX,
                               int viewY,
                               int& screenX,
                               int& screenY) {
  if (!::IsWindow(hWnd_))
    return false;

  // Convert the point from view coordinates to actual screen coordinates.
  POINT screen_pt = {viewX, viewY};
  ClientToScreen(hWnd_, &screen_pt);
  screenX = screen_pt.x;
  screenY = screen_pt.y;
  return true;
}

void OSRWindow::OnPopupShow(CefRefPtr<CefBrowser> browser,
                            bool show) {
  if (!show) {
    renderer_.ClearPopupRects();
    browser->GetHost()->Invalidate(PET_VIEW);
  }
  renderer_.OnPopupShow(browser, show);
}

void OSRWindow::OnPopupSize(CefRefPtr<CefBrowser> browser,
                            const CefRect& rect) {
  renderer_.OnPopupSize(browser, rect);
}

int GetEncoderClsid2(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}


typedef _com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)>> IStreamPtr;

bool Gen32Bitmap(const void* buffer, int width, int height, Gdiplus::Bitmap*& pBitmap){

	bool bRet = false;
	unsigned int linebyte = (((width << 5) + 31) >> 5) << 2;

	unsigned int lineInval = 4 - ((width << 5) >> 3) & 3;

	unsigned int datalen = linebyte * height;

	pBitmap = NULL;
	IStreamPtr pStream;
	Gdiplus::Status st;
	HRESULT hResult = ::CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (hResult == S_OK && pStream)
	{		
		BITMAPINFO info;
		unsigned int headlen = sizeof(BITMAPINFO);
		memset(&info, 0, sizeof(BITMAPINFO));

		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biBitCount = 32;
		info.bmiHeader.biClrImportant = 0;
		info.bmiHeader.biClrUsed = 0;
		info.bmiHeader.biCompression = BI_RGB;
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biSizeImage = 0;
		info.bmiHeader.biXPelsPerMeter = 0;
		info.bmiHeader.biYPelsPerMeter = 0;
		info.bmiHeader.biHeight = -height;//  
		info.bmiHeader.biWidth = width;// 

		BITMAPFILEHEADER bmphead;
		bmphead.bfType = 0x4d42;
		bmphead.bfSize = datalen + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bmphead.bfReserved1 = 0;
		bmphead.bfReserved2 = 0;
		bmphead.bfOffBits = bmphead.bfSize - datalen;

		ULARGE_INTEGER uul;
		_LARGE_INTEGER off;
		off.HighPart = 0;
		off.LowPart = 0;

		hResult = pStream->Write(&bmphead, sizeof(BITMAPFILEHEADER), NULL);
		off.LowPart = sizeof(BITMAPFILEHEADER);
		hResult = pStream->Seek(off, off.LowPart, &uul);
		hResult = pStream->Write(&info, sizeof(BITMAPINFO), NULL);
		off.LowPart = sizeof(BITMAPINFO);
		hResult = pStream->Seek(off, uul.LowPart, &uul);
		hResult = pStream->Write(buffer, datalen, NULL);
		if (hResult == S_OK){			
			pBitmap = Gdiplus::Bitmap::FromStream(pStream);
			st = pBitmap->GetLastStatus();
		}
	}

	return st == Gdiplus::Status::Ok;
}

void Get32BitmapInfo(int width, int height, BITMAPINFO& info, unsigned long& datalen){

	unsigned int linebyte = (((width << 5) + 31) >> 5) << 2;

	//unsigned int lineInval = 4 - ((width << 5) >> 3) & 3;

	datalen = linebyte * height;

	unsigned int headlen = sizeof(BITMAPINFO);
	memset(&info, 0, sizeof(BITMAPINFO));

	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biClrImportant = 0;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biSizeImage = 0;
	info.bmiHeader.biXPelsPerMeter = 0;
	info.bmiHeader.biYPelsPerMeter = 0;
	info.bmiHeader.biHeight = height;//  
	info.bmiHeader.biWidth = width;// 

	BITMAPFILEHEADER bmphead;
	bmphead.bfType = 0x4d42;
	bmphead.bfSize = datalen + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmphead.bfReserved1 = 0;
	bmphead.bfReserved2 = 0;
	bmphead.bfOffBits = bmphead.bfSize - datalen;
}

void OSRWindow::SetAlpha(const unsigned int& alpha){
	m_alpah = alpha % 256;
}

void OSRWindow::ShowTip(const std::wstring& info)
{
	tipinfo_.setToolTip(info);
}

bool SaveToOtherFormat(Gdiplus::Bitmap* pImage, const wchar_t* pFileName)
{
	Gdiplus::EncoderParameters encoderParameters;
	CLSID fClsid;
	GetEncoderClsid2(L"image/png", &fClsid);
	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	ULONG quality = 100;
	encoderParameters.Parameter[0].Value = &quality;
	Gdiplus::Status status = pImage->Save(pFileName, &fClsid, &encoderParameters);
	if (status != Gdiplus::Ok)
	{
		return false;
	}

	return true;
}



bool SaveBitmapToFile(HBITMAP hBitmap)
{

	HPALETTE     hOldPal = NULL;	//定义文件，分配内存句柄，调色板句柄
	HANDLE hPal;
	// 处理调色板   
	hPal = GetStockObject(DEFAULT_PALETTE);

	Gdiplus::Bitmap bmp(hBitmap, (HPALETTE)hPal);
	static unsigned int idx = 0;
	WCHAR path[256];
	wsprintf(path, L"d:\\tt\\disp_%d.png", ++idx);
	SaveToOtherFormat(&bmp, path);


	return TRUE;
}

void OSRWindow::OnPaint(CefRefPtr<CefBrowser> browser,
                        PaintElementType type,
                        const RectList& dirtyRects,
                        const void* buffer,
                        int width, int height) {
  /*if (painting_popup_) {
    renderer_.OnPaint(browser, type, dirtyRects, buffer, width, height);
    return;
  }
  if (!hDC_)
    EnableGL();

  {
    ScopedGLContext scoped_gl_context(hDC_, hRC_, true);
    renderer_.OnPaint(browser, type, dirtyRects, buffer, width, height);
    if (type == PET_VIEW && !renderer_.popup_rect().IsEmpty()) {
      painting_popup_ = true;
      browser->GetHost()->Invalidate(PET_POPUP);
      painting_popup_ = false;
    }
    renderer_.Render();
  }
  return;*/

	BITMAPINFO info;
	unsigned long datalen = 0;
	Get32BitmapInfo(width, height, info, datalen);

	if ( bRenderDX_ )
	{
		if (type == PET_VIEW) {
			int old_width = view_width_;
			int old_height = view_height_;

			view_width_ = width;
			view_height_ = height;
			POINT destPt;
			POINT srcPt = { 0, 0 };
			SIZE destSize;			
			if (old_width != view_width_ || old_height != view_height_ ||
				(dirtyRects.size() == 1 &&
				dirtyRects[0] == CefRect(0, 0, view_width_, view_height_))) {
				destSize.cx = dirtyRects[0].width;
				destSize.cy = dirtyRects[0].height;
				dx_Render(buffer, datalen, 0, 0, destSize.cx, destSize.cy);
			}
			else{
				destPt.x = 0;
				destPt.y = 0;
				destSize.cx = view_width_;
				destSize.cy = view_height_;
				dx_Render(buffer, datalen, 0, 0, destSize.cx, destSize.cy);
			}
		}		
	}
	else{
		HDC hdc = ::GetDC(hWnd_);
		HDC hSrcDC = ::CreateCompatibleDC(hdc);
		void* bitbuf = NULL;
		HBITMAP hBitmap = ::CreateDIBSection(hSrcDC, &info, DIB_RGB_COLORS, (void**)&bitbuf, NULL, 0);
		if (hBitmap){
			if (SetBitmapBits(hBitmap, datalen, buffer)){
				HBITMAP hOldBmp = static_cast<HBITMAP>(SelectObject(hSrcDC, hBitmap));
				if (type == PET_VIEW) {
					int old_width = view_width_;
					int old_height = view_height_;

					view_width_ = width;
					view_height_ = height;

					POINT destPt;
					POINT srcPt = { 0, 0 };
					SIZE destSize;
					BLENDFUNCTION pb;
					pb.AlphaFormat = AC_SRC_ALPHA;
					pb.BlendOp = AC_SRC_OVER;
					pb.BlendFlags = 0;
					pb.SourceConstantAlpha = m_alpah;
					if (old_width != view_width_ || old_height != view_height_ ||
						(dirtyRects.size() == 1 &&
						dirtyRects[0] == CefRect(0, 0, view_width_, view_height_))) {
						destSize.cx = dirtyRects[0].width;
						destSize.cy = dirtyRects[0].height;
						if (bTrans_)
						{
							BOOL ret = ::UpdateLayeredWindow(hWnd_, hdc, 0, &destSize, hSrcDC, &srcPt, 0, &pb, ULW_ALPHA);
							if ( !ret )
							{
								if (GetLastError() == ERROR_INVALID_PARAMETER)
								{
									long val = GetWindowLong(hWnd_, GWL_EXSTYLE);
									val &= ~WS_EX_LAYERED;
									SetWindowLong(hWnd_, GWL_EXSTYLE, val);
									::SetWindowPos(hWnd_, 0, 0, 0, 0, 0,
										SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
									val = GetWindowLong(hWnd_, GWL_EXSTYLE);
									val |= WS_EX_LAYERED;
									SetWindowLong(hWnd_, GWL_EXSTYLE, val);
									::SetWindowPos(hWnd_, 0, 0, 0, 0, 0,
										SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
									::UpdateLayeredWindow(hWnd_, hdc, 0, &destSize, hSrcDC, &srcPt, 0, &pb, ULW_ALPHA);
									//bTrans_ = false;
									//BitBlt(hdc, 0, 0, destSize.cx, destSize.cy, hSrcDC, 0, 0, SRCCOPY);
								}
							}
						}
						else
							BitBlt(hdc, 0, 0, destSize.cx, destSize.cy, hSrcDC, 0, 0, SRCCOPY);
					}
					else {
						destPt.x = 0;
						destPt.y = 0;
						destSize.cx = view_width_;
						destSize.cy = view_height_;
						if (bTrans_){
							BOOL ret = UpdateLayeredWindow(hWnd_, hdc, 0, &destSize, hSrcDC, &srcPt, RGB(0, 0, 0), &pb, ULW_ALPHA);
							if (!ret)
							{
								if (GetLastError() == ERROR_INVALID_PARAMETER)
								{
									long val = GetWindowLong(hWnd_, GWL_EXSTYLE);
									val &= ~WS_EX_LAYERED;
									SetWindowLong(hWnd_, GWL_EXSTYLE, val);
									::SetWindowPos(hWnd_, 0, 0, 0, 0, 0,
										SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
									val = GetWindowLong(hWnd_, GWL_EXSTYLE);
									val |= WS_EX_LAYERED;
									SetWindowLong(hWnd_, GWL_EXSTYLE, val);
									::SetWindowPos(hWnd_, 0, 0, 0, 0, 0,
										SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
									::UpdateLayeredWindow(hWnd_, hdc, 0, &destSize, hSrcDC, &srcPt, 0, &pb, ULW_ALPHA);
								}
							}
						}
						else
							BitBlt(hdc, 0, 0, destSize.cx, destSize.cy, hSrcDC, 0, 0, SRCCOPY);
					}
				}
				else if (type == PET_POPUP) {

				}
				SelectObject(hSrcDC, hOldBmp);
			}
			//SaveBitmapToFile(hBitmap);
			DeleteObject(hBitmap);
		}

		DeleteDC(hSrcDC);
		ReleaseDC(hWnd_, hdc);
	}
	//OutputDebugStringW(L"--------------------------------OSRWindow::OnPaint");
}

void OSRWindow::OnCursorChange(CefRefPtr<CefBrowser> browser,
                               CefCursorHandle cursor,
                               CursorType type,
                               const CefCursorInfo& custom_cursor_info) {
  if (!::IsWindow(hWnd_))
    return;

  // Change the plugin window's cursor.
  SetClassLongPtr(hWnd_, GCLP_HCURSOR,
                  static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
  SetCursor(cursor);
}

bool OSRWindow::StartDragging(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefDragData> drag_data,
							   void* hbitmap,
							   int imgcx,
							   int imgcy,
							   int imgx,
							   int imgy,
                               CefRenderHandler::DragOperationsMask allowed_ops,
                               int x, int y) {
#if defined(CEF_USE_ATL)
  if (!drop_target_)
    return false;
  current_drag_op_ = DRAG_OPERATION_NONE;
  CefBrowserHost::DragOperationsMask result =
      drop_target_->StartDragging(browser, drag_data, hbitmap, imgcx, imgcy, imgx, imgy, allowed_ops, x, y);
  current_drag_op_ = DRAG_OPERATION_NONE;
  POINT pt = {};
  GetCursorPos(&pt);
  ScreenToClient(hWnd_, &pt);
  browser->GetHost()->DragSourceEndedAt(pt.x, pt.y, result);
  browser->GetHost()->DragSourceSystemDragEnded();
  return true;
#else
  // Cancel the drag. The dragging implementation requires ATL support.
  return false;
#endif
}

void OSRWindow::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                                 CefRenderHandler::DragOperation operation) {
#if defined(CEF_USE_ATL)
  current_drag_op_ = operation;
#endif
}

void OSRWindow::Invalidate() {
  if (!CefCurrentlyOn(TID_UI)) {
    CefPostTask(TID_UI, base::Bind(&OSRWindow::Invalidate, this));
    return;
  }

  // Don't post another task if the previous task is still pending.
  if (render_task_pending_)
    return;

  render_task_pending_ = true;

  // Render at 30fps.
  static const int kRenderDelay = 1000 / 30;
  CefPostDelayedTask(TID_UI, base::Bind(&OSRWindow::Render, this),
                     kRenderDelay);
}

void OSRWindow::WasHidden(bool hidden) {
  if (hidden == hidden_)
    return;
  CefRefPtr<CefBrowser> browser = browser_provider_->GetBrowser();
  if (!browser)
    return;
  browser->GetHost()->WasHidden(hidden);
  hidden_ = hidden;
}

#if defined(CEF_USE_ATL)

CefBrowserHost::DragOperationsMask
    OSRWindow::OnDragEnter(CefRefPtr<CefDragData> drag_data,
                           CefMouseEvent ev,
                           CefBrowserHost::DragOperationsMask effect) {
  browser_provider_->GetBrowser()->GetHost()->DragTargetDragEnter(
      drag_data, ev, effect);
  browser_provider_->GetBrowser()->GetHost()->DragTargetDragOver(ev, effect);
  return current_drag_op_;
}

CefBrowserHost::DragOperationsMask OSRWindow::OnDragOver(CefMouseEvent ev,
                              CefBrowserHost::DragOperationsMask effect) {
  browser_provider_->GetBrowser()->GetHost()->DragTargetDragOver(ev, effect);
  return current_drag_op_;
}

void OSRWindow::OnDragLeave() {
  browser_provider_->GetBrowser()->GetHost()->DragTargetDragLeave();
}

CefBrowserHost::DragOperationsMask
    OSRWindow::OnDrop(CefMouseEvent ev,
                      CefBrowserHost::DragOperationsMask effect) {
  browser_provider_->GetBrowser()->GetHost()->DragTargetDragOver(ev, effect);
  browser_provider_->GetBrowser()->GetHost()->DragTargetDrop(ev);
  return current_drag_op_;
}

#endif  // defined(CEF_USE_ATL)

OSRWindow::OSRWindow(CefRefPtr<OSRBrowserProvider> browser_provider,
                     bool transparent,
                     bool show_update_rect)
    : renderer_(transparent, show_update_rect),
      browser_provider_(browser_provider),
      hWnd_(NULL),
      hDC_(NULL),
      hRC_(NULL),
#if defined(CEF_USE_ATL)
      current_drag_op_(DRAG_OPERATION_NONE),
#endif
      painting_popup_(false),
      render_task_pending_(false),
      hidden_(false),
	  view_width_(0),
	  view_height_(0),
	  m_alpah(0xff){

	bRenderDX_ = false;
	bTrans_ = true;
#ifdef _D3DX
	d3d_ = NULL;
	d3ddev_ = NULL;
#endif
	m_bPrepareClose = false;
	m_bNeedClose = false;
}

OSRWindow::~OSRWindow() {
  DestroyWidget();
  dx_Destroy();
}

void OSRWindow::Render() {
  CEF_REQUIRE_UI_THREAD();
  if (render_task_pending_)
    render_task_pending_ = false;

  if (!hDC_)
    EnableGL();

  ScopedGLContext scoped_gl_context(hDC_, hRC_, true);
  renderer_.Render();
}

void OSRWindow::EnableGL() {
  CEF_REQUIRE_UI_THREAD();

  PIXELFORMATDESCRIPTOR pfd;
  int format;

  // Get the device context.
  hDC_ = GetDC(hWnd_);

  // Set the pixel format for the DC.
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;
  format = ChoosePixelFormat(hDC_, &pfd);
  SetPixelFormat(hDC_, format, &pfd);

  // Create and enable the render context.
  hRC_ = wglCreateContext(hDC_);

  ScopedGLContext scoped_gl_context(hDC_, hRC_, false);
  renderer_.Initialize();
}

void OSRWindow::DisableGL() {
  CEF_REQUIRE_UI_THREAD();

  if (!hDC_)
    return;

  {
    ScopedGLContext scoped_gl_context(hDC_, hRC_, false);
    renderer_.Cleanup();
  }

  if (IsWindow(hWnd_)) {
    // wglDeleteContext will make the context not current before deleting it.
    BOOL result = wglDeleteContext(hRC_);
    DCHECK(result);
    ReleaseDC(hWnd_, hDC_);
  }

  hDC_ = NULL;
  hRC_ = NULL;
}

void OSRWindow::OnDestroyed() {
	WebViewFactory::getInstance().RemoveWindow(hWnd_);
  SetWindowLongPtr(hWnd_, GWLP_USERDATA, 0L);
  hWnd_ = NULL;
  Release();
}

ATOM OSRWindow::RegisterOSRClass(HINSTANCE hInstance, LPCTSTR className) {
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS;
  wcex.lpfnWndProc   = &OSRWindow::WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = className;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
  return RegisterClassEx(&wcex);
}

bool OSRWindow::isKeyDown(WPARAM wparam) {
  return (GetKeyState(wparam) & 0x8000) != 0;
}

int OSRWindow::GetCefMouseModifiers(WPARAM wparam) {
  int modifiers = 0;
  if (wparam & MK_CONTROL)
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (wparam & MK_SHIFT)
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (isKeyDown(VK_MENU))
    modifiers |= EVENTFLAG_ALT_DOWN;
  if (wparam & MK_LBUTTON)
    modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  if (wparam & MK_MBUTTON)
    modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  if (wparam & MK_RBUTTON)
    modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  if (::GetKeyState(VK_CAPITAL) & 1)
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;
  return modifiers;
}

int OSRWindow::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
  int modifiers = 0;
  if (isKeyDown(VK_SHIFT))
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (isKeyDown(VK_CONTROL))
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (isKeyDown(VK_MENU))
    modifiers |= EVENTFLAG_ALT_DOWN;

  // Low bit set from GetKeyState indicates "toggled".
  if (::GetKeyState(VK_NUMLOCK) & 1)
    modifiers |= EVENTFLAG_NUM_LOCK_ON;
  if (::GetKeyState(VK_CAPITAL) & 1)
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;

  switch (wparam) {
  case VK_RETURN:
    if ((lparam >> 16) & KF_EXTENDED)
      modifiers |= EVENTFLAG_IS_KEY_PAD;
    break;
  case VK_INSERT:
  case VK_DELETE:
  case VK_HOME:
  case VK_END:
  case VK_PRIOR:
  case VK_NEXT:
  case VK_UP:
  case VK_DOWN:
  case VK_LEFT:
  case VK_RIGHT:
    if (!((lparam >> 16) & KF_EXTENDED))
      modifiers |= EVENTFLAG_IS_KEY_PAD;
    break;
  case VK_NUMLOCK:
  case VK_NUMPAD0:
  case VK_NUMPAD1:
  case VK_NUMPAD2:
  case VK_NUMPAD3:
  case VK_NUMPAD4:
  case VK_NUMPAD5:
  case VK_NUMPAD6:
  case VK_NUMPAD7:
  case VK_NUMPAD8:
  case VK_NUMPAD9:
  case VK_DIVIDE:
  case VK_MULTIPLY:
  case VK_SUBTRACT:
  case VK_ADD:
  case VK_DECIMAL:
  case VK_CLEAR:
    modifiers |= EVENTFLAG_IS_KEY_PAD;
    break;
  case VK_SHIFT:
    if (isKeyDown(VK_LSHIFT))
      modifiers |= EVENTFLAG_IS_LEFT;
    else if (isKeyDown(VK_RSHIFT))
      modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  case VK_CONTROL:
    if (isKeyDown(VK_LCONTROL))
      modifiers |= EVENTFLAG_IS_LEFT;
    else if (isKeyDown(VK_RCONTROL))
      modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  case VK_MENU:
    if (isKeyDown(VK_LMENU))
      modifiers |= EVENTFLAG_IS_LEFT;
    else if (isKeyDown(VK_RMENU))
      modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  case VK_LWIN:
    modifiers |= EVENTFLAG_IS_LEFT;
    break;
  case VK_RWIN:
    modifiers |= EVENTFLAG_IS_RIGHT;
    break;
  }
  return modifiers;
}

bool OSRWindow::IsOverPopupWidget(int x, int y) const {
  const CefRect& rc = renderer_.popup_rect();
  int popup_right = rc.x + rc.width;
  int popup_bottom = rc.y + rc.height;
  return (x >= rc.x) && (x < popup_right) &&
         (y >= rc.y) && (y < popup_bottom);
}

int OSRWindow::GetPopupXOffset() const {
  return renderer_.original_popup_rect().x - renderer_.popup_rect().x;
}

int OSRWindow::GetPopupYOffset() const {
  return renderer_.original_popup_rect().y - renderer_.popup_rect().y;
}

void OSRWindow::ApplyPopupOffset(int& x, int& y) const {
  if (IsOverPopupWidget(x, y)) {
    x += GetPopupXOffset();
    y += GetPopupYOffset();
  }
}

// Plugin window procedure.
// static
LRESULT CALLBACK OSRWindow::WndProc(HWND hWnd, UINT message,
                                    WPARAM wParam, LPARAM lParam) {
  static POINT lastMousePos, curMousePos;
  static bool mouseRotation = false;
  static bool mouseTracking = false;

  static int lastClickX = 0;
  static int lastClickY = 0;
  static CefBrowserHost::MouseButtonType lastClickButton = MBT_LEFT;
  static int gLastClickCount = 0;
  static long gLastClickTime = 0;

  static bool gLastMouseDownOnView = false;
  static unsigned int dbClickTiem = GetDoubleClickTime();

  OSRWindow* window =
      reinterpret_cast<OSRWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

  CefRefPtr<CefBrowser> browser;
  CefRefPtr<CefBrowserHost> browserhost;

  if (window && window->browser_provider_->GetBrowser().get()){
	  browser = window->browser_provider_->GetBrowser();
	  browserhost = browser->GetHost();
  }
	  

  LONG currentTime = 0;
  bool cancelPreviousClick = false;

  
  if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
      message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
      message == WM_MOUSELEAVE) {
    currentTime = GetMessageTime();
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    cancelPreviousClick =
        (abs(lastClickX - x) > (GetSystemMetrics(SM_CXDOUBLECLK)/2 ))
        || (abs(lastClickY - y) > (GetSystemMetrics(SM_CYDOUBLECLK)/2 ))
		|| ((currentTime - gLastClickTime) > dbClickTiem);
    if (cancelPreviousClick &&
        (message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
      gLastClickCount = 0;
      lastClickX = 0;
      lastClickY = 0;
      gLastClickTime = 0;
    }
  }

  switch (message) {
  case WM_DESTROY:
    if (window)
      window->OnDestroyed();
    return 0;
  case WM_CREATE:{
	  PostMessage(hWnd, WM_SETFOCUS, NULL, NULL);
  }
	break;
  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN: {
    SetCapture(hWnd);
    SetFocus(hWnd);
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    if (wParam & MK_SHIFT) {
      // Start rotation effect.
      lastMousePos.x = curMousePos.x = x;
      lastMousePos.y = curMousePos.y = y;
	  mouseRotation = false;// true;
	  OutputDebugStringA("-----[ shift");
    } else {
      CefBrowserHost::MouseButtonType btnType =
          (message == WM_LBUTTONDOWN ? MBT_LEFT : (
           message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
      if (!cancelPreviousClick && (btnType == lastClickButton)) {
        ++gLastClickCount;
		if ( gLastClickCount == 2 )
		{
			++gLastClickCount;
		}
      } else {
        gLastClickCount = 1;
        lastClickX = x;
        lastClickY = y;
      }
      gLastClickTime = currentTime;
      lastClickButton = btnType;

	  if (browserhost.get()) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        gLastMouseDownOnView = !window->IsOverPopupWidget(x, y);
        window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
		mouse_event.modifiers = GetCefMouseModifiers(wParam);
		browserhost->SendMouseClickEvent(mouse_event, btnType, false,
                                     gLastClickCount);
      }
    }
    break;
  }

  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
    if (GetCapture() == hWnd)
      ReleaseCapture();
    if (mouseRotation) {
      // End rotation effect.
      mouseRotation = false;
      window->renderer_.SetSpin(0, 0);
      window->Invalidate();
    } else {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      CefBrowserHost::MouseButtonType btnType =
          (message == WM_LBUTTONUP ? MBT_LEFT : (
           message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
	  if (browserhost.get()) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        if (gLastMouseDownOnView &&
            window->IsOverPopupWidget(x, y) &&
            (window->GetPopupXOffset() || window->GetPopupYOffset())) {
          break;
        }
        window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
        mouse_event.modifiers = GetCefMouseModifiers(wParam);
		browserhost->SendMouseClickEvent(mouse_event, btnType, true,
                                     gLastClickCount);
      }
    }
    break;
  case WM_LBUTTONDBLCLK:{
	  if ( gLastClickCount == 1 )
	  {
		  if (browserhost.get()) {
			  int x = GET_X_LPARAM(lParam);
			  int y = GET_Y_LPARAM(lParam);
			  CefMouseEvent mouse_event;
			  mouse_event.x = x;
			  mouse_event.y = y;
			  window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
			  mouse_event.modifiers = GetCefMouseModifiers(wParam);
			  browserhost->SendMouseClickEvent(mouse_event, MBT_LEFT, false,
				  2);
			}	  
	  }
  }
	break;
  case WM_MOUSEMOVE: {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    if (mouseRotation) {
      // Apply rotation effect.
      curMousePos.x = x;
      curMousePos.y = y;
      window->renderer_.IncrementSpin((curMousePos.x - lastMousePos.x),
        (curMousePos.y - lastMousePos.y));
      lastMousePos.x = curMousePos.x;
      lastMousePos.y = curMousePos.y;
      window->Invalidate();
    } else {
      if (!mouseTracking) {
        // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
        // be generated.
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);
        mouseTracking = true;
      }
	  if (browserhost.get()) {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
        mouse_event.modifiers = GetCefMouseModifiers(wParam);
		browserhost->SendMouseMoveEvent(mouse_event, false);
      }
    }
    break;
  }

  case WM_MOUSELEAVE:
    if (mouseTracking) {
      // Stop tracking mouse leave.
      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE & TME_CANCEL;
      tme.hwndTrack = hWnd;
      TrackMouseEvent(&tme);
      mouseTracking = false;
    }
	if (browserhost.get()) {
      // Determine the cursor position in screen coordinates.
      POINT p;
      ::GetCursorPos(&p);
      ::ScreenToClient(hWnd, &p);

      CefMouseEvent mouse_event;
      mouse_event.x = p.x;
      mouse_event.y = p.y;
      mouse_event.modifiers = GetCefMouseModifiers(wParam);
	  browserhost->SendMouseMoveEvent(mouse_event, true);
    }
    break;

  case WM_MOUSEWHEEL:
	  if (browserhost.get()) {
      POINT screen_point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      HWND scrolled_wnd = ::WindowFromPoint(screen_point);
      if (scrolled_wnd != hWnd) {
        break;
      }
      ScreenToClient(hWnd, &screen_point);
      int delta = GET_WHEEL_DELTA_WPARAM(wParam);

      CefMouseEvent mouse_event;
      mouse_event.x = screen_point.x;
      mouse_event.y = screen_point.y;
      window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
      mouse_event.modifiers = GetCefMouseModifiers(wParam);

	  browserhost->SendMouseWheelEvent(mouse_event,
                                   isKeyDown(VK_SHIFT) ? delta : 0,
                                   !isKeyDown(VK_SHIFT) ? delta : 0);
    }
    break;

  case WM_SIZE:
	  if (browserhost.get()){
		  browserhost->WasResized();
	  }
    break;

  case WM_SETFOCUS:
  case WM_KILLFOCUS:{
	  if (browserhost.get())
	  {
#ifdef _DEBUG1
		  WCHAR sz[256];
		  wsprintf(sz, L"----- is focus hwnd=%0x, %d", hWnd, message == WM_SETFOCUS);
		  OutputDebugStringW(sz);
#endif
		  if ( window )
		  {
			//window->tipinfo_.DestroyTipWin();
		  }		  
		  browserhost->SendFocusEvent(message == WM_SETFOCUS);
		  if ( message == WM_KILLFOCUS )
		  {
			  CefMouseEvent mouse_event;
			  mouse_event.x = -100;
			  mouse_event.y = -100;
			  mouse_event.modifiers = 0;
			  browserhost->SendMouseMoveEvent(mouse_event, true);
			  //SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		  }
	  }
	  else{
#ifdef _DEBUG1
		  WCHAR sz[256];
		  wsprintf(sz, L"----- focus no find browser=%0x, %d", hWnd, message == WM_SETFOCUS);
		  OutputDebugStringW(sz);
#endif
	  }
  }
    break;

  case WM_CAPTURECHANGED:
  case WM_CANCELMODE:
    if (!mouseRotation) {
		if (browserhost.get())
			browserhost->SendCaptureLostEvent();
    }
    break;
  case WM_SYSCHAR:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_CHAR: {
    CefKeyEvent event;
    event.windows_key_code = wParam;
    event.native_key_code = lParam;
    event.is_system_key = message == WM_SYSCHAR ||
                          message == WM_SYSKEYDOWN ||
                          message == WM_SYSKEYUP;

    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
      event.type = KEYEVENT_RAWKEYDOWN;
    else if (message == WM_KEYUP || message == WM_SYSKEYUP)
      event.type = KEYEVENT_KEYUP;
    else
      event.type = KEYEVENT_CHAR;
    event.modifiers = GetCefKeyboardModifiers(wParam, lParam);
	if (browserhost.get()){
		browserhost->SendKeyEvent(event);
	}
    break;
  }

  case WM_PAINT: {
    PAINTSTRUCT ps;
    RECT rc;
    BeginPaint(hWnd, &ps);
    rc = ps.rcPaint;
    EndPaint(hWnd, &ps);
	if (browserhost.get())
		browserhost->Invalidate(PET_VIEW);
    return 0;
  }

  case WM_ERASEBKGND:
    return 0;

  case WM_GETMINMAXINFO:
  {
	  RECT rcScr;
	  SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScr, 0);
	  MINMAXINFO *pmmi = (MINMAXINFO*)lParam;
	  pmmi->ptMaxSize.x = rcScr.right - rcScr.left;
	  pmmi->ptMaxSize.y = rcScr.bottom - rcScr.top;
	  pmmi->ptMaxPosition.x = rcScr.left;
	  pmmi->ptMaxPosition.y = rcScr.top;
	  return 0;
  }
  break;
  case WM_CLOSE:
  {
#ifdef _DEBUG1
	  WCHAR szbuf[128] = { 0 };
	  wsprintfW(szbuf, L"-----[ revc  close in ui hwnd: %d", hWnd);
	  OutputDebugStringW(szbuf);
#endif
	  bool bPrepareClose = true;
	  if ( browser.get() )
	  {
		  int ipcID = 0;
		  if (!OSRWindow::s_singleProcess && browser.get())
		  {
			  CefRefPtr<OSRWindow> window = WebViewFactory::getInstance().getWindowByID(browser->GetIdentifier());
			  if (window.get())
			  {
				  //ipcID = item->m_ipcID;
				  //std::shared_ptr<cyjh::IPCUnit> ipc = cyjh::IPC_Manager::getInstance().GetIpc(ipcID);
				  CefRefPtr<cyjh::UIThreadCombin> ipcsync = ClientApp::getGlobalApp()->getUIThreadCombin();
				  window->m_bNeedClose = true;
				  {
					  int id = browser->GetIdentifier();
					  if (!ipcsync->isEmptyRequest(id) && !ipcsync->isEmptyResponse(id))
					  {
						  return 0;
					  }
				  }
				  
				  bPrepareClose = window->m_bPrepareClose;
				  if ( bPrepareClose == false )
				  {
					  window->m_bPrepareClose = true;
					  int id = 0;
					  if (browser.get())
					  {
						  id = browser->GetIdentifier();
					  }
#ifdef _DEBUG1
					  WCHAR szbuf[128] = { 0 };
					  wsprintfW(szbuf, L"-----[ prepare close in ui, id : %d, hwnd: %d", id, hWnd);
					  OutputDebugStringW(szbuf);
#endif
					  cyjh::Instruct parm;
					  parm.setName("closeBrowser");
					  
					  parm.getList().AppendVal(id);
					  parm.setInstructType(cyjh::INSTRUCT_REQUEST);
					  cyjh::Pickle pick;
					  cyjh::Instruct::SerializationInstruct(&parm, pick);
					  CefRefPtr<cyjh::UIThreadCombin> ipcsync = ClientApp::getGlobalApp()->getUIThreadCombin();
					  std::shared_ptr<cyjh::Instruct> spOut(new cyjh::Instruct);
					  ipcsync->Request(browser, parm, spOut);

					  //ipcsync->AsyncRequest(browser, parm);
					  ipcsync->DisableSendBrowser(id);
				  }
			  }
		  }
		  //if ( bPrepareClose )
		  {
			  browserhost->CloseBrowser(true); //关闭单独
		  }
		  
		  //window->browser_provider_->GetClientHandler()->CloseAllBrowsers(true);
	  }
	  //if (bPrepareClose == false)
	  //{
	//	  return 0;
	  //}
	 // else{
		  return DefWindowProc(hWnd, message, wParam, lParam);
	 // }
  }
  break;

  }


  return DefWindowProc(hWnd, message, wParam, lParam);
}


bool OSRWindow::dx_Init(HWND window, int width, int height)
{
	bool ret = false;
#ifdef _D3DX
	//initialize Direct3D
	d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d_ == NULL)
	{
		return 0;
	}

	//set Direct3D presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = 0;
	d3dpp.BackBufferHeight = 0;
	d3dpp.hDeviceWindow = window;

	//create Direct3D device
	d3d_->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev_);

	if (d3ddev_ == NULL)
	{
		return 0;
	}
	ret = true;
#endif
	return ret;
}

void OSRWindow::dx_Render(const void* data, unsigned int size, int x, int y, int width, int height)
{
#ifdef _D3DX
	//make sure the Direct3D device is valid
	if (!d3ddev_) return;

	//clear the backbuffer to bright green
	d3ddev_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	//d3ddev_->set
	//start rendering
	if (d3ddev_->BeginScene())
	{
		//do something?
		{
			LPD3DXSPRITE pSprite = NULL;
			LPDIRECT3DTEXTURE9 pTexture = NULL;
			if (D3DXCreateSprite(d3ddev_, &pSprite) == D3D_OK)
			{
				if (D3DXCreateTexture(d3ddev_, width, height, D3DX_DEFAULT, 0,
					D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
					&pTexture) == D3D_OK)
				{
					D3DLOCKED_RECT lockrc = { 0, 0 };
					if (pTexture->LockRect(0, &lockrc, NULL, 0) == D3D_OK)
					{
						memcpy(lockrc.pBits, data, size);
						pTexture->UnlockRect(0);
						pSprite->Begin(D3DXSPRITE_ALPHABLEND);
						RECT rct = { x, y, width, height };
						pSprite->Draw(pTexture, &rct, NULL, NULL, 0xffffffff);
						pSprite->End();
					}										
					pTexture->Release();
				}
				pSprite->Release();
			}

		}

		//stop rendering
		d3ddev_->EndScene();

		//copy back buffer on the screen
		d3ddev_->Present(NULL, NULL, NULL, NULL);
	}
#endif
}

void OSRWindow::dx_Destroy()
{
#ifdef _D3DX
	//display close message

	//free memory
	if (d3ddev_) d3ddev_->Release();
	if (d3d_) d3d_->Release();
#endif
}