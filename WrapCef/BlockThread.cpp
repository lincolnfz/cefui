#include "stdafx.h"
#include "BlockThread.h"
#include "ThreadCombin.h"
#include "ShareHelper.h"

namespace cyjh{
	BlockThread::BlockThread(CombinThreadComit* ptr) :threadComit_(ptr)
	{
		hEnv_[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
		hEnv_[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}


	BlockThread::~BlockThread()
	{
		CloseHandle(hEnv_[0]);
		CloseHandle(hEnv_[1]);
	}

	bool BlockThread::ProcTrunk(std::shared_ptr<Instruct> val)
	{
		/*if (swap_.get())
			swap_.reset();
		swap_ = val;*/
		if (Empty()){
			return false;
		}
		else{
			if (swap_.get())
				swap_.reset();
			swap_ = val;
			SetEvent(hEnv_[1]);
		}		
		return true;
	}

	void BlockThread::WakeUp()
	{
		SetEvent(hEnv_[0]);
#ifdef _DEBUG
		OutputDebugStringW(L"------BlockThread::WakeUp()");
#endif		
	}

	void BlockThread::Push()
	{
		std::unique_lock<std::mutex> lock(blockQueueMutex_);
		static int id = 0;
		++id;
		blockQueue_.push_front(id);
	}

	void BlockThread::Pop()
	{
		std::unique_lock<std::mutex> lock(blockQueueMutex_);
		blockQueue_.pop_front();
	}

	bool BlockThread::Empty()
	{
		std::unique_lock<std::mutex> lock(blockQueueMutex_);
		return blockQueue_.empty();
	}

	///////////////////////////////////////////////////////////////////
	UIBlockThread::UIBlockThread(CombinThreadComit* ptr) :BlockThread(ptr)
	{

	}
	UIBlockThread::~UIBlockThread()
	{

	}

	void UIBlockThread::block(bool haveNewInstruct)
	{
		//Push();
		ResetEvent(hEnv_[0]);
		ResetEvent(hEnv_[1]);
		if ( haveNewInstruct )
		{
			SetEvent(hEnv_[1]);
		}
		while ( true )
		{
			//DWORD dwWait = WaitWithMessageLoop(hEnv_, 2, INFINITE);
			DWORD dwWait = WaitForMultiEvent(hEnv_, 2, INFINITE);
			if (dwWait == WAIT_OBJECT_0 + 0){
				break;
			}else if ( dwWait == WAIT_OBJECT_0 + 1 )
			{
				threadComit_->ProcTrunkReq(swap_);
				continue;
			}
			else{
				continue;;
			}
		}
		ResetEvent(hEnv_[0]);
		ResetEvent(hEnv_[1]);
		Pop();
	}


	/////////////////////////////////////////////////////

	RenderBlockThread::RenderBlockThread(CombinThreadComit* ptr) : BlockThread(ptr)
	{

	}

	RenderBlockThread::~RenderBlockThread()
	{

	}

	void RenderBlockThread::block(bool haveNewInstruct)
	{
		//Push();
		ResetEvent(hEnv_[0]);
		ResetEvent(hEnv_[1]);
		if (haveNewInstruct)
		{
			SetEvent(hEnv_[1]);
		}
		while (true)
		{
			DWORD dwWait = WaitForMultiEvent(hEnv_, 2, INFINITE);
			if (dwWait == WAIT_OBJECT_0){
				break;
			}
			else if (dwWait == WAIT_OBJECT_0 + 1)
			{
				threadComit_->ProcTrunkReq(swap_);
				continue;
			}
			else{
				break;
			}
		}
		ResetEvent(hEnv_[0]);
		ResetEvent(hEnv_[1]);
		Pop();
	}
}