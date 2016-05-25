#pragma once
#include "ResponseHandle.h"

namespace cyjh{

class CLogicUnit
{
public:
	CLogicUnit(void);
	virtual ~CLogicUnit(void);

	virtual void ProcReq(const std::shared_ptr<Instruct> spReq);
	virtual void ProcAck(const std::shared_ptr<Instruct> spAck);
	virtual void Response(const std::shared_ptr<Instruct> spOut) = 0;

protected:
	ResponseHandle* m_response;
	AckHandle* m_ack;
};

}