#pragma once
#include "pipe\ResponseHandle.h"

class ClientResponse : public cyjh::ResponseHandle
{
public:
	ClientResponse();
	virtual ~ClientResponse();

	bool rsp_AdjustFlashSpeed(const int id,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_AudioMuted(const int id,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_ProtectWindow(const int id,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_UnProtectWindow(const int id,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);

	bool rsp_QueryDllLoad(const int id,
		const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal);
};

