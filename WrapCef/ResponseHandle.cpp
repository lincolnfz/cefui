#include "stdafx.h"
#include "ResponseHandle.h"


ResponseHandle::ResponseHandle()
{
}


ResponseHandle::~ResponseHandle()
{
}

bool ResponseHandle::handleQuest(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> out_val)
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
			bRet = cbFun(browser, req_parm, out_val);
		}
	}
	else{
		assert(false);
	}
	return bRet;
}