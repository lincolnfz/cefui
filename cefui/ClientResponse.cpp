#include "stdafx.h"
#include "ClientResponse.h"
#include "SpeedBox.h"

ClientResponse::ClientResponse()
{
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_AdjustFlashSpeed);
}


ClientResponse::~ClientResponse()
{
}

bool ClientResponse::rsp_AdjustFlashSpeed(const int id,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	double dt = req_parm->getList().GetDoubleVal(0);
	if (abs(dt - 1.0) < 0.01)
	{
		EnableSpeedControl(true);
	}
	else{
		EnableSpeedControl(FALSE);
		SetSpeed(dt);
	}

	return true;
}