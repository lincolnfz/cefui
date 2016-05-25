#pragma once
#include "pipe\ResponseHandle.h"

class ClientResponse : public cyjh::ResponseHandle
{
public:
	ClientResponse();
	virtual ~ClientResponse();

	bool rsp_AdjustFlashSpeed(const int id,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);
};

