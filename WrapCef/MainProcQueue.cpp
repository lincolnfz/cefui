#include "stdafx.h"
#include "MainProcQueue.h"
#include <assert.h>
#include <include\wrapper\cef_helpers.h>


namespace cyjh{

	MainProcQueue MainProcQueue::s_inst;

	MainProcQueue::MainProcQueue()
	{
	}


	MainProcQueue::~MainProcQueue()
	{
	}

	bool MainProcQueue::pushReq(int browserid, int req)
	{
		std::unique_lock<std::mutex> lock(inProcStackMutex_);
		bool ret = false;
		if (m_inProcStack.empty())
		{
			m_inProcStack.push_front(req);
			ret = true;
		}
		else{
			if (m_inProcStack.front() == req)
			{
				m_inProcStack.push_front(req);
				ret = true;
			}
			else{
				PENDING_REQ pend_req(browserid, req);
				m_pendingQueue.push_back(pend_req);
			}
		}
		assert(m_inProcStack.size() == 1);
		return ret;
	}

	void MainProcQueue::popReq(int req)
	{
		std::unique_lock<std::mutex> lock(inProcStackMutex_);
		assert(m_inProcStack.front() == req);
		m_inProcStack.pop_front();
		if ( m_inProcStack.empty() )
		{
			if (!m_pendingQueue.empty())
			{
				int newReqid = m_pendingQueue.front().reqID_;
				int browserid = m_pendingQueue.front().browserID_;
				m_pendingQueue.pop_front();
				if (browserid == 0)
				{
					//ui线程 wake up
					combinThread_->WakeUp();
				}
				else{
					//渲染线程 wake up
					combinThread_->SendRenderWakeUpHelp(browserid);
				}

				m_inProcStack.push_front(newReqid);
			}			
		}
	}
}