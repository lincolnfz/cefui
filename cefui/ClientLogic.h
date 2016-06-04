#pragma once
#include "pipe/IPC.h"
#include "pipe/instruct.h"
#include "pipe/DataProcessQueue.h"
#include "pipe/LogicUnit.h"
#include "pipe/sockCli.h"
#include "ClientResponse.h"
#include "CliAck.h"

class ClientLogic : public cyjh::CLogicUnit, public CDataProcessQueue<cyjh::Instruct>
{
public:
	ClientLogic();
	virtual ~ClientLogic();	

	void Connect();	
	void RecvSockData(const unsigned char* data, const int len);
	void setSockCli(cyjh::SockCli* cli){
		m_cli = cli;
	}
	void Error(int code);

protected:
	void RecvPipeData(const unsigned char* data, DWORD len);
	void PipeStateChange(int state);
	virtual BOOL ProcDataPack(std::shared_ptr<cyjh::Instruct>) override;
	virtual void Response(const std::shared_ptr<cyjh::Instruct> spOut) override;

	void InitClient();

private:
	std::shared_ptr<cyjh::IPCUnit> m_ipcUnit;
	ClientResponse m_cliResponse;
	CliAck m_cliAck;
	cyjh::SockCli* m_cli;
	BOOL m_bCreateIpc;
};

