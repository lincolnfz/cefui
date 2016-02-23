#ifndef MAINPROCQUEUE_H
#define MAINPROCQUEUE_H
#pragma once

#include <deque>
#include <mutex>
#include "ThreadCombin.h"

namespace cyjh{
	struct PENDING_REQ{
		int browserID_;
		int reqID_;
		PENDING_REQ(int browsr, int req){
			browserID_ = browsr;
			reqID_ = req;
		}
	};

	class MainProcQueue
	{
	public:		
		virtual ~MainProcQueue();

		static MainProcQueue& getInst(){
			return s_inst;
		}

		bool pushReq(int browserid, int req);
		void popReq(int req);
		void setCombinThread(CombinThreadComit* combin){
			combinThread_ = combin;
		}

	protected:
		MainProcQueue();
		std::deque<int> m_inProcStack;
		std::deque<PENDING_REQ> m_pendingQueue;
		std::mutex inProcStackMutex_;
		CombinThreadComit* combinThread_;
		static MainProcQueue s_inst;
	};
}

#endif