#pragma once
#include "./pipe//ResponseHandle.h"

class CliAck : public cyjh::AckHandle
{
public:
	CliAck(void);
	virtual ~CliAck(void);

	bool ack_InitClient(const int id, const std::shared_ptr<cyjh::Instruct>);
};

