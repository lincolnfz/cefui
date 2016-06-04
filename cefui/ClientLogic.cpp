#include "stdafx.h"
#include "ClientLogic.h"

ClientLogic::ClientLogic()
{
	m_cli = NULL;
	m_response = &m_cliResponse;
	this->m_ack = &m_cliAck;
	m_bCreateIpc = FALSE;
}


ClientLogic::~ClientLogic()
{
}

void ClientLogic::Error(int code)
{

}

void ClientLogic::RecvSockData(const unsigned char* data, const int len)
{
	m_cli->Disconnect();
	cyjh::Pickle pick(reinterpret_cast<const char*>(data), len);
	std::shared_ptr<cyjh::Instruct> spInstruct(new cyjh::Instruct());
	bool objected = cyjh::Instruct::ObjectInstruct(pick, spInstruct.get()); //对像化
	assert(objected);
	if (!objected)
	{
		return;
	}

	std::wstring srv = spInstruct->getList().GetWStrVal(0);
	std::wstring client = spInstruct->getList().GetWStrVal(1);
	m_ipcUnit = cyjh::IPC_Manager::getInstance().GenerateIPC(srv.c_str(), client.c_str());
	m_ipcUnit->BindStateCallback(&ClientLogic::PipeStateChange, this);
	m_ipcUnit->Launch();
	m_ipcUnit->BindRecvCallback(&ClientLogic::RecvPipeData, this);
	m_bCreateIpc = TRUE;

}

void ClientLogic::Connect()
{
	std::shared_ptr<cyjh::Instruct> spInstruct(new cyjh::Instruct());
	spInstruct->setSender(GetCurrentProcessId());
	spInstruct->setID(1);
	spInstruct->setName("NewConnect");
	spInstruct->setType(cyjh::Instruct::RECV_REQ);
	spInstruct->getList().AppendVal(false);
	spInstruct->getList().AppendVal(1);
	cyjh::Pickle pick;
	cyjh::Instruct::SerializationInstruct(spInstruct.get(), pick);
	m_cli->SendData(static_cast<const unsigned char*>(pick.data()), pick.size());
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
	if (!m_bCreateIpc)
		return FALSE;

	//OutputDebugStringW(L"-------------ClientLogic::ProcDataPack");
	if (spInst->getType() == cyjh::Instruct::NONE_NIL)
	{
		//这是向对方请求的
		spInst->setType(cyjh::Instruct::RECV_REQ);
		spInst->setSender(GetCurrentProcessId());
		cyjh::Pickle pick;
		cyjh::Instruct::SerializationInstruct(spInst.get(), pick);

		int time = 0;
		while (m_ipcUnit->GetState() != IPC_STATE_DUX_RDY){
			Sleep(150);
			++time;
			if (time > 5)
			{
				return FALSE;
			}
		}
		m_ipcUnit->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
	}
	else if (spInst->getType() == cyjh::Instruct::RECV_REQ)
	{		
		ProcReq(spInst);
	}
	else if (spInst->getType() == cyjh::Instruct::RECV_ACK)
	{
		ProcAck(spInst);
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
		InitClient();
	}
}

void ClientLogic::InitClient()
{
	std::shared_ptr<cyjh::Instruct> spInstruct(new cyjh::Instruct());
	spInstruct->setName(cyjh::PICK_MEMBER_FUN_NAME(__FUNCTION__));
	Submit(spInstruct);
}