#pragma once
#include "pipe/IPC.h"
#include "pipe/instruct.h"
#include "pipe/DataProcessQueue.h"
#include "pipe/LogicUnit.h"
#include "ClientResponse.h"

class ClientLogic : public cyjh::CLogicUnit, public CDataProcessQueue<cyjh::Instruct>
{
public:
	ClientLogic();
	virtual ~ClientLogic();	

	void Connect(WCHAR* srv, WCHAR* cli);	

protected:
	void RecvPipeData(const unsigned char* data, DWORD len);
	virtual BOOL ProcDataPack(std::shared_ptr<cyjh::Instruct>) override;
	virtual void Response(const std::shared_ptr<cyjh::Instruct> spOut) override;

private:
	std::shared_ptr<cyjh::IPCUnit> m_ipcUnit;
	ClientResponse m_cliResponse;
};

