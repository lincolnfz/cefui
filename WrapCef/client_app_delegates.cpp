// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "client_app.h"
#include "client_renderer.h"
//#include "performance_test.h"
#include "scheme_test.h"

#if defined(OS_LINUX)
#include "cefclient/print_handler_gtk.h"
#endif
#include <include/base/cef_logging.h>

// static
void ClientApp::CreateBrowserDelegates(BrowserDelegateSet& delegates) {
}

//这是一个响应js的c++ 函数(类)
class Handler : public CefV8Handler {
public:
	Handler(CefRefPtr<CefBrowser> browser) :browser_(browser){}
	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE{

		//arguments.size();
		//arguments[0]->IsString();
		//arguments[0]->GetStringValue().c_str();
		CefRefPtr<CefProcessMessage> message =
		CefProcessMessage::Create("fdesa");
		message->GetArgumentList()->SetBool(0, false);
		browser_->SendProcessMessage(PID_BROWSER, message);
		//browser_->SendProcessDoneFunction(PID_BROWSER, CefString("invokemethod"), CefString("ss"), false, true);
		retval = CefV8Value::CreateString("ok111");
		return true;
	}

	CefRefPtr<CefV8Value> object_;
	CefRefPtr<CefBrowser> browser_;

	IMPLEMENT_REFCOUNTING(Handler);
};

// Handle bindings in the render process.
class MyRenderDelegate : public ClientApp::RenderDelegate {
public:
	MyRenderDelegate() {
	}

	virtual void OnContextCreated(CefRefPtr<ClientApp> app,
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context) OVERRIDE{
		class Accessor : public CefV8Accessor {
		public:
			Accessor() : val_(CefV8Value::CreateBool(true)) {}
			virtual bool Get(const CefString& name,
				const CefRefPtr<CefV8Value> object,
				CefRefPtr<CefV8Value>& retval,
				CefString& exception) OVERRIDE{
				retval = val_;
				return true;
			}
				virtual bool Set(const CefString& name,
				const CefRefPtr<CefV8Value> object,
				const CefRefPtr<CefV8Value> value,
				CefString& exception) OVERRIDE{
				return true;
			}
			CefRefPtr<CefV8Value> val_;
			IMPLEMENT_REFCOUNTING(Accessor);
		};
		CefRefPtr<CefV8Value> window = context->GetGlobal();
		CefRefPtr<CefV8Accessor> myV8Acc = new Accessor;
		CefRefPtr<CefV8Value> val = CefV8Value::CreateString(L"Application");
		CefString cefException;
		myV8Acc->Set(L"name", window, val, cefException);
		CefRefPtr<CefV8Value> pObjApp = CefV8Value::CreateObject(myV8Acc);
		window->SetValue(L"Application", pObjApp, V8_PROPERTY_ATTRIBUTE_NONE);
		const char myfuname[] = "myfoo";
		CefRefPtr<CefV8Handler> myV8handle = new Handler(browser);
		CefRefPtr<CefV8Value> myFun = CefV8Value::CreateFunction(myfuname, myV8handle);
		CefV8Value::PropertyAttribute attributes =
			static_cast<CefV8Value::PropertyAttribute>(
			V8_PROPERTY_ATTRIBUTE_READONLY |
			V8_PROPERTY_ATTRIBUTE_DONTENUM |
			V8_PROPERTY_ATTRIBUTE_DONTDELETE);
		
		//DCHECK_EQ(context->GetGlobal()->SetValue(myfuname, myFun, attributes), true);
		context->GetGlobal()->SetValue(myfuname, myFun, attributes);
	}

private:
	IMPLEMENT_REFCOUNTING(MyRenderDelegate);
};

// static
void ClientApp::CreateRenderDelegates(RenderDelegateSet& delegates) {
  client_renderer::CreateRenderDelegates(delegates);
  //performance_test::CreateRenderDelegates(delegates);
  delegates.insert(new MyRenderDelegate);
}

// static
void ClientApp::RegisterCustomSchemes(
    CefRefPtr<CefSchemeRegistrar> registrar,
    std::vector<CefString>& cookiable_schemes) {
  scheme_test::RegisterCustomSchemes(registrar, cookiable_schemes);
}

// static
CefRefPtr<CefPrintHandler> ClientApp::CreatePrintHandler() {
#if defined(OS_LINUX)
  return new ClientPrintHandlerGtk();
#else
  return NULL;
#endif
}

