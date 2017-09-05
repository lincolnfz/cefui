// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#include "stdafx.h"
#include "scheme_test.h"

#include <algorithm>
#include <string>

#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"
//#include "cefclient/resource_util.h"
//#include "cefclient/string_util.h"
#include "pack.h"

#if defined(OS_WIN)
//#include "cefclient/resource.h"
#endif

namespace scheme_test {

namespace {

	static const char* getMimeType(const char* ext){
		typedef struct _mime_table{
			char mime_val[64];
			char ext_val[10][8];
		}mime_table;
		const char *val = "";
		static mime_table table[] = {
			{ "text/html", { ".html", ".htm" } }
			, { "application/x-javascript;", { ".js" } }
			, { "text/css;", { ".css" } }
			, { "image/gif;", { ".gif" } }
			, { "image/png;", { ".png" } }
			, { "image/jpeg;", { ".jpeg", ".jpg" } }
		};
		static int len = sizeof(table) / sizeof(mime_table);
		bool bfind = false;
		for (int i = 0; i < len; ++i){
			for (int j = 0; j < 10; ++j){
				if (_stricmp(table[i].ext_val[j], ext) == 0){
					val = table[i].mime_val;
					bfind = true;
					break;
				}
			}
			if (bfind)
				break;
		}

		return val;
	}

	static const WCHAR* getMimeType(const WCHAR* ext){
		typedef struct _mime_table{
			WCHAR mime_val[64];
			WCHAR ext_val[10][8];
		}mime_table;
		const WCHAR *val = L"";
		static mime_table table[] = {
			{ L"text/html", { L".html", L".htm" } }
			, { L"application/x-javascript;", { L".js" } }
			, { L"text/css;", { L".css" } }
			, { L"image/gif;", { L".gif" } }
			, { L"image/png;", { L".png" } }
			, { L"image/jpeg;", { L".jpeg", L".jpg" } }
		};
		static int len = sizeof(table) / sizeof(mime_table);
		bool bfind = false;
		for (int i = 0; i < len; ++i){
			for (int j = 0; j < 10; ++j){
				if (_wcsicmp(table[i].ext_val[j], ext) == 0){
					val = table[i].mime_val;
					bfind = true;
					break;
				}
			}
			if (bfind)
				break;
		}

		return val;
	}

	std::string&   replace_all_distinct(std::string&   str, const   std::string&   old_value, const   std::string&   new_value)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())   {
			if ((pos = str.find(old_value, pos)) != std::string::npos)
				str.replace(pos, old_value.length(), new_value);
			else   break;
		}
		return   str;
	}

	std::wstring&   replace_all_distinct(std::wstring&   str, const   std::wstring&   old_value, const   std::wstring&   new_value)
	{
		for (std::wstring::size_type pos(0); pos != std::wstring::npos; pos += new_value.length())   {
			if ((pos = str.find(old_value, pos)) != std::wstring::npos)
				str.replace(pos, old_value.length(), new_value);
			else   break;
		}
		return   str;
	}

	std::string& removeUrlParm( std::string& str ){
		int idx = str.find("?");
		if ( idx > 0 )
		{
			str.erase(str.begin() + idx, str.end());
		}
		return str;
	}

	std::wstring& removeUrlParm(std::wstring& str){
		int idx = str.find(L"?");
		if (idx > 0)
		{
			str.erase(str.begin() + idx, str.end());
		}
		return str;
	}

// Implementation of the schema handler for client:// requests.
class ClientSchemeHandler : public CefResourceHandler {
 public:
  ClientSchemeHandler() : offset_(0) {
  }

  ~ClientSchemeHandler(){
  }

  BOOL UrlDecode(const char* szSrc, WCHAR* pBuf, int cbBufLen)
  {
	  if (szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		  return FALSE;

	  size_t len_ascii = strlen(szSrc);
	  if (len_ascii == 0)
	  {
		  pBuf[0] = 0;
		  return TRUE;
	  }

	  char *pUTF8 = (char*)malloc(len_ascii + 1);
	  if (pUTF8 == NULL)
		  return FALSE;

	  int cbDest = 0; //累加
	  unsigned char *pSrc = (unsigned char*)szSrc;
	  unsigned char *pDest = (unsigned char*)pUTF8;
	  while (*pSrc)
	  {
		  if (*pSrc == '%')
		  {
			  *pDest = 0;
			  //高位
			  if (pSrc[1] >= 'A' && pSrc[1] <= 'F')
				  *pDest += (pSrc[1] - 'A' + 10) * 0x10;
			  else if (pSrc[1] >= 'a' && pSrc[1] <= 'f')
				  *pDest += (pSrc[1] - 'a' + 10) * 0x10;
			  else
				  *pDest += (pSrc[1] - '0') * 0x10;

			  //低位
			  if (pSrc[2] >= 'A' && pSrc[2] <= 'F')
				  *pDest += (pSrc[2] - 'A' + 10);
			  else if (pSrc[2] >= 'a' && pSrc[2] <= 'f')
				  *pDest += (pSrc[2] - 'a' + 10);
			  else
				  *pDest += (pSrc[2] - '0');

			  pSrc += 3;
		  }
		  else if (*pSrc == '+')
		  {
			  *pDest = ' ';
			  ++pSrc;
		  }
		  else
		  {
			  *pDest = *pSrc;
			  ++pSrc;
		  }
		  ++pDest;
		  ++cbDest;
	  }
	  //null-terminator
	  *pDest = '\0';
	  ++cbDest;

	  //if (bUTF8)
	  {
		  int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, NULL, 0);
		  LPWSTR pUnicode = (LPWSTR)malloc(cchWideChar * sizeof(WCHAR));
		  if (pUnicode == NULL)
		  {
			  free(pUTF8);
			  return FALSE;
		  }
		  MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, pUnicode, cchWideChar);
		  //WideCharToMultiByte(CP_ACP, 0, pUnicode, cchWideChar, pBuf, cbBufLen, NULL, NULL);
		  wcscpy_s(pBuf, cbBufLen, pUnicode);
		  free(pUnicode);
	  }
	  //else{
	//	  strcpy_s(pBuf, cbBufLen, pUTF8);
	 // }


	  free(pUTF8);
	  return TRUE;
  }

  std::wstring char2wchar(const std::string& str)
  {
	  int iTextLen = 0;
	  iTextLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	  WCHAR* wsz = new WCHAR[iTextLen + 1];
	  memset((void*)wsz, 0, sizeof(WCHAR) * (iTextLen + 1));
	  ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wsz, iTextLen);
	  std::wstring strText(wsz);
	  delete[]wsz;
	  return strText;
  }

  std::wstring utf82wchar(const std::string& str)
  {
	  int iTextLen = 0;
	  iTextLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	  WCHAR* wsz = new WCHAR[iTextLen + 1];
	  memset((void*)wsz, 0, sizeof(WCHAR) * (iTextLen + 1));
	  ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wsz, iTextLen);
	  std::wstring strText(wsz);
	  delete[]wsz;
	  return strText;
  }

  virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                              CefRefPtr<CefCallback> callback)
                              OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    bool handled = false;
	
    std::string path = request->GetURL();
	WCHAR url_buf[8192] = {0};
	UrlDecode(path.c_str(), url_buf, 8192);
	std::wstring url = url_buf;
    /*if (strstr(url.c_str(), "index.html") != NULL) {
      // Build the response html
      data_ = "<html><head><title>Client Scheme Handler</title></head>"
              "<body bgcolor=\"white\">"
              "This contents of this page page are served by the "
              "ClientSchemeHandler class handling the client:// protocol."
              "<br/>You should see an image:"
              "<br/><img src=\"file:///D:/logo.png\"><pre>";

      // Output a string representation of the request
      std::string dump;
     // DumpRequestContents(request, dump);
      data_.append(dump);

      data_.append("</pre><br/>Try the test form:"
                   "<form method=\"POST\" action=\"handler.html\">"
                   "<input type=\"text\" name=\"field1\">"
                   "<input type=\"text\" name=\"field2\">"
                   "<input type=\"submit\">"
                   "</form></body></html>");

      handled = true;

      // Set the resulting mime type
      mime_type_ = "text/html";
    } else if (strstr(url.c_str(), "logo.png") != NULL) {
      // Load the response image
      //if (LoadBinaryResource("logo.png", data_))
	{
        handled = true;
        // Set the resulting mime type
        mime_type_ = "image/png";
      }
    }*/

	url = replace_all_distinct(url, L"/", L"\\");
	int idx = url.rfind(L".pack");
	if (idx > 0){
		std::wstring file = url.substr(0, idx + 5);
		file.erase(0, 9);
		std::wstring resource = removeUrlParm(url.substr(idx + 5));
		//std::string win_standfile = replace_all_distinct(file, "/", "\\");
		unsigned char* data = 0;
		unsigned long data_len = 0;

		//开始是两个\\,删除掉一个
		if ( resource.find(L"\\\\") == 0 )
		{
			resource.erase(resource.begin());
		}
		if (exZipFile(file.c_str(), resource.c_str(), &data, &data_len)){
			handled = true;
			data_ = std::string(reinterpret_cast<char*>(data), data_len);
			freeBuf(data);
			DCHECK(!data_.empty());
		}
		else{
			int i = 0;
		}
		WCHAR ext[64] = { 0 };
		_wsplitpath_s(resource.c_str(), NULL, 0, NULL, 0, NULL, 0, ext, 64);
		mime_type_ = getMimeType(ext);
		if (mime_type_.empty())
		{
			int i = 0;
		}
	}
	//request->
    if (handled) {
      // Indicate the headers are available.
      callback->Continue();
      return true;
    }

    return false;
  }

  virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                  int64& response_length,
                                  CefString& redirectUrl) OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    DCHECK(!data_.empty());

	DCHECK(!mime_type_.empty());

    response->SetMimeType(mime_type_);
    response->SetStatus(200);

    // Set the resulting response length
    response_length = data_.length();
  }

  virtual void Cancel() OVERRIDE {
    CEF_REQUIRE_IO_THREAD();
  }

  virtual bool ReadResponse(void* data_out,
                            int bytes_to_read,
                            int& bytes_read,
                            CefRefPtr<CefCallback> callback)
                            OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    bool has_data = false;
    bytes_read = 0;

    if (offset_ < data_.length()) {
      // Copy the next block of data into the buffer.
		int transfer_size  = min(bytes_to_read, static_cast<int>(data_.length() - offset_));
      memcpy(data_out, data_.data() + offset_, transfer_size);
      offset_ += transfer_size;

      bytes_read = transfer_size;
      has_data = true;
    }

    return has_data;
  }

 private:
  std::string data_;
  std::wstring mime_type_;
  size_t offset_;

  IMPLEMENT_REFCOUNTING(ClientSchemeHandler);
};

// Implementation of the factory for for creating schema handlers.
class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory {
 public:
  // Return a new scheme handler instance to handle the request.
  virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                               CefRefPtr<CefFrame> frame,
                                               const CefString& scheme_name,
                                               CefRefPtr<CefRequest> request)
                                               OVERRIDE {
    CEF_REQUIRE_IO_THREAD();
    return new ClientSchemeHandler();
  }

  IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
};

}  // namespace

void RegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar,
                           std::vector<CefString>& cookiable_schemes) {
  registrar->AddCustomScheme("xpack", true, true, true);
  registrar->AddCustomScheme("file", false, true, true);
  registrar->AddCustomScheme("resui", false, true, true);
}

void InitTest() {
  CefRegisterSchemeHandlerFactory("xpack", "",
      new ClientSchemeHandlerFactory());

  //保证scheme为5个字符,可以通用ClientSchemeHandler
  CefRegisterSchemeHandlerFactory("resui", "",
	  new ClientSchemeHandlerFactory());
}

void RegisterSchemes(CefRefPtr<CefRequestContext> request){
	request->RegisterSchemeHandlerFactory("xpack", "", new ClientSchemeHandlerFactory());
}

}  // namespace scheme_test
