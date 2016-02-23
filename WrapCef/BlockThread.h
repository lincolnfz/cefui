#ifndef BLOCKTHREAD_H
#define BLOCKTHREAD_H
#pragma once

#include <memory>
#include <deque>
#include <mutex>
//#include "CombinThreadComit.h"

namespace cyjh{
	class CombinThreadComit;
	class Instruct;
	class BlockThread
	{
	public:
		BlockThread(CombinThreadComit*);
		virtual ~BlockThread();

		virtual void block() = 0;

		virtual bool ProcTrunk(std::shared_ptr<Instruct> val);

		virtual void WakeUp();

		void Push();

		void Pop();

		bool Empty();
	protected:
		HANDLE hEnv_[2];
		CombinThreadComit* threadComit_;
		std::shared_ptr<Instruct> swap_;
		std::deque<int> blockQueue_;
		std::mutex blockQueueMutex_;
	};

	class UIBlockThread : public BlockThread
	{
	public:
		UIBlockThread(CombinThreadComit*);
		virtual ~UIBlockThread();

		virtual void block() override;
	};

	class RenderBlockThread : public BlockThread
	{
	public:
		RenderBlockThread(CombinThreadComit*);
		virtual ~RenderBlockThread();

		virtual void block() override;
	};

}

#endif