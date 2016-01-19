#ifndef _respnosehandle_h
#define _respnosehandle_h
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "CombinThreadComit.h"
#include "include/cef_browser.h"

typedef boost::function<bool(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>)> ResponseCb;
typedef std::map<unsigned int, ResponseCb> ResponseCbMap;
#define REGISTER_RESPONSE_FUNCTION(class_name, function_name) \
	this->RegisterResponseFunction( #function_name , &class_name::##function_name , this )

class ResponseHandle
{
public:
	ResponseHandle();
	virtual ~ResponseHandle();
	bool handleQuest(const CefRefPtr<CefBrowser>, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);

protected:
	//注册成员回调函数
	template<typename T>
	bool RegisterResponseFunction(const char* pFunctionName,
		bool(T::*function)(const CefRefPtr<CefBrowser>, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>),
		T* obj)
	{
		boost::hash<std::string> string_hash;
		return responseMap_.insert(std::make_pair(string_hash(pFunctionName),
			boost::bind(function, obj, _1, _2, _3))).second;
	}	

protected:
	ResponseCbMap responseMap_;
};

#endif