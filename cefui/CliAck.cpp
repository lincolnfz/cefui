#include "StdAfx.h"
#include "CliAck.h"
#include "SoundBox.h"

CliAck::CliAck(void)
{
	REGISTER_ACK_FUNCTION(CliAck, ack_InitClient);
}


CliAck::~CliAck(void)
{
}

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
	return true;
}
