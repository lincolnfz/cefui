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
#ifdef _DEBUG
			char szTmp[8192] = { 0 };
			sprintf_s(szTmp, "---- pushReq main  -----= id = %d ; browser = %d; size = %d", req, browserid, m_inProcStack.size());
			OutputDebugStringA(szTmp);
#endif		
		}
		else{
			if (m_inProcStack.front() == req)
			{
				m_inProcStack.push_front(req);
				ret = true;
#ifdef _DEBUG
				char szTmp[8192] = { 0 };
				sprintf_s(szTmp, "---- pushReq main  -----= id = %d ; browser = %d; size = %d", req, browserid, m_inProcStack.size());
				OutputDebugStringA(szTmp);
#endif		
			}
			else{
				PENDING_REQ pend_req(browserid, req);
				m_pendingQueue.push_back(pend_req);
#ifdef _DEBUG
				char szTmp[8192] = { 0 };
				sprintf_s(szTmp, "---- pushReq pending  -----= id = %d ; browser = %d; size = %d", req, browserid, m_pendingQueue.size());
				OutputDebugStringA(szTmp);
#endif		
			}
		}
		assert(m_inProcStack.size() == 1);
		return ret;
	}

	unsigned int __stdcall WakeUpUIBlock(void * parm){

		CombinThreadComit* thread = reinterpret_cast<CombinThreadComit*>(parm);
		thread->WakeUp();
		return 0;
	}

	void MainProcQueue::popReq(int req)
	{
		std::unique_lock<std::mutex> lock(inProcStackMutex_);
		assert(m_inProcStack.front() == req);
#ifdef _DEBUG
		int front_req = m_inProcStack.front();
		char szTmp[8192] = { 0 };
		sprintf_s(szTmp, "---- popReq main  -----= id = %d", front_req);
		OutputDebugStringA(szTmp);
#endif		
		m_inProcStack.pop_front();
		if ( m_inProcStack.empty() )
		{
			if (!m_pendingQueue.empty())
			{
				int newReqid = m_pendingQueue.front().reqID_;
				int browserid = m_pendingQueue.front().browserID_;
				m_pendingQueue.pop_front();
#ifdef _DEBUG
				char szTmp[8192] = { 0 };
				sprintf_s(szTmp, "---- pushReq from pending  -----= id = %d ; browser = %d", newReqid, browserid);
				OutputDebugStringA(szTmp);
#endif		
				m_inProcStack.push_front(newReqid);
				if (browserid == 0)
				{
					//ui线程 wake up
					//combinThread_->WakeUp();
					if (CefCurrentlyOn(TID_UI)){
						unsigned int id;
						HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, WakeUpUIBlock, combinThread_, 0, &id);
						CloseHandle(hThread);
#ifdef _DEBUG
						OutputDebugStringW(L"wake up in uiThread!!!!!");
#endif
					}
					else{
						combinThread_->WakeUp();
#ifdef _DEBUG
						OutputDebugStringW(L"wake up no in uiThread");
#endif
					}
					
				}
				else{
					//渲染线程 wake up
					combinThread_->SendRenderWakeUpHelp(browserid);
				}

				//m_inProcStack.push_front(newReqid);
			}			
		}
	}
}