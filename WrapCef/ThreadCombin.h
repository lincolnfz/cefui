#ifndef THREAD_COMBIN_H
#define THREAD_COMBIN_H
#pragma once
#include "CombinThreadComit.h"
#include "ResponseRender.h"
#include "ResponseUI.h"

namespace cyjh{

	class UIThreadCombin : public CombinThreadComit
	{
	public:
		UIThreadCombin();
		virtual ~UIThreadCombin();

	protected:
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) override;

	protected:
		ResponseUI handle_;
	};


	class RenderThreadCombin : public CombinThreadComit
	{
	public:
		RenderThreadCombin();
		virtual ~RenderThreadCombin();

	protected:
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) override;

	protected:
		ResponseRender handle_;
	};

}
#endif