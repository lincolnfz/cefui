#include "StdAfx.h"
#include "CliAck.h"
#include "SoundBox.h"
#include "SpeedBox.h"

CliAck::CliAck(void)
{
	REGISTER_ACK_FUNCTION(CliAck, ack_InitClient);
}


CliAck::~CliAck(void)
{
}

extern bool speed_adjust;
bool CliAck::ack_InitClient(const int id, const std::shared_ptr<cyjh::Instruct> spAck)
{
	const bool muted = spAck->getList().GetBooleanVal(0);
	if (muted)
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

	const double dt = spAck->getList().GetDoubleVal(1);
	if (fabs(dt - 1.0) < 0.01)
	{
		if (!speed_adjust)
		{
			EnableSpeedControl(FALSE);
		}
		else{
			EnableSpeedControl(TRUE);
			SetSpeed(1.001);
		}
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
