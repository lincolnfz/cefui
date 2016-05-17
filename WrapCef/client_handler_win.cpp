// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "client_handler.h"

#include <string>
#include <windows.h>
#include <shlobj.h> 

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "WebViewFactory.h"
#include "WebkitEcho.h"
//#include "cefclient/resource.h"

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
  CEF_REQUIRE_UI_THREAD();

  std::wstring strUrl(url);
  if (GetBrowserId() == browser->GetIdentifier() && frame->IsMain()) {
    // Set the edit window text
    //SetWindowText(edit_handle_, std::wstring(url).c_str());
	  if (WebkitEcho::getFunMap())
	  {
		  int id = browser->GetIdentifier();
		  WebkitEcho::getFunMap()->webkitChangeUrl(id, strUrl.c_str());
	  }
  }
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  // Set the frame window title bar
  //CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  /*if (GetBrowserId() == browser->GetIdentifier()) {
    // The frame window will be the parent of the browser window
    hwnd = GetParent(hwnd);
  }*/

  std::wstring strTitle(title);
  CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
  if (item.get() && IsWindow(item->m_window->hwnd()))
  {
	  HWND hWnd = item->m_window->hwnd();
	  SetWindowText(hWnd, strTitle.c_str());
  }
  if ( WebkitEcho::getFunMap() )
  {
	  int id = browser->GetIdentifier();
	  WebkitEcho::getFunMap()->webkitChangeTitle(id, strTitle.c_str());
  }
}

bool ClientHandler::OnTooltip(CefRefPtr<CefBrowser> browser,
	CefString& text){

	CEF_REQUIRE_UI_THREAD();
	//CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	std::wstring tipTxt = text.ToWString();
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if (item.get())
	{
		item->m_window->ShowTip(tipTxt);
	}
	
	return true;
}

void ClientHandler::SendNotification(NotificationType type) {
  UINT id;

  //modi by lincoln
 /* switch (type) {
  case NOTIFY_CONSOLE_MESSAGE:
    id = ID_WARN_CONSOLEMESSAGE;
    break;
  case NOTIFY_DOWNLOAD_COMPLETE:
    id = ID_WARN_DOWNLOADCOMPLETE;
    break;
  case NOTIFY_DOWNLOAD_ERROR:
    id = ID_WARN_DOWNLOADERROR;
    break;
  default:
    return;
  }*/
  //PostMessage(main_handle_, WM_COMMAND, id, 0);
}

void ClientHandler::SetLoading(bool isLoading) {
	//modi by lincoln
  /*DCHECK(edit_handle_ != NULL && reload_handle_ != NULL &&
         stop_handle_ != NULL);
  EnableWindow(edit_handle_, TRUE);
  EnableWindow(reload_handle_, !isLoading);
  EnableWindow(stop_handle_, isLoading);*/
}

void ClientHandler::SetNavState(bool canGoBack, bool canGoForward) {
	//modi by lincoln
  /*DCHECK(back_handle_ != NULL && forward_handle_ != NULL);
  EnableWindow(back_handle_, canGoBack);
  EnableWindow(forward_handle_, canGoForward);*/
}

std::string ClientHandler::GetDownloadPath(const std::string& file_name) {
  TCHAR szFolderPath[MAX_PATH];
  std::string path;

  // Save the file in the user's "My Documents" folder.
  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
                                NULL, 0, szFolderPath))) {
    path = CefString(szFolderPath);
    path += "\\" + file_name;
  }

  return path;
}
