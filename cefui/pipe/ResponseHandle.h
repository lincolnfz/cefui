#ifndef _respnosehandle_h
#define _respnosehandle_h
#pragma once

#include <map>
#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "Instruct.h"


typedef boost::function<bool(const int id, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>)> ResponseCb;
typedef std::map<unsigned int, ResponseCb> ResponseCbMap;
#define REGISTER_RESPONSE_FUNCTION(class_name, function_name) \
	this->RegisterResponseFunction( #function_name , &class_name::##function_name , this )

typedef boost::function<bool(const int id, const std::shared_ptr<cyjh::Instruct>)> AckCb;
typedef std::map<unsigned int, AckCb> AckCbMap;
#define REGISTER_ACK_FUNCTION(class_name, function_name) \
	this->RegisterAckFunction( #function_name , &class_name::##function_name , this )




namespace cyjh{

std::string x_funName(char* name);
#define PICK_MEMBER_FUN_NAME(x) x_funName(x).c_str()

class ResponseHandle
{
public:
	ResponseHandle();
	virtual ~ResponseHandle();
	bool handleQuest(const int id, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>);

protected:
	//注册成员回调函数
	template<typename T>
	bool RegisterResponseFunction(const char* pFunctionName,
		bool(T::*function)(const int id, const std::shared_ptr<cyjh::Instruct>, std::shared_ptr<cyjh::Instruct>),
		T* obj)
	{
		boost::hash<std::string> string_hash;
		return responseMap_.insert(std::make_pair(string_hash(pFunctionName),
			boost::bind(function, obj, _1, _2, _3))).second;
	}	

protected:
	ResponseCbMap responseMap_;
};

class AckHandle
{
public:
	AckHandle();
	virtual ~AckHandle();
	bool handleAck(const int id, const std::shared_ptr<cyjh::Instruct>);

protected:
	//注册成员回调函数
	template<typename T>
	bool RegisterAckFunction(const char* pFunctionName,
		bool(T::*function)(const int id, const std::shared_ptr<cyjh::Instruct>),
		T* obj)
	{
		boost::hash<std::string> string_hash;
		return ackMap_.insert(std::make_pair(string_hash(pFunctionName),
			boost::bind(function, obj, _1, _2))).second;
	}	

protected:
	AckCbMap ackMap_;
};

}
#endif