#include "StdAfx.h"
#include "LogicUnit.h"

namespace cyjh{

CLogicUnit::CLogicUnit(void)
{
	m_response = NULL;
	m_ack = NULL;
}


CLogicUnit::~CLogicUnit(void)
{
}

void CLogicUnit::ProcReq(const std::shared_ptr<Instruct> spReq)
{
	std::shared_ptr<Instruct> spOut(new Instruct);
	spOut->setName(spReq->getName().c_str());
	spOut->setID(spReq->getID());
	spOut->setFD(spReq->getFD());
	spOut->setTarget(spReq->getSender());
	spOut->setType(Instruct::RECV_ACK);
	if ( m_response )
	{
		bool succ = m_response->handleQuest(spReq->getID(), spReq, spOut);
		spOut->setSucc(succ);
		Response( spOut );
	}
}

void CLogicUnit::ProcAck(const std::shared_ptr<Instruct> spAck)
{
	if ( m_ack )
	{
		m_ack->handleAck( spAck->getID(), spAck );
	}
}

}