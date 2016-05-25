#include "stdafx.h"
#include "ResponseHandle.h"
#include <regex>

namespace cyjh{

	std::string x_funName(char* name){
		std::string out;
		std::string mathstr(name);
		std::tr1::regex pattern("::\\S+");
		std::smatch result;
		bool match = std::regex_search(mathstr, result, pattern);
		if (match){
			std::ssub_match base_sub_match = result[0];
			std::string temp = base_sub_match.str();
			std::tr1::regex pattern2("[^:+]\\S+");
			std::smatch result2;
			match = std::regex_search(temp, result2, pattern2);
			if (match){
				std::ssub_match base_sub_match = result2[0];
				out = base_sub_match.str();
			}
		}
		return out;
	}

ResponseHandle::ResponseHandle()
{
}


ResponseHandle::~ResponseHandle()
{
}

bool ResponseHandle::handleQuest(const int id, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out_val)
{
	bool bRet = false;
	boost::hash<std::string> string_hash;
	std::string name("rsp_");
	name.append(req_parm->getName());
	size_t key = string_hash(name);
	ResponseCbMap::iterator it = responseMap_.find(key);
	if (it != responseMap_.end())
	{
		//it->second(app, browser, msg, response, responseAck);
		ResponseCb cbFun = it->second;
		if (cbFun)
		{
			bRet = cbFun(id, req_parm, out_val);
		}
	}
	else{
		assert(false);
	}
	return bRet;
}

//////////////////////////////////////////////////////////////////////////

AckHandle::AckHandle()
{
}


AckHandle::~AckHandle()
{
}

bool AckHandle::handleAck(const int id, const std::shared_ptr<cyjh::Instruct> ack_parm)
{
	bool bRet = false;
	boost::hash<std::string> string_hash;
	std::string name("ack_");
	name.append(ack_parm->getName());
	size_t key = string_hash(name);
	AckCbMap::iterator it = ackMap_.find(key);
	if (it != ackMap_.end())
	{
		//it->second(app, browser, msg, response, responseAck);
		AckCb cbFun = it->second;
		if (cbFun)
		{
			bRet = cbFun(id, ack_parm);
		}
	}
	else{
		//assert(false);
	}
	return bRet;
}

}