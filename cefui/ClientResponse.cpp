#include "stdafx.h"
#include "ClientResponse.h"
#include "SpeedBox.h"
#include "SoundBox.h"
#include "SubClassWindow.h"

bool speed_adjust = false;
ClientResponse::ClientResponse()
{
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_AdjustFlashSpeed);
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_AudioMuted);
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_ProtectWindow);
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_UnProtectWindow);
	REGISTER_RESPONSE_FUNCTION(ClientResponse, rsp_QueryDllLoad);
}


ClientResponse::~ClientResponse()
{
}

bool ClientResponse::rsp_AdjustFlashSpeed(const int id,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	double dt = req_parm->getList().GetDoubleVal(0);
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

bool ClientResponse::rsp_ProtectWindow(const int id,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	HWND hWnd = (HWND)req_parm->getList().GetIntVal(0);
	int flag = req_parm->getList().GetIntVal(1);
	SubClassWindow::GetInst().SubWindow(hWnd, flag);
	return true;
}

bool ClientResponse::rsp_UnProtectWindow(const int id,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	HWND hWnd = (HWND)req_parm->getList().GetIntVal(0);
	SubClassWindow::GetInst().UnSubWIndow(hWnd);
	return true;
}

bool ClientResponse::rsp_QueryDllLoad(const int id,
	const std::shared_ptr<cyjh::Instruct> req_parm, std::shared_ptr<cyjh::Instruct> outVal)
{
	std::wstring dll_name = req_parm->getList().GetWStrVal(0);
	HANDLE hMod = GetModuleHandle(dll_name.c_str());
	bool bLoad = hMod ? true : false;
	outVal->getList().AppendVal(bLoad);
	return true;
}