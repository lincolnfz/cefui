// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "client_app.h"
#include "client_renderer.h"
//#include "performance_test.h"
#include "scheme_test.h"
#include "BridageHost.h"

#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#if defined(OS_LINUX)
#include "cefclient/print_handler_gtk.h"
#endif
#include <include/base/cef_logging.h>

// static
void ClientApp::CreateBrowserDelegates(BrowserDelegateSet& delegates) {
}

//定义JS回调函数指针
typedef boost::function<void(const CefV8ValueList&, CefRefPtr<CefV8Value>&)> HandleCallback;
typedef std::map<unsigned long, HandleCallback> FunctionMap;

//定义JS属性函数指针
typedef boost::function<void(CefRefPtr<CefV8Value>&)> ProtyCallback;
typedef std::map<unsigned long, ProtyCallback> ProtyMap;


//这是一个响应js的c++ 函数(类)
class NativeappHandler : public CefV8Handler {
public:
	NativeappHandler(CefRefPtr<CefBrowser> browser) :browser_(browser){
		//RegisterFunction("setWindowSize", &NativeappHandler::setWindowSize);
	}
	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE{

		boost::hash<std::string> string_hash;
		return callfn(string_hash(name.ToString().c_str()), arguments, retval );

		//arguments.size();
		//arguments[0]->IsString();
		//arguments[0]->GetStringValue().c_str();
		//CefRefPtr<CefProcessMessage> message =
		//CefProcessMessage::Create("fdesa");
		//message->GetArgumentList()->SetBool(0, false);
		//browser_->SendProcessMessage(PID_BROWSER, message);
		//browser_->SendProcessDoneFunction(PID_BROWSER, CefString("invokemethod"), CefString("ss"), false, true);
		//retval = CefV8Value::CreateString("ok111");
		//return true;
	}

		//注册成员回调函数
	//template<typename T>
	bool RegisterFunction(const char* pFunctionName, void(NativeappHandler::*function)(const CefV8ValueList&, CefRefPtr<CefV8Value>&) )
	{
		boost::hash<std::string> string_hash;
		return m_FunctionMap.insert(std::make_pair(string_hash(pFunctionName),
			boost::bind(function, this, _1, _2))).second;
	}

	//impl
	void setWindowSize(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		//val = CefV8Value::CreateString("map458");
	}

	void minWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		int i = 0;
	}

	void maxWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void restoreWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void closeWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void setWindowText(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void fullScreen(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void createWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void createModalWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void createModalWindow2(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void setAlpha(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void invokeMethod(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void writePrivateProfileString(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void getPrivateProfileInt(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void getPrivateProfileString(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){

		CefRefPtr<CefProcessMessage> message =
			CefProcessMessage::Create("getPrivateProfileString");
		message->GetArgumentList()->SetString(0, CefString(L"中男is_editable"));
		//browser_->SendProcessMessageEx(PID_BROWSER, message, true, -1, 987, true);
		CefRefPtr<CefListValue> response;
		//browser_->SendProcessMessageEx(PID_BROWSER, message, true, -1, 0, true);
		int id = browser_->GetIdentifier();
		DWORD tid = GetCurrentThreadId();
		browser_->SendSyncProcessMessage(message, response);
		
		//CefContentRendererClient::Get()->render_task_runner()->
		//BridageHost::getInst().SendRequest(browser_, message, response, 60000);
		//CefString ss = response->GetString(0);
		//CefProcessHostMsg_GetNewRenderThreadInfo_Params params;
		//content::RenderThread::Get()
		//thread->Send(new CefProcessHostMsg_GetNewRenderThreadInfo(&params));
		int i = 0;
	}

	void setWindowPos(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void pushMessage(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void crossInvokeWebMethod(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void crossInvokeWebMethod2(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void winProty(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void setProfile(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void getProfile(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void getSoftwareAttribute(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void addFrameStateChanged(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

	void removeFrameStateChanged(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
	}

protected:
	bool callfn(const unsigned long& id, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval){
		bool ret = false;
		FunctionMap::iterator it = m_FunctionMap.find(id);
		if ( it != m_FunctionMap.end() )
		{
			it->second(arguments, retval);
			ret = true;
		}
		return ret;
	}


private:
	CefRefPtr<CefV8Value> object_;
	CefRefPtr<CefBrowser> browser_;
	FunctionMap	m_FunctionMap;

	IMPLEMENT_REFCOUNTING(NativeappHandler);
};

class Nativeapp : public CefV8Accessor {
public:
	Nativeapp(CefRefPtr<CefBrowser> browser):browser_(browser){}
	virtual bool Get(const CefString& name,
		const CefRefPtr<CefV8Value> object,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE
	{
		boost::hash<std::string> string_hash;
		return callProty(string_hash(name.ToString().c_str()), retval);
	}
		virtual bool Set(const CefString& name,
		const CefRefPtr<CefV8Value> object,
		const CefRefPtr<CefV8Value> value,
		CefString& exception) OVERRIDE
	{
		return true;
	}

	bool RegisterFunction(const char* protyName, void(Nativeapp::*function)(CefRefPtr<CefV8Value>&))
	{
		boost::hash<std::string> string_hash;
		return m_map.insert(std::make_pair(string_hash(protyName),
			boost::bind(function, this, _1))).second;
	}

	void appname(CefRefPtr<CefV8Value>& value){

	}

	void appDir(CefRefPtr<CefV8Value>& value){

	}

	void appDataPath(CefRefPtr<CefV8Value>& value){

	}

	void screen_w(CefRefPtr<CefV8Value>& value){

	}

	void screen_h(CefRefPtr<CefV8Value>& value){

	}

	void desktop_w(CefRefPtr<CefV8Value>& value){

	}

	void desktop_h(CefRefPtr<CefV8Value>& value){

	}

	void window_x(CefRefPtr<CefV8Value>& value){

	}

	void window_y(CefRefPtr<CefV8Value>& value){

	}

	void window_w(CefRefPtr<CefV8Value>& value){

	}

	void window_h(CefRefPtr<CefV8Value>& value){

	}

	void is_zoomed(CefRefPtr<CefV8Value>& value){
		value = CefV8Value::CreateBool(true);
	}

	void is_iconic(CefRefPtr<CefV8Value>& value){

	}

protected:
	bool callProty(unsigned long id, CefRefPtr<CefV8Value>& val){
		bool ret = false;
		ProtyMap::iterator it = m_map.find(id);
		if (it != m_map.end())
		{
			it->second(val);
			ret = true;
		}
		return ret;
	}

private:
	CefRefPtr<CefBrowser> browser_;
	ProtyMap m_map;
	IMPLEMENT_REFCOUNTING(Nativeapp);
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
		
		CefRefPtr<CefV8Value> window = context->GetGlobal();
		CefRefPtr<CefV8Accessor> myV8Acc = new Nativeapp(browser);
		char native[] = {"nativeapp"};
		CefRefPtr<CefV8Value> val = CefV8Value::CreateString(native);
		CefString cefException;
		//myV8Acc->Set("name", window, val, cefException);
		CefRefPtr<CefV8Value> pObjApp = CefV8Value::CreateObject(myV8Acc);
		window->SetValue(native, pObjApp, V8_PROPERTY_ATTRIBUTE_NONE);


		CefV8Value::PropertyAttribute attributes =
			static_cast<CefV8Value::PropertyAttribute>(
			V8_PROPERTY_ATTRIBUTE_READONLY |
			V8_PROPERTY_ATTRIBUTE_DONTENUM |
			V8_PROPERTY_ATTRIBUTE_DONTDELETE);

		CefRefPtr<CefV8Handler> myV8handle = new NativeappHandler(browser);

#define REG_JS_FUN(fnName) \
		static_cast<NativeappHandler*>(myV8handle.get())->RegisterFunction(#fnName, &NativeappHandler::##fnName); \
		pObjApp->SetValue(#fnName, CefV8Value::CreateFunction(#fnName, myV8handle), attributes);

		//注册给js调用的函数
		REG_JS_FUN(setWindowSize);
		REG_JS_FUN(minWindow);
		REG_JS_FUN(maxWindow);
		REG_JS_FUN(restoreWindow);
		REG_JS_FUN(closeWindow);
		REG_JS_FUN(setWindowText);
		REG_JS_FUN(fullScreen);
		REG_JS_FUN(createWindow);
		REG_JS_FUN(createModalWindow);
		REG_JS_FUN(createModalWindow2);
		REG_JS_FUN(setAlpha);
		REG_JS_FUN(invokeMethod);
		REG_JS_FUN(writePrivateProfileString);
		REG_JS_FUN(getPrivateProfileInt);
		REG_JS_FUN(getPrivateProfileString);
		REG_JS_FUN(setWindowPos);
		REG_JS_FUN(pushMessage);
		REG_JS_FUN(crossInvokeWebMethod);
		REG_JS_FUN(crossInvokeWebMethod2);
		REG_JS_FUN(winProty);
		REG_JS_FUN(setProfile);
		REG_JS_FUN(getProfile);
		REG_JS_FUN(getSoftwareAttribute);
		REG_JS_FUN(addFrameStateChanged);
		REG_JS_FUN(removeFrameStateChanged);

#undef REG_JS_FUN
		/*const char proty[][16] = { "appname", "appDir", "appDataPath", "screen_w", "screen_h", "desktop_w", "desktop_h",
									"window_x", "window_y", "window_w", "window_h", "is_zoomed", "is_iconic"};
		int len = sizeof(proty) / sizeof(proty[0]);
		for (int i = 0; i < len; ++i)
		{
			pObjApp->SetValue(proty[i], V8_ACCESS_CONTROL_DEFAULT, attributes);
		}

		static_cast<Nativeapp*>(myV8Acc.get())->RegisterFunction("appname", &Nativeapp::appname);*/

#define REG_JS_PROTY(proyName) \
		static_cast<Nativeapp*>(myV8Acc.get())->RegisterFunction(#proyName, &Nativeapp::##proyName); \
		pObjApp->SetValue(#proyName, V8_ACCESS_CONTROL_DEFAULT, attributes);

		REG_JS_PROTY(appname);
		REG_JS_PROTY(appDir);
		REG_JS_PROTY(appDataPath);
		REG_JS_PROTY(screen_w);
		REG_JS_PROTY(screen_h);
		REG_JS_PROTY(desktop_w);
		REG_JS_PROTY(desktop_h);
		REG_JS_PROTY(window_x);
		REG_JS_PROTY(window_y);
		REG_JS_PROTY(window_w);
		REG_JS_PROTY(window_h);
		REG_JS_PROTY(is_zoomed);
		REG_JS_PROTY(is_iconic);

#undef REG_JS_PROTY
		
		//CefRefPtr<CefV8Value> myFun1 = CefV8Value::create
		
		//pObjApp->SetValue(proty, myFun1, attributes);


		//DCHECK_EQ(context->GetGlobal()->SetValue(myfuname, myFun, attributes), true);
		//context->GetGlobal()->SetValue(myfuname, myFun, attributes);
	}

		//下面三个函数集中处里从主线程发送的消息
		///

	virtual bool OnProcessMessageReceived2(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message, CefRefPtr<CefListValue> response, bool& response_ack)  OVERRIDE
	{
		//test code
		//CefString name = message->GetName();
		//CefString pp = message->GetArgumentList()->GetString(0);
		//CefRefPtr<CefFrame> frame = browser->GetMainFrame();
		//frame->ExecuteJavaScript( CefString(L"pprr('454xx')"), CefString(L""), 0);
		//test code end
		return BridageHost::getInst().ProcRequest(app, browser, message, response, response_ack);		
	}

		virtual bool OnProcessResponseReceived(
		CefRefPtr<ClientApp> app,
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		int request_id,
		bool succ,
		CefRefPtr<CefListValue> response)OVERRIDE{

		return BridageHost::getInst().ProcResponse(browser, request_id, succ, response);
	}

	virtual bool OnProcessResponseAckReceived(
		CefRefPtr<ClientApp> app,
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		int request_id)OVERRIDE{

		int i = 0;
		return false;
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

