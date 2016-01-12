#ifndef _bridage_h_
#define _bridage_h_
#pragma once
#include "include/base/cef_lock.h"
#include <include/cef_process_message.h>
#include <include/cef_browser.h>

#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "client_app.h"

class RequestContext : public CefBase
{
public:
	CefRefPtr<CefListValue> m_val;
	HANDLE m_hAck;
	RequestContext()
	{
		m_hAck = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	~RequestContext()
	{
		CloseHandle(m_hAck);
	}

	IMPLEMENT_REFCOUNTING(RequestContext);
};

typedef std::map<int, CefRefPtr<RequestContext>> REQUEST_MAP;

//定义响应回调指针
typedef boost::function<bool(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>, bool&)> ResponseCallback;
typedef std::map<unsigned int, ResponseCallback> ResponseMap;

#define REGISTER_RESPONSE_ACK_FUNCTION(class_name, function_name) \
	this->RegisterResponseFunction( #function_name , &class_name::##function_name , this )

#define REQ_TIME_OUT 5000

class Bridage
{
public:
	Bridage();
	virtual ~Bridage();		
	//static Bridage& getInstance();
	virtual bool SendRequest(CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>&, int timeout = REQ_TIME_OUT) = 0;
	virtual bool ProcRequest(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>, bool&);
	virtual bool ProcResponse(CefRefPtr<CefBrowser> browser, int request_id, bool succ, CefRefPtr<CefListValue>& response);

protected:	
	//static Bridage s_bridage;
	//Bridage();
	const int generateID();

	bool _SendRequest(CefRefPtr<CefBrowser>, CefProcessId, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>&, int timeout = 0);
	
	//注册成员回调函数
	template<typename T>
	bool RegisterResponseFunction(const char* pFunctionName,
		bool(T::*function)(CefRefPtr<ClientApp>, CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>, CefRefPtr<CefListValue>, bool&),
		T* obj)
	{
		boost::hash<std::string> string_hash;
		return responseMap_.insert(std::make_pair(string_hash(pFunctionName),
			boost::bind(function, obj, _1, _2, _3, _4, _5))).second;
	}

	ResponseMap responseMap_;

private:
	int m_id;
	mutable base::Lock lock_;
	REQUEST_MAP request_;	
};

#endif

