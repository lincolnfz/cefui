// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "resource.h"
#include "client_app.h"
#include "client_renderer.h"
//#include "performance_test.h"
#include "scheme_test.h"
#include "BridageHost.h"

#include <regex>
#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#if defined(OS_LINUX)
#include "cefclient/print_handler_gtk.h"
#endif
#include <include/base/cef_logging.h>
#include "IPC.h"
#include <string>
#include <shlobj.h> 
#include "json/json.h"
#include "ResponseRender.h"
#include "ShareHelper.h"
#include "BrowserIdentifier.h"
#include "globalTools.h"

#define BROWSER_UI 0x01 
#define BROWSER_WEB 0x02
#define BROWSER_ALL (BROWSER_UI|BROWSER_WEB)

// static
void ClientApp::CreateBrowserDelegates(BrowserDelegateSet& delegates) {
}

//定义JS回调函数指针
typedef boost::function<void(const CefV8ValueList&, CefRefPtr<CefV8Value>&)> HandleCallback;
typedef std::map<unsigned long, HandleCallback> FunctionMap;

//定义JS属性函数指针
typedef boost::function<void(CefRefPtr<CefV8Value>&)> ProtyCallback;
typedef std::map<unsigned long, ProtyCallback> ProtyMap;

struct DectetFrameID 
{
	unsigned int browserID_;
	std::string frameID_;
	unsigned int dectID_;
	int64 identifier_;
	int lastHttpCode_;
};

struct HttpCode
{
	int64 frameID_;
	int lastHttpCode_;
	HttpCode(int64 frameID, int lastHttpCode){
		frameID_ = frameID;
		lastHttpCode_ = lastHttpCode;
	}
};

class DectetFrameLoad
{
public:
	static DectetFrameLoad& getInst(){
		return s_inst;
	}
	virtual ~DectetFrameLoad(){}

	bool Add(unsigned int browser, std::string& frame, unsigned int id, int64 identifier){
		bool ret = false;
		if (hit(browser, frame, id, 200) == false){
			DectetFrameID dectItem;
			dectItem.browserID_ = browser;
			dectItem.frameID_ = frame;
			dectItem.dectID_ = id;
			dectItem.identifier_ = identifier;
			dectItem.lastHttpCode_ = 200; //默认是200,如果事件没有被触发,页面加载停止发200
			m_dectList.push_back(dectItem);
			ret = true;
		}
		return ret;
	}

	bool Remove(unsigned int browser, std::string& frame, unsigned int id){
		bool ret = false;
		std::list<DectetFrameID>::iterator it = m_dectList.begin();
		for (; it != m_dectList.end(); ++it){
			if (it->browserID_ == browser && it->frameID_.compare(frame) == 0 && it->dectID_ == id){
				m_dectList.erase(it);
				ret = true;
				break;
			}
		}
		return ret;
	}

	bool hit(unsigned int browser, std::string& frame, unsigned int id, int httpCode){
		bool ret = false;
		std::list<DectetFrameID>::iterator it = m_dectList.begin();
		for (; it != m_dectList.end(); ++it)
		{
			if (it->browserID_ == browser && it->frameID_.compare(frame) == 0 && it->dectID_ == id){
				it->lastHttpCode_ = httpCode;
				ret = true;
				break;
			}
		}
		return ret;
	}

	void getNotifyLoadStop(unsigned int browser, std::vector<HttpCode>& frameIDs){
		std::list<DectetFrameID>::iterator it = m_dectList.begin();
		for (; it != m_dectList.end(); ++it)
		{
			if ( it->browserID_ == browser )
			{
				HttpCode item(it->identifier_, it->lastHttpCode_);
				frameIDs.push_back(item);
			}
		}
	}
protected:
	DectetFrameLoad(){}
	static DectetFrameLoad s_inst;

private:
	std::list<DectetFrameID> m_dectList;
};

DectetFrameLoad DectetFrameLoad::s_inst;

std::string getFramePath(CefRefPtr<CefFrame>& frame){
	std::string path;
	if ( frame.get() )
	{
		CefRefPtr<CefFrame> vist = frame;
		while ( vist.get() && !vist->IsMain() )
		{
			char buf[32] = {0};
			_itoa_s(vist->GetIdentifier(), buf, 10);
			path.append(buf);
			path.append(",");;
			vist = vist->GetParent();
		}
		if (vist.get() && vist->IsMain())
		{
			char buf[32] = { 0 };
			_itoa_s(vist->GetIdentifier(), buf, 10);
			path.append(buf);
		}
			 
	}	
	return path;
}

struct OriginFrameName 
{
	int _browserID;
	int64 _frameID;
	std::string _name;
};

class RecordFrameName
{
public:
	static RecordFrameName& getInst(){
		return s_inst_record_name;
	}	
	~RecordFrameName(){}

	bool SaveRecord(const int browser, const int64 frame, const char* name){
		bool find = false;
		std::list<OriginFrameName>::iterator it = _record_name_list.begin();
		for (; it != _record_name_list.end(); ++it)
		{
			if ( it->_browserID == browser && it->_frameID == frame )
			{
				find = true;
				break;
			}
		}
		if ( !find )
		{
			OriginFrameName item;
			item._browserID = browser;
			item._frameID = frame;
			item._name = name;
			_record_name_list.push_back(item);
		}
		return !find;
	}

	std::string GetRecord(const int browser, const int64 frame){
		std::string val;
		std::list<OriginFrameName>::iterator it = _record_name_list.begin();
		for (; it != _record_name_list.end(); ++it)
		{
			if (it->_browserID == browser && it->_frameID == frame)
			{
				val = it->_name;
			}
		}
		return val;
	}

protected:
	RecordFrameName(){}
	static RecordFrameName s_inst_record_name;

private:
	std::list<OriginFrameName> _record_name_list;	
};
RecordFrameName RecordFrameName::s_inst_record_name;

//这是一个响应js的c++ 函数(类)
class NativeappHandler : public CefV8Handler {
public:
	NativeappHandler(CefRefPtr<CefBrowser>& browser, CefRefPtr<CefFrame>& frame) :browser_(browser), frame_(frame){
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
		int x = list[0]->GetIntValue();
		int y = list[1]->GetIntValue();
		int width = list[2]->GetIntValue();
		int height = list[3]->GetIntValue();
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		parm.getList().AppendVal(x);
		parm.getList().AppendVal(y);
		parm.getList().AppendVal(width);
		parm.getList().AppendVal(height);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void minWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		bool ret = false;
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void maxWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		bool ret = false;
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void restoreWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		bool ret = false;
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void closeWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		//std::shared_ptr<cyjh::Instruct> outVal;
		//ipc->Request(this->browser_, parm, outVal);
		ipc->AsyncRequest(this->browser_, parm);
	}

	void setWindowText(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		parm.getList().AppendVal(list[0]->GetStringValue().ToWString());
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		//std::shared_ptr<cyjh::Instruct> outVal;
		//ipc->Request(this->browser_, parm, outVal);
		ipc->AsyncRequest(this->browser_, parm);
	}

	void fullScreen(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		bool full = list[0]->GetBoolValue();
		parm.getList().AppendVal(full);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void createWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::string strParm = list[0]->GetStringValue().ToString();
		parm.getList().AppendVal(strParm);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void createModalWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::string strParm = list[0]->GetStringValue().ToString();
		parm.getList().AppendVal(strParm);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void createModalWindow2(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::string strParm = list[0]->GetStringValue().ToString();
		parm.getList().AppendVal(strParm);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	/*void createWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		std::string strParm = list[0]->GetStringValue().ToString();
		CefRefPtr<CefProcessMessage> message =
			CefProcessMessage::Create( CefString(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__)));
		message->GetArgumentList()->SetString(0, CefString(strParm));
		CefRefPtr<CefListValue> response;
		BridageHost::getInst().SendRequest(browser_, message, response, 0);
		val = CefV8Value::CreateInt(1);
	}

	void createModalWindow(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		std::string strParm = list[0]->GetStringValue().ToString();
		CefRefPtr<CefProcessMessage> message =
			CefProcessMessage::Create(CefString(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__)));
		message->GetArgumentList()->SetString(0, CefString(strParm));
		CefRefPtr<CefListValue> response;
		BridageHost::getInst().SendRequest(browser_, message, response, 0);
		val = CefV8Value::CreateInt(1);
	}

	void createModalWindow2(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		std::string strParm = list[0]->GetStringValue().ToString();
		CefRefPtr<CefProcessMessage> message =
			CefProcessMessage::Create(CefString(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__)));
		message->GetArgumentList()->SetString(0, CefString(strParm));
		CefRefPtr<CefListValue> response;
		BridageHost::getInst().SendRequest(browser_, message, response, 0);
		val = CefV8Value::CreateInt(1);
	}*/

	void setAlpha(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		int alpah = list[0]->GetIntValue();
		parm.getList().AppendVal(alpah);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void invokeMethod(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring modulename = list[0]->GetStringValue().ToWString();
		std::wstring methodname = list[1]->GetStringValue().ToWString();
		std::wstring strparm = list[2]->GetStringValue().ToWString();
		if (methodname.compare(L"getCurrentSkinName") == 0 && modulename.compare(L"CUI") == 0 )
		{
			//过滤getCurrentSkinName，用来解决4399网站使用这个方法引起ui进程卡死
			val = CefV8Value::CreateString(CefString(L"temp"));
			//OutputDebugStringW(L"-----[ render getCurrentSkinName");
			return;
		}
		int extra = list[3]->GetIntValue();
		parm.getList().AppendVal(modulename);
		parm.getList().AppendVal(methodname);
		parm.getList().AppendVal(strparm);
		parm.getList().AppendVal(extra);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateString(CefString(outVal->getList().GetWStrVal(0)));
		}
		else{
			val = CefV8Value::CreateString(CefString(L""));
		}
	}

	void asyncCallMethod(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring modulename = list[0]->GetStringValue().ToWString();
		std::wstring methodname = list[1]->GetStringValue().ToWString();
		std::wstring strparm = list[2]->GetStringValue().ToWString();
		int extra = list[3]->GetIntValue();
		parm.getList().AppendVal(modulename);
		parm.getList().AppendVal(methodname);
		parm.getList().AppendVal(strparm);
		parm.getList().AppendVal(extra);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		ipc->AsyncRequest(this->browser_, parm);
	}

	void writePrivateProfileString(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring appName = list[0]->GetStringValue().ToWString();
		std::wstring keyName = list[1]->GetStringValue().ToWString();
		std::wstring strval = list[2]->GetStringValue().ToWString();
		std::wstring file = list[3]->GetStringValue().ToWString();
		parm.getList().AppendVal(appName);
		parm.getList().AppendVal(keyName);
		parm.getList().AppendVal(strval);
		parm.getList().AppendVal(file);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void getPrivateProfileInt(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring appName = list[0]->GetStringValue().ToWString();
		std::wstring keyName = list[1]->GetStringValue().ToWString();
		int defVal = list[2]->GetIntValue();
		std::wstring file = list[3]->GetStringValue().ToWString();
		parm.getList().AppendVal(appName);
		parm.getList().AppendVal(keyName);
		parm.getList().AppendVal(defVal);
		parm.getList().AppendVal(file);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		bool ret = false;
		if (outVal.get() && outVal->getSucc())
		{
			int retVal = outVal->getList().GetIntVal(0);
			val = CefV8Value::CreateInt (retVal);
		}
		else{
			val = CefV8Value::CreateInt(defVal);
		}
	}

	/*void getPrivateProfileString(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){

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
	}*/

	void getPrivateProfileString(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val)
	{
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring appName = list[0]->GetStringValue().ToWString();
		std::wstring keyName = list[1]->GetStringValue().ToWString();
		std::wstring defVal = list[2]->GetStringValue().ToWString();
		std::wstring file = list[3]->GetStringValue().ToWString();
		parm.getList().AppendVal(appName);
		parm.getList().AppendVal(keyName);
		parm.getList().AppendVal(defVal);
		parm.getList().AppendVal(file);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			std::wstring retVal = outVal->getList().GetWStrVal(0);
			val = CefV8Value::CreateString(CefString(retVal));
		}
		else{
			val = CefV8Value::CreateString(CefString(defVal));
		}		
	}

	void setWindowPos(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		int order = list[0]->GetIntValue();
		int x = list[1]->GetIntValue();
		int y = list[2]->GetIntValue();
		int width = list[3]->GetIntValue();
		int height = list[4]->GetIntValue();
		int flag = list[5]->GetIntValue();
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		parm.getList().AppendVal(order);
		parm.getList().AppendVal(x);
		parm.getList().AppendVal(y);
		parm.getList().AppendVal(width);
		parm.getList().AppendVal(height);
		parm.getList().AppendVal(flag);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void pushMessage(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		//no impl
	}

	void crossInvokeWebMethod(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		int sign = list[0]->GetIntValue();
		std::wstring modulename = list[1]->GetStringValue().ToWString();
		std::wstring methodname = list[2]->GetStringValue().ToWString();
		std::wstring strparm = list[3]->GetStringValue().ToWString();
		bool bjson = list[4]->GetBoolValue();
		parm.getList().AppendVal(sign);
		parm.getList().AppendVal(modulename);
		parm.getList().AppendVal(methodname);
		parm.getList().AppendVal(strparm);
		parm.getList().AppendVal(bjson);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateString(CefString(outVal->getList().GetWStrVal(0)));
		}
		else{
			val = CefV8Value::CreateString(CefString(L""));
		}
	}

	void crossInvokeWebMethod2(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		int sign = list[0]->GetIntValue();
		std::wstring framename = list[1]->GetStringValue().ToWString();
		std::wstring modulename = list[2]->GetStringValue().ToWString();
		std::wstring methodname = list[3]->GetStringValue().ToWString();
		std::wstring strparm = list[4]->GetStringValue().ToWString();
		bool bjson = list[5]->GetBoolValue();
		parm.getList().AppendVal(sign);
		parm.getList().AppendVal(framename);
		parm.getList().AppendVal(modulename);
		parm.getList().AppendVal(methodname);
		parm.getList().AppendVal(strparm);
		parm.getList().AppendVal(bjson);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			val = CefV8Value::CreateString(CefString(outVal->getList().GetWStrVal(0)));
		}
		else{
			val = CefV8Value::CreateString(CefString(L""));
		}
	}

	void asyncCrossInvokeWebMethod(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		int sign = list[0]->GetIntValue();
		std::wstring modulename = list[1]->GetStringValue().ToWString();
		std::wstring methodname = list[2]->GetStringValue().ToWString();
		std::wstring strparm = list[3]->GetStringValue().ToWString();
		bool bjson = list[4]->GetBoolValue();
		parm.getList().AppendVal(sign);
		parm.getList().AppendVal(modulename);
		parm.getList().AppendVal(methodname);
		parm.getList().AppendVal(strparm);
		parm.getList().AppendVal(bjson);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		ipc->AsyncRequest(this->browser_, parm);
	}

	void asyncCrossInvokeWebMethod2(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		int sign = list[0]->GetIntValue();
		std::wstring framename = list[1]->GetStringValue().ToWString();
		std::wstring modulename = list[2]->GetStringValue().ToWString();
		std::wstring methodname = list[3]->GetStringValue().ToWString();
		std::wstring strparm = list[4]->GetStringValue().ToWString();
		bool bjson = list[5]->GetBoolValue();
		parm.getList().AppendVal(sign);
		parm.getList().AppendVal(framename);
		parm.getList().AppendVal(modulename);
		parm.getList().AppendVal(methodname);
		parm.getList().AppendVal(strparm);
		parm.getList().AppendVal(bjson);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		ipc->AsyncRequest(this->browser_, parm);
	}

	void winProty(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc()){
			std::wstring retVal = outVal->getList().GetWStrVal(0);
			val = CefV8Value::CreateString(CefString(retVal));
		}
		else{
			val = CefV8Value::CreateString(CefString(L""));
		}
	}

	void setProfile(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring keyName = list[0]->GetStringValue().ToWString();
		std::wstring strval = list[1]->GetStringValue().ToWString();
		parm.getList().AppendVal(keyName);
		parm.getList().AppendVal(strval);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc()){
			val = CefV8Value::CreateInt(1);
		}
		else{
			val = CefV8Value::CreateInt(0);
		}
	}

	void getProfile(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		std::wstring keyName = list[0]->GetStringValue().ToWString();
		parm.getList().AppendVal(keyName);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc()){
			std::wstring retVal = outVal->getList().GetWStrVal(0);
			val = CefV8Value::CreateString(CefString(retVal));
		}
		else{
			val = CefV8Value::CreateString(CefString(L""));
		}
	}

	void getSoftwareAttribute(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		cyjh::Instruct parm;
		int attIdx = list[0]->GetIntValue();
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		parm.getList().AppendVal(attIdx);
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc()){
			std::wstring retVal = outVal->getList().GetWStrVal(0);
			val = CefV8Value::CreateString(CefString(retVal));
		}
		else{
			val = CefV8Value::CreateString(CefString(L""));
		}
	}

	void addFrameStateChanged(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		std::string id = list[0]->GetStringValue().ToString();
		boost::hash<std::string> string_hash;
		unsigned int uid = string_hash(id);
		bool bAdd = DectetFrameLoad::getInst().Add(browser_->GetIdentifier(), getFramePath(frame_), uid, frame_->GetIdentifier());
		val = CefV8Value::CreateInt ( bAdd ? 1 : 0 );
	}

	void removeFrameStateChanged(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		std::string id = list[0]->GetStringValue().ToString();
		boost::hash<std::string> string_hash;
		unsigned int uid = string_hash(id);
		bool bAdd = DectetFrameLoad::getInst().Remove(browser_->GetIdentifier(), getFramePath(frame_), uid);
		val = CefV8Value::CreateInt(bAdd ? 1 : 0);
	}

	void queryProduct(const CefV8ValueList& list, CefRefPtr<CefV8Value>& val){
		val = CefV8Value::CreateString(CefString(L"cyjh"));
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
	CefRefPtr<CefFrame> frame_;
	FunctionMap	m_FunctionMap;

	IMPLEMENT_REFCOUNTING(NativeappHandler);
};

class MyExternal : public CefV8Accessor {
public:
	MyExternal(CefRefPtr<CefBrowser>& browser, CefRefPtr<CefFrame>& frame) :browser_(browser), frame_(frame){}
	virtual bool Get(const CefString& name,
		const CefRefPtr<CefV8Value> object,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE
	{
		return false;
	}
		virtual bool Set(const CefString& name,
		const CefRefPtr<CefV8Value> object,
		const CefRefPtr<CefV8Value> value,
		CefString& exception) OVERRIDE
	{
		return true;
	}

private:
	CefRefPtr<CefBrowser> browser_;
	CefRefPtr<CefFrame> frame_;
	ProtyMap m_map;
	IMPLEMENT_REFCOUNTING(MyExternal);
};

class Nativeapp : public CefV8Accessor {
public:
	Nativeapp(CefRefPtr<CefBrowser>& browser, CefRefPtr<CefFrame>& frame) :browser_(browser), frame_(frame){}
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
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			std::wstring val = outVal->getList().GetWStrVal(0);
			value = CefV8Value::CreateString(CefString(val));
		}
		else{
			value = CefV8Value::CreateString(CefString(L""));
		}
	}

	void appDir(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			std::wstring val = outVal->getList().GetWStrVal(0);
			value = CefV8Value::CreateString(CefString(val));
		}
		else{
			value = CefV8Value::CreateString(CefString(L""));
		}
	}

	void appDataPath(CefRefPtr<CefV8Value>& value){
		WCHAR szAppData[MAX_PATH];
		SHGetSpecialFolderPathW(NULL, szAppData, CSIDL_APPDATA, TRUE);
		wcscat_s(szAppData, L"\\");

		//为兼容旧版本,不在后面加上程序目录
		/*cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			std::wstring val = outVal->getList().GetWStrVal(0);
			value = CefV8Value::CreateString(CefString(val));
		}
		else{
			value = CefV8Value::CreateString(CefString(szAppData));
		}*/

		value = CefV8Value::CreateString(CefString(szAppData));
	}

	void screen_w(CefRefPtr<CefV8Value>& value){
		int val = GetSystemMetrics(SM_CXSCREEN);
		value = CefV8Value::CreateInt(val);
	}

	void screen_h(CefRefPtr<CefV8Value>& value){
		int val = GetSystemMetrics(SM_CYSCREEN);
		value = CefV8Value::CreateInt(val);
	}

	void desktop_w(CefRefPtr<CefV8Value>& value){
		int val = GetSystemMetrics(SM_CXFULLSCREEN);
		value = CefV8Value::CreateInt(val);
	}

	void desktop_h(CefRefPtr<CefV8Value>& value){
		int val = GetSystemMetrics(SM_CYFULLSCREEN);
		value = CefV8Value::CreateInt(val);
	}

	void window_x(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			value = CefV8Value::CreateInt(outVal->getList().GetIntVal(0));
		}
		else{
			value = CefV8Value::CreateInt(0);
		}
		
	}

	void window_y(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if ( outVal.get() && outVal->getSucc() )
		{
			value = CefV8Value::CreateInt(outVal->getList().GetIntVal(0));
		}
		else{
			value = CefV8Value::CreateInt(0);
		}
	}

	void window_w(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			value = CefV8Value::CreateInt(outVal->getList().GetIntVal(0));
		}
		else{
			value = CefV8Value::CreateInt(0);
		}
	}

	void window_h(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			value = CefV8Value::CreateInt(outVal->getList().GetIntVal(0));
		}
		else{
			value = CefV8Value::CreateInt(0);
		}
	}

	void is_zoomed(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			value = CefV8Value::CreateInt(outVal->getList().GetIntVal(0));
		}
		else{
			value = CefV8Value::CreateInt(0);
		}
	}

	void is_iconic(CefRefPtr<CefV8Value>& value){
		cyjh::Instruct parm;
		parm.setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(this->browser_, parm, outVal);
		if (outVal.get() && outVal->getSucc())
		{
			value = CefV8Value::CreateInt(outVal->getList().GetIntVal(0));
		}
		else{
			value = CefV8Value::CreateInt(0);
		}
	}

	void ver(CefRefPtr<CefV8Value>& value){
		value = CefV8Value::CreateInt(1001);
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
	CefRefPtr<CefFrame> frame_;
	ProtyMap m_map;
	IMPLEMENT_REFCOUNTING(Nativeapp);
};

bool call_FrameStateChanged(CefRefPtr<CefFrame>& frame, const char* frameName, const char* url, const int& code, bool didComit)
{
	bool ret = false;
	Json::Value root;
	root["frameid"] = frameName;
	root["src"] = url;
	root["state"] = code;
	root["resloaded"] = didComit;
	Json::StyledWriter write;
	std::string strJson = write.write(root);

	boost::format fmt("_onFrameStateChanged(%1%)");
	fmt % strJson;
	std::string strJs = fmt.str();
	CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
	CefRefPtr<CefV8Value> retVal;
	CefRefPtr<CefV8Exception> excp;
	if ( v8.get() )
	{
		if (v8->Eval(CefString(strJs), retVal, excp)){
			ret = true;
		}
	}	
	return ret;
}

class MyLoaderHandler :public virtual CefLoadHandler
{
public:
	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
		bool isLoading,
		bool canGoBack,
		bool canGoForward) {

		if (BrowserIdentifier::GetInst().GetType(browser->GetIdentifier()) != 1){
			return;
		}
			if(!isLoading){
				CefRefPtr<CefFrame>frame = browser->GetMainFrame();
				std::vector<HttpCode> ids;
				DectetFrameLoad::getInst().getNotifyLoadStop(browser->GetIdentifier(), ids);
				std::vector<HttpCode>::iterator it = ids.begin();
				for (; it != ids.end(); ++it)
				{
					CefRefPtr<CefFrame>notify_frame = browser->GetFrame(it->frameID_);
					if ( notify_frame.get() )
					{
						CefRefPtr<CefFrame>parent = notify_frame->GetParent();
						if ( parent.get() )
						{
							std::string frameNam = notify_frame->GetName().ToString();
							std::string url = notify_frame->GetURL().ToString();
							//call_FrameStateChanged(parent, frameNam.c_str(), url.c_str(), it->lastHttpCode_, true);
						}
					}					
				}
			}
	}

	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame) {

		int browserType = BrowserIdentifier::GetInst().GetType(browser->GetIdentifier());
		if (browserType == BROWSER_UI)
		{
			//if (BrowserIdentifier::GetInst().GetType(browser->GetIdentifier()) != 1){
				//只注入主ui界面
			//	return;
			//}
			if (frame->IsMain())
				DocComplate::getInst().setBrowsr(browser->GetIdentifier(), false);

			CefRefPtr<CefFrame> parent = frame->GetParent();
			if (parent.get())
			{
				std::string frameNam = frame->GetName().ToString();
				if (!RecordFrameName::getInst().SaveRecord(browser->GetIdentifier(), frame->GetIdentifier(), frameNam.c_str())){
					frameNam = RecordFrameName::getInst().GetRecord(browser->GetIdentifier(), frame->GetIdentifier());
				}
				boost::hash<std::string> string_hash;
				unsigned int id = string_hash(frameNam);
				if (DectetFrameLoad::getInst().hit(browser->GetIdentifier(), getFramePath(parent), id, -10086)){
					std::string url = frame->GetURL().ToString();
					call_FrameStateChanged(parent, frameNam.c_str(), url.c_str(), -10086, false);
				}
			}
		}

	}

	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int httpStatusCode) {

		int browserType = BrowserIdentifier::GetInst().GetType(browser->GetIdentifier());
		if (browserType == BROWSER_UI)
		{
			//if (BrowserIdentifier::GetInst().GetType(browser->GetIdentifier()) != 1){
				//只注入主ui界面
			//	return;
			//}
			CefRefPtr<CefFrame> parent = frame->GetParent();
			if (parent.get())
			{
				boost::hash<std::string> string_hash;
				//std::string frameNam = frame->GetName().ToString();
				std::string frameNam = RecordFrameName::getInst().GetRecord(browser->GetIdentifier(), frame->GetIdentifier());
				unsigned int id = string_hash(frameNam);
				if (DectetFrameLoad::getInst().hit(browser->GetIdentifier(), getFramePath(parent), id, httpStatusCode)){
					std::string url = frame->GetURL().ToString();
					call_FrameStateChanged(parent, frameNam.c_str(), url.c_str(), httpStatusCode, true);
				}
			}
		}
	}

	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) {

		int browserType = BrowserIdentifier::GetInst().GetType(browser->GetIdentifier());
		if (browserType == BROWSER_UI)
		{
			//if (BrowserIdentifier::GetInst().GetType(browser->GetIdentifier()) != 1){
				//只注入主ui界面
			//	return;
			//}
			CefRefPtr<CefFrame> parent = frame->GetParent();
			if (parent.get())
			{
				boost::hash<std::string> string_hash;
				//std::string frameNam = frame->GetName().ToString();
				std::string frameNam = RecordFrameName::getInst().GetRecord(browser->GetIdentifier(), frame->GetIdentifier());
				unsigned int id = string_hash(frameNam);
				if (DectetFrameLoad::getInst().hit(browser->GetIdentifier(), getFramePath(parent), id, errorCode)){
					std::string url = frame->GetURL().ToString();
					call_FrameStateChanged(parent, frameNam.c_str(), url.c_str(), errorCode, false);
					call_FrameStateChanged(parent, frameNam.c_str(), url.c_str(), errorCode, true);
				}
			}
		}
	}

	virtual void OnDocumentLoadedInFrame(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame, int httpStatusCode){

		int browserType = BrowserIdentifier::GetInst().GetType(browser->GetIdentifier());
		if (browserType == BROWSER_UI)
		{
			//if (BrowserIdentifier::GetInst().GetType(browser->GetIdentifier()) != 1){
				//只注入主ui界面
			//	return;
			//}

			if (frame->IsMain())
				DocComplate::getInst().setBrowsr(browser->GetIdentifier(), true);

			CefRefPtr<CefFrame> parent = frame->GetParent();
			if (parent.get())
			{
				boost::hash<std::string> string_hash;
				//std::string frameNam = frame->GetName().ToString();
				std::string frameNam = RecordFrameName::getInst().GetRecord(browser->GetIdentifier(), frame->GetIdentifier());
				unsigned int id = string_hash(frameNam);
				if (DectetFrameLoad::getInst().hit(browser->GetIdentifier(), getFramePath(parent), id, httpStatusCode)){
					std::string url = frame->GetURL().ToString();
					call_FrameStateChanged(parent, frameNam.c_str(), url.c_str(), httpStatusCode, false);
				}

				/*
				CefRefPtr<CefV8Value> retVal;
				CefRefPtr<CefV8Exception> excp;
				if (frame->GetV8Context().get() && frame->GetV8Context()->Eval(CefString(L"document.title"), retVal, excp)){
					if (retVal.get() && retVal.get()->IsString()){
						std::wstring title = retVal->GetStringValue().ToWString();
						std::wstring name = frame->GetName().ToWString();
						if (name.find(L"tabbar_f_") == 0)
						{
							boost::wformat val(L"{\"frameid\":\"%1%\", \"title\":\"%2%\"}");
							val % name % title;
							CefRefPtr<CefV8Value> retVal_tmp;
							CefRefPtr<CefV8Exception> excp_tmp;
							parent->GetV8Context()->CallInvokeMethod(CefString(L"invokeMethod"), CefString(L"TabBar"),
								CefString(L"TitleChanged"), CefString(std::wstring(val.str())), true, retVal_tmp, excp_tmp);
						}
					}
				}*/

				std::wstring name = frame->GetName().ToWString();
				if (name.find(L"tabbar_f_") == 0)
				{
					//inject js
#ifdef _DEBUG
					WCHAR dll_name[] = { L"wrapcef_d.dll" };
#else
					WCHAR dll_name[] = { L"wrapcef.dll" };
#endif
					HRSRC   hrsc = FindResource(GetModuleHandle(dll_name), MAKEINTRESOURCE(IDR_INJECT_JS), _T("TXT"));
					HGLOBAL hG = LoadResource(GetModuleHandle(dll_name), hrsc);
					char* ptr = (char*)LockResource(hG);
					std::wstring inject_js;
					if ( ptr )
					{
						inject_js = Utf82Unicode(std::string(ptr));
						UnlockResource(hrsc);
					}
					if ( !inject_js.empty() )
					{
						inject_js.append(L";document.title");
					}
					else{
						inject_js = L"document.title";
					}

					CefRefPtr<CefV8Value> retVal;
					CefRefPtr<CefV8Exception> excp;
					if (frame->GetV8Context().get() && frame->GetV8Context()->Eval(CefString(inject_js.c_str()), retVal, excp)){
						if (retVal.get() && retVal.get()->IsString()){
							std::wstring title = retVal->GetStringValue().ToWString();
							boost::wformat val(L"{\"frameid\":\"%1%\", \"title\":\"%2%\"}");
							val % name % title;
							CefRefPtr<CefV8Value> retVal_tmp;
							CefRefPtr<CefV8Exception> excp_tmp;
							parent->GetV8Context()->CallInvokeMethod(CefString(L"invokeMethod"), CefString(L"TabBar"),
								CefString(L"TitleChanged"), CefString(std::wstring(val.str())), true, retVal_tmp, excp_tmp);
						}
					}
				}//"tabbar_f_"
			}
		}

		if (browserType == BROWSER_WEB){
			if (frame->IsMain()){
				std::wstring shout_icon;
				CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
				if (v8.get())
				{
					//获取网站图标
					std::string strJS = "document.head.innerHTML";
					CefRefPtr<CefV8Value> retVal;
					CefRefPtr<CefV8Exception> excp;
					v8->Eval(CefString(strJS.c_str()), retVal, excp);
					std::wstring html = retVal->GetStringValue().ToWString();

					WCHAR szPat[] = L"<link\\s*\\S*\\s*rel=\"\\s*\\S*\\s*icon\\s*\\S*\\s*\"\\s*\\S*[\\s*$]";
					WCHAR szHref[] = L"href=\"\\s*\\S+\\s*\"";
					WCHAR szVal[] = L"\"\\s*\\S+\\s*\"";
					std::tr1::wregex pattern(szPat, std::regex::icase);
					std::wsmatch result;
					if (std::regex_search(html, result, pattern)){
						std::wssub_match base_sub_match = result[0];
						std::wstring temp = base_sub_match.str();
						std::tr1::wregex pattern2(szHref, std::regex::icase);
						std::wsmatch result2;
						if ( std::regex_search(temp, result2, pattern2) )
						{
							std::wssub_match base_sub_match = result2[0];
							std::wstring temp3 = base_sub_match.str();
							std::tr1::wregex pattern3(szVal, std::regex::icase);
							std::wsmatch result3;
							if (std::regex_search(temp3, result3, pattern3))
							{
								std::wssub_match base_sub_match = result3[0];
								shout_icon = base_sub_match.str();
								if ( shout_icon.size() > 2 )
								{
									shout_icon.erase(shout_icon.size()-1);
									shout_icon.erase(shout_icon.begin());
								}
								else{
									shout_icon.clear();
								}
							}
						}
					}
					
				}
				cyjh::Instruct parm;
				parm.setName("siteicon");
				CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
				//ipc->AsyncRequest(browser, parm);
			}
		}

		/*
		//get inject js
		std::wstring name = frame->GetName().ToWString();
		std::wstring url = frame->GetURL().ToWString();
		std::wstring mainurl;
		if (browser->GetMainFrame().get()){
			mainurl = browser->GetMainFrame()->GetURL().ToWString();
		}
#ifdef _DEBUG1
		WCHAR szBuf[2048] = { 0 };
		wsprintf(szBuf, L"-----[1 inject_js name: %s url: %s", name.c_str(), url.c_str());
		OutputDebugStringW(szBuf);
#endif
		cyjh::Instruct parm;
		parm.setName("getInjectJS");
		parm.getList().AppendVal(url);
		parm.getList().AppendVal(name);
		parm.getList().AppendVal(mainurl);
		parm.getList().AppendVal(frame->GetIdentifier());

		CefRefPtr<cyjh::RenderThreadCombin> ipc = ClientApp::getGlobalApp()->getRenderThreadCombin();
		ipc->AsyncRequest(browser, parm);//异步请求注入
		*/
		
		/*
		//同步请求注入,如果要在OnDocumentLoadedInFrame注入js，不建议使用同步
		std::shared_ptr<cyjh::Instruct> outVal;
		ipc->Request(browser, parm, outVal);		
		if (outVal.get() && outVal->getSucc())
		{
			std::wstring js = outVal->getList().GetWStrVal(0);
			//std::wstring js = L"alert('00');";
			if (!js.empty())
			{
#ifdef _DEBUG1
				WCHAR szBuf[2048] = { 0 };
				wnsprintf(szBuf, 2040, L"-----[1 inject_js js: %s", js.c_str());
				OutputDebugStringW(szBuf);
#endif
				CefString cefjs(outVal->getList().GetWStrVal(0));
				CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
				CefRefPtr<CefV8Value> retVal;
				CefRefPtr<CefV8Exception> excp;
				v8->Eval(cefjs, retVal, excp);
			}
		}
		*/
	}
protected:
private:
	IMPLEMENT_REFCOUNTING(MyLoaderHandler);
};

// Handle bindings in the render process.
class MyRenderDelegate : public ClientApp::RenderDelegate {
public:
	MyRenderDelegate() :ClientApp::RenderDelegate(new MyLoaderHandler){
	}

	virtual void OnContextCreated(CefRefPtr<ClientApp> app,
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context) OVERRIDE{

		int browserType = BrowserIdentifier::GetInst().GetType(browser->GetIdentifier());
		
		CefRefPtr<CefV8Value> window = context->GetGlobal();
		CefRefPtr<CefV8Accessor> myV8Acc = new Nativeapp(browser, frame);
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

		CefRefPtr<CefV8Handler> myV8handle = new NativeappHandler(browser, frame);

		bool testSetVal = false;
#define REG_JS_FUN(fnName, comp) \
		if( comp & browserType ){ \
		static_cast<NativeappHandler*>(myV8handle.get())->RegisterFunction(#fnName, &NativeappHandler::##fnName); \
		testSetVal = pObjApp->SetValue(#fnName, CefV8Value::CreateFunction(#fnName, myV8handle), attributes);\
		assert(testSetVal);\
		}

		//REG_JS_FUN comp参数使用组合模式 1.只注册ui， 2.只注册控件模式窗口, 3.两种窗口都注册
		//注册给js调用的函数
		REG_JS_FUN(setWindowSize, 1);
		REG_JS_FUN(minWindow, 1);
		REG_JS_FUN(maxWindow, 1);
		REG_JS_FUN(restoreWindow, 1);
		REG_JS_FUN(closeWindow, 1);
		REG_JS_FUN(setWindowText, 1);
		REG_JS_FUN(fullScreen, 1);
		REG_JS_FUN(createWindow, 1);
		REG_JS_FUN(createModalWindow, 1);
		REG_JS_FUN(createModalWindow2, 1);
		REG_JS_FUN(setAlpha, 1);
		REG_JS_FUN(invokeMethod, 3); 
		REG_JS_FUN(asyncCallMethod, 3);
		REG_JS_FUN(writePrivateProfileString, 1);
		REG_JS_FUN(getPrivateProfileInt, 1);
		REG_JS_FUN(getPrivateProfileString, 1);
		REG_JS_FUN(setWindowPos, 1);
		REG_JS_FUN(pushMessage, 1);
		REG_JS_FUN(crossInvokeWebMethod, 1);
		REG_JS_FUN(crossInvokeWebMethod2, 1);
		REG_JS_FUN(asyncCrossInvokeWebMethod, 1);
		REG_JS_FUN(asyncCrossInvokeWebMethod2, 1);
		REG_JS_FUN(winProty, 1);
		REG_JS_FUN(setProfile, 1);
		REG_JS_FUN(getProfile, 1);
		REG_JS_FUN(getSoftwareAttribute, 1);
		REG_JS_FUN(addFrameStateChanged, 1);
		REG_JS_FUN(removeFrameStateChanged, 1);
		REG_JS_FUN(queryProduct, 2);

#undef REG_JS_FUN
		/*const char proty[][16] = { "appname", "appDir", "appDataPath", "screen_w", "screen_h", "desktop_w", "desktop_h",
									"window_x", "window_y", "window_w", "window_h", "is_zoomed", "is_iconic"};
		int len = sizeof(proty) / sizeof(proty[0]);
		for (int i = 0; i < len; ++i)
		{
			pObjApp->SetValue(proty[i], V8_ACCESS_CONTROL_DEFAULT, attributes);
		}

		static_cast<Nativeapp*>(myV8Acc.get())->RegisterFunction("appname", &Nativeapp::appname);*/

		//属性只注册在ui
#define REG_JS_PROTY(proyName) \
		static_cast<Nativeapp*>(myV8Acc.get())->RegisterFunction(#proyName, &Nativeapp::##proyName); \
		pObjApp->SetValue(#proyName, V8_ACCESS_CONTROL_DEFAULT, attributes);

		if (browserType == 1){
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
			REG_JS_PROTY(ver);
		}

#undef REG_JS_PROTY
		
		//CefRefPtr<CefV8Value> myFun1 = CefV8Value::create
		
		//pObjApp->SetValue(proty, myFun1, attributes);


		//DCHECK_EQ(context->GetGlobal()->SetValue(myfuname, myFun, attributes), true);
		//context->GetGlobal()->SetValue(myfuname, myFun, attributes);

		//增加扩展external
		char external[] = { "external" };
		if (!window->HasValue(CefString(external))){
			CefRefPtr<CefV8Accessor> myExternal = new MyExternal(browser, frame);
			//CefRefPtr<CefV8Value> Val_ext = CefV8Value::CreateString(external);
			CefString cefException_ext;
			CefRefPtr<CefV8Value> pObjApp_ext = CefV8Value::CreateObject(myExternal);
			window->SetValue(external, pObjApp_ext, V8_PROPERTY_ATTRIBUTE_NONE);
		}
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

