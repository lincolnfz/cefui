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
	m_ipcUnit->BindRecvCallback(&ClientLogic::RecvPipeData, this);
	m_ipcUnit->Launch();
}

void ClientLogic::RecvPipeData(const unsigned char* data, DWORD len)
{
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