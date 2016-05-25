#include "stdafx.h"
#include "ClientLogic.h"

ClientLogic::ClientLogic()
{
	m_response = &m_cliResponse;
}


ClientLogic::~ClientLogic()
{
}

void ClientLogic::Connect(WCHAR* srv, WCHAR* cli)
{
	m_ipcUnit = cyjh::IPC_Manager::getInstance().GenerateIPC(srv, cli);	
	m_ipcUnit->BindStateCallback(&ClientLogic::PipeStateChange, this);
	m_ipcUnit->Launch();	
	m_ipcUnit->BindRecvCallback(&ClientLogic::RecvPipeData, this);
}

void ClientLogic::RecvPipeData(const unsigned char* data, DWORD len)
{
	//OutputDebugStringW(L"-------------------ClientLogic::RecvPipeData");
	cyjh::Pickle pick(reinterpret_cast<const char*>(data), len);
	std::shared_ptr<cyjh::Instruct> spInstruct(new cyjh::Instruct());
	bool objected = cyjh::Instruct::ObjectInstruct(pick, spInstruct.get()); //对像化
	assert(objected);
	if (!objected)
	{
		return;
	}
	Submit(spInstruct);
}

BOOL ClientLogic::ProcDataPack(std::shared_ptr<cyjh::Instruct> spInst)
{
	//OutputDebugStringW(L"-------------ClientLogic::ProcDataPack");
	if (spInst->getType() == cyjh::Instruct::NONE_NIL)
	{
		//这是向对方请求的
	}
	else if (spInst->getType() == cyjh::Instruct::RECV_REQ)
	{		
		ProcReq(spInst);
	}
	return true;
}

void ClientLogic::Response(const std::shared_ptr<cyjh::Instruct> spOut)
{
	
}

void ClientLogic::PipeStateChange(int state)
{
	if (state == IPC_STATE_DUX_RDY)
	{
		//管道已联上,如果不是主程序需要向服务请求初始化资料
		//OutputDebugString(_T("------------------ClientLogic::PipeStateChange IPC_STATE_DUX_RDY"));
	}
}