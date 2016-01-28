// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file.

#include "stdafx.h"
#include "osrenderer.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#if defined(OS_WIN)
#include <gl/gl.h>
#include <gl/glu.h>
#include <comdef.h>
#include <gdiplus.h>
#elif defined(OS_MACOSX)
#include <OpenGL/gl.h>
#elif defined(OS_LINUX)
#include <GL/gl.h>
#include <GL/glu.h>
#else
#error Platform is not supported.
#endif

#include "include/wrapper/cef_helpers.h"

#if defined(OS_WIN)
#pragma comment(lib, "gdiplus.lib")
#endif

#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

// DCHECK on gl errors.
#ifndef NDEBUG
#define VERIFY_NO_ERROR { \
    int _gl_error = glGetError(); \
    DCHECK(_gl_error == GL_NO_ERROR) << \
    "glGetError returned " << _gl_error; \
  }
#else
#define VERIFY_NO_ERROR
#endif

ClientOSRenderer::ClientOSRenderer(bool transparent,
                                   bool show_update_rect)
    : transparent_(transparent),
      show_update_rect_(show_update_rect),
      initialized_(false),
      texture_id_(0),
      view_width_(0),
      view_height_(0),
      spin_x_(0),
      spin_y_(0) {
}

ClientOSRenderer::~ClientOSRenderer() {
  Cleanup();
}

void ClientOSRenderer::Initialize() {
  if (initialized_)
    return;

  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); VERIFY_NO_ERROR;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); VERIFY_NO_ERROR;

  // Necessary for non-power-of-2 textures to render correctly.
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); VERIFY_NO_ERROR;

  // Create the texture.
  glGenTextures(1, &texture_id_); VERIFY_NO_ERROR;
  DCHECK_NE(texture_id_, 0U); VERIFY_NO_ERROR;

  glBindTexture(GL_TEXTURE_2D, texture_id_); VERIFY_NO_ERROR;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST); VERIFY_NO_ERROR;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST); VERIFY_NO_ERROR;
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); VERIFY_NO_ERROR;
  const GLubyte* byth = glGetString(GL_VERSION);
  byth = glGetString(GL_VENDOR);
  byth = glGetString(GL_RENDERER);
  byth = glGetString(GL_EXTENSIONS);
  //OutputDebugStringA((char*)byth);
  //Gdiplus::GdiplusStartupInput StartupInput;
  //GdiplusStartup(&m_gdiplusToken, &StartupInput, NULL);
  initialized_ = true;
}

void ClientOSRenderer::Cleanup() {
  if (texture_id_ != 0)
    glDeleteTextures(1, &texture_id_);
  //Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

void ClientOSRenderer::Render() {
  if (view_width_ == 0 || view_height_ == 0)
    return;

  DCHECK(initialized_);

  struct {
    float tu, tv;
    float x, y, z;
  } static vertices[] = {
    {0.0f, 1.0f, -1.0f, -1.0f, 0.0f},
    {1.0f, 1.0f,  1.0f, -1.0f, 0.0f},
    {1.0f, 0.0f,  1.0f,  1.0f, 0.0f},
    {0.0f, 0.0f, -1.0f,  1.0f, 0.0f}
  };

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); VERIFY_NO_ERROR;

  glMatrixMode(GL_MODELVIEW); VERIFY_NO_ERROR;
  glLoadIdentity(); VERIFY_NO_ERROR;

  // Match GL units to screen coordinates.
  glViewport(0, 0, view_width_, view_height_); VERIFY_NO_ERROR;
  glMatrixMode(GL_PROJECTION); VERIFY_NO_ERROR;
  glLoadIdentity(); VERIFY_NO_ERROR;

  // Draw the background gradient.
  glPushAttrib(GL_ALL_ATTRIB_BITS); VERIFY_NO_ERROR;
  // Don't check for errors until glEnd().
  glBegin(GL_QUADS);
  glColor4f(1.0, 0.0, 0.0, 1.0);  // red
  glVertex2f(-1.0, -1.0);
  glVertex2f(1.0, -1.0);
  glColor4f(0.0, 0.0, 1.0, 1.0);  // blue
  glVertex2f(1.0, 1.0);
  glVertex2f(-1.0, 1.0);
  glEnd(); VERIFY_NO_ERROR;
  glPopAttrib(); VERIFY_NO_ERROR;

  // Rotate the view based on the mouse spin.
  if (spin_x_ != 0) {
    glRotatef(-spin_x_, 1.0f, 0.0f, 0.0f); VERIFY_NO_ERROR;
  }
  if (spin_y_ != 0) {
    glRotatef(-spin_y_, 0.0f, 1.0f, 0.0f); VERIFY_NO_ERROR;
  }

  if (transparent_) {
    // Alpha blending style. Texture values have premultiplied alpha.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); VERIFY_NO_ERROR;

    // Enable alpha blending.
    glEnable(GL_BLEND); VERIFY_NO_ERROR;
  }

  // Enable 2D textures.
  glEnable(GL_TEXTURE_2D); VERIFY_NO_ERROR;

  // Draw the facets with the texture.
  DCHECK_NE(texture_id_, 0U); VERIFY_NO_ERROR;
  glBindTexture(GL_TEXTURE_2D, texture_id_); VERIFY_NO_ERROR;
  glInterleavedArrays(GL_T2F_V3F, 0, vertices); VERIFY_NO_ERROR;
  glDrawArrays(GL_QUADS, 0, 4); VERIFY_NO_ERROR;

  // Disable 2D textures.
  glDisable(GL_TEXTURE_2D); VERIFY_NO_ERROR;

  if (transparent_) {
    // Disable alpha blending.
    glDisable(GL_BLEND); VERIFY_NO_ERROR;
  }

  // Draw a rectangle around the update region.
  if (show_update_rect_ && !update_rect_.IsEmpty()) {
    int left = update_rect_.x;
    int right = update_rect_.x + update_rect_.width;
    int top = update_rect_.y;
    int bottom = update_rect_.y + update_rect_.height;

#if defined(OS_LINUX)
    // Shrink the box so that top & right sides are drawn.
    top += 1;
    right -= 1;
#else
    // Shrink the box so that left & bottom sides are drawn.
    left += 1;
    bottom -= 1;
#endif

    glPushAttrib(GL_ALL_ATTRIB_BITS); VERIFY_NO_ERROR
    glMatrixMode(GL_PROJECTION); VERIFY_NO_ERROR;
    glPushMatrix(); VERIFY_NO_ERROR;
    glLoadIdentity(); VERIFY_NO_ERROR;
    glOrtho(0, view_width_, view_height_, 0, 0, 1); VERIFY_NO_ERROR;

    glLineWidth(1); VERIFY_NO_ERROR;
    glColor3f(1.0f, 0.0f, 0.0f); VERIFY_NO_ERROR;
    // Don't check for errors until glEnd().
    glBegin(GL_LINE_STRIP);
    glVertex2i(left, top);
    glVertex2i(right, top);
    glVertex2i(right, bottom);
    glVertex2i(left, bottom);
    glVertex2i(left, top);
    glEnd(); VERIFY_NO_ERROR;

    glPopMatrix(); VERIFY_NO_ERROR;
    glPopAttrib(); VERIFY_NO_ERROR;
  }
}

void ClientOSRenderer::OnPopupShow(CefRefPtr<CefBrowser> browser,
                                   bool show) {
  if (!show) {
    // Clear the popup rectangle.
    ClearPopupRects();
  }
}

void ClientOSRenderer::OnPopupSize(CefRefPtr<CefBrowser> browser,
                                   const CefRect& rect) {
  if (rect.width <= 0 || rect.height <= 0)
    return;
  original_popup_rect_ = rect;
  popup_rect_ = GetPopupRectInWebView(original_popup_rect_);
}

CefRect ClientOSRenderer::GetPopupRectInWebView(const CefRect& original_rect) {
  CefRect rc(original_rect);
  // if x or y are negative, move them to 0.
  if (rc.x < 0)
    rc.x = 0;
  if (rc.y < 0)
    rc.y = 0;
  // if popup goes outside the view, try to reposition origin
  if (rc.x + rc.width > view_width_)
    rc.x = view_width_ - rc.width;
  if (rc.y + rc.height > view_height_)
    rc.y = view_height_ - rc.height;
  // if x or y became negative, move them to 0 again.
  if (rc.x < 0)
    rc.x = 0;
  if (rc.y < 0)
    rc.y = 0;
  return rc;
}

void ClientOSRenderer::ClearPopupRects() {
  popup_rect_.Set(0, 0, 0, 0);
  original_popup_rect_.Set(0, 0, 0, 0);
}

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num, size;
	Gdiplus::GetImageEncodersSize(&num, &size);
	Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
	bool found = false;
	for (UINT ix = 0; !found && ix < num; ++ix) {
		if (0 == _wcsicmp(pImageCodecInfo[ix].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[ix].Clsid;
			found = true;
		}
	}
	free(pImageCodecInfo);
	return found;
}

typedef _com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)>> IStreamSmartPtr;

void test32Bmp(const void* buffer, int width, int height){

	unsigned int linebyte = (((width << 5) + 31) >> 5) << 2;

	unsigned int lineInval = 4 - ((width << 5) >> 3) & 3;

	unsigned int sum = linebyte * height;

	//unsigned int total = width << 5 >> 3 * height;// width * 32 / 8 * height;
	unsigned int total = width * 4 * height;

	Gdiplus::Bitmap* pBitmap = NULL;
	IStream* pStream;

	HRESULT hResult = ::CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (hResult == S_OK && pStream)
	{
		Gdiplus::Status st;
		BITMAPINFO info;
		unsigned int headlen = sizeof(BITMAPINFO);
		memset(&info, 0, sizeof(BITMAPINFO));
		/*for (int i = 0; i < 256; i++)
		{
			info.bmiColors[i].rgbBlue = i;
			info.bmiColors[i].rgbGreen = i;
			info.bmiColors[i].rgbRed = i;
			info.bmiColors[i].rgbReserved = i;
		}*/

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
		bmphead.bfSize = sum + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bmphead.bfReserved1 = 0;
		bmphead.bfReserved2 = 0;
		bmphead.bfOffBits = bmphead.bfSize - sum;

		{			
			/*WCHAR filename[256];
			static int ssfe = 0;
			wsprintfW(filename, L"D:\\testbmp\\ssx_%d.bmp", ++ssfe);
			HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
			DWORD dw;
			WriteFile(hFile, &bmphead, sizeof(BITMAPFILEHEADER), &dw, NULL);
			WriteFile(hFile, &info, sizeof(BITMAPINFOHEADER), &dw, NULL);
			WriteFile(hFile, buffer, total, &dw, NULL);
			CloseHandle(hFile);*/
		}

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
		hResult = pStream->Write(buffer, total, NULL);
		if (hResult == S_OK){
			CLSID encoder;
			//Gdiplus::geten ::GetEncoderClsid(L"image/png", &pngid);
			pBitmap = Gdiplus::Bitmap::FromStream(pStream);
			pStream->Release();
			delete pBitmap;
			//HBITMAP hBitmap = NULL;
			//pBitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);
			//st = pBitmap->GetLastStatus();
			
		}
		//pStream->Release();
	}
}

void ClientOSRenderer::OnPaint(CefRefPtr<CefBrowser> browser,
                               CefRenderHandler::PaintElementType type,
                               const CefRenderHandler::RectList& dirtyRects,
                               const void* buffer, int width, int height) {
  if (!initialized_)
    Initialize();

  if (transparent_) {
    // Enable alpha blending.
    glEnable(GL_BLEND); VERIFY_NO_ERROR;
  }

  // Enable 2D textures.
  glEnable(GL_TEXTURE_2D); VERIFY_NO_ERROR;

  DCHECK_NE(texture_id_, 0U);
  glBindTexture(GL_TEXTURE_2D, texture_id_); VERIFY_NO_ERROR;

  if (type == PET_VIEW) {
    int old_width = view_width_;
    int old_height = view_height_;

    view_width_ = width;
	view_height_ = height;

	{
		int linebyte = (((width << 5) + 31) >> 5) << 2;

		int lineInval = 4 - ((width << 5) >> 3) & 3;
	}

    if (show_update_rect_)
      update_rect_ = dirtyRects[0];

    glPixelStorei(GL_UNPACK_ROW_LENGTH, view_width_); VERIFY_NO_ERROR;

    if (old_width != view_width_ || old_height != view_height_ ||
        (dirtyRects.size() == 1 &&
         dirtyRects[0] == CefRect(0, 0, view_width_, view_height_))) {
      // Update/resize the whole texture.
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0); VERIFY_NO_ERROR;
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0); VERIFY_NO_ERROR;	
	  test32Bmp(buffer, width, height);
      glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGBA, view_width_, view_height_, 0,
          GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer); VERIFY_NO_ERROR;
    } else {
      // Update just the dirty rectangles.
      CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();
      for (; i != dirtyRects.end(); ++i) {
        const CefRect& rect = *i;
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x); VERIFY_NO_ERROR;
        glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y); VERIFY_NO_ERROR;
		test32Bmp(buffer, width, height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width,
                        rect.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                        buffer); VERIFY_NO_ERROR;
      }
    }
  } else if (type == PET_POPUP && popup_rect_.width > 0 &&
             popup_rect_.height > 0) {
    int skip_pixels = 0, x = popup_rect_.x;
    int skip_rows = 0, y = popup_rect_.y;
    int w = width;
    int h = height;

    // Adjust the popup to fit inside the view.
    if (x < 0) {
      skip_pixels = -x;
      x = 0;
    }
    if (y < 0) {
      skip_rows = -y;
      y = 0;
    }
    if (x + w > view_width_)
      w -= x + w - view_width_;
    if (y + h > view_height_)
      h -= y + h - view_height_;

    // Update the popup rectangle.
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width); VERIFY_NO_ERROR;
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skip_pixels); VERIFY_NO_ERROR;
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skip_rows); VERIFY_NO_ERROR;
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_BGRA,
                    GL_UNSIGNED_INT_8_8_8_8_REV, buffer); VERIFY_NO_ERROR;
  }

  // Disable 2D textures.
  glDisable(GL_TEXTURE_2D); VERIFY_NO_ERROR;

  if (transparent_) {
    // Disable alpha blending.
    glDisable(GL_BLEND); VERIFY_NO_ERROR;
  }
}

void ClientOSRenderer::SetSpin(float spinX, float spinY) {
  spin_x_ = spinX;
  spin_y_ = spinY;
}

void ClientOSRenderer::IncrementSpin(float spinDX, float spinDY) {
  spin_x_ -= spinDX;
  spin_y_ -= spinDY;
}
