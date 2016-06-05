#include "stdafx.h"
#include "ClientResponse.h"
#include "SpeedBox.h"
#include "SoundBox.h"

bool speed_adjust = false;
ClientResponse::ClientResponse()
{
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_AdjustFlashSpeed);
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_AudioMuted);
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
		//if (!speed_adjust)
		//{
			EnableSpeedControl(FALSE);
		//}
		//else{
		//	EnableSpeedControl(TRUE);
		//	SetSpeed(1.0);
		//}
		//OutputDebugString(_T("------------------ClientResponse::rsp_AdjustFlashSpeed disable "));
	}
	else{
		EnableSpeedControl(TRUE);
		SetSpeed(dt);
		speed_adjust = true;
		//OutputDebugString(_T("------------------ClientResponse::rsp_AdjustFlashSpeed enable "));
	}

	return true;
}

bool ClientResponse::rsp_AudioMuted(const int id,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	bool muted = req_parm->getList().GetBooleanVal(0);
	if ( muted )
	{
		EnableSoundControl(TRUE);
		CloseSound();
		//OutputDebugString(_T("------------------111ClientResponse::rsp_AudioMuted Enable "));
	}
	else{
		OpenSound();
		EnableSoundControl(FALSE);
		//OutputDebugString(_T("------------------111ClientResponse::rsp_AudioMuted disable "));
	}
	return true;
}