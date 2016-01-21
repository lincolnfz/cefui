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
				if (stricmp(table[i].ext_val[j], ext) == 0){
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

	std::string& removeUrlParm( std::string& str ){
		int idx = str.find("?");
		if ( idx > 0 )
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

  virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                              CefRefPtr<CefCallback> callback)
                              OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    bool handled = false;
	
    std::string url = request->GetURL();
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

	url = replace_all_distinct(url, "/", "\\");
	int idx = url.rfind(".pack");
	if (idx > 0){
		std::string file = url.substr(0, idx + 5);
		file.erase(0, 9);
		std::string resource = removeUrlParm(url.substr(idx + 5));
		//std::string win_standfile = replace_all_distinct(file, "/", "\\");
		unsigned char* data = 0;
		unsigned long data_len = 0;
		if (exZipFile(file.c_str(), resource.c_str(), &data, &data_len)){
			handled = true;
			data_ = std::string(reinterpret_cast<char*>(data), data_len);
			freeBuf(data);
			DCHECK(!data_.empty());
		}
		else{
			int i = 0;
		}
		char ext[64] = { 0 };
		_splitpath(resource.c_str(), 0, 0, 0, ext);
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
  std::string mime_type_;
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
  registrar->AddCustomScheme("xpack", false, true, true);
  registrar->AddCustomScheme("file", false, true, true);
}

void InitTest() {
  CefRegisterSchemeHandlerFactory("xpack", "",
      new ClientSchemeHandlerFactory());
}

}  // namespace scheme_test
