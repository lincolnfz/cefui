//#include "stdafx.h"
#include "CombinThreadComit.h"
#include "DataProcessQueue.h"

namespace cyjh{

	static void DoEvents()
	{
		MSG msg;

		// window message         
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	static DWORD WaitWithMessageLoop(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds)
	{
		DWORD dwRet = WAIT_FAILED;

		DWORD dwMaxTick = (dwMilliseconds == INFINITE) ? INFINITE : GetTickCount() + dwMilliseconds;

		while (1)
		{
			DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
			// wait for event or message, if it's a message, process it and return to waiting state
			dwRet = MsgWaitForMultipleObjects(nCount, hEvent, FALSE, dwTimeOut/*dwMaxTick - GetTickCount()*/, QS_ALLINPUT);
			if (dwRet == WAIT_OBJECT_0 + nCount - 1)
			{
				//OutputDebugStringA("WaitWithMessageLoop() event triggered.\n");
				return dwRet;
			}
			else if (dwRet == WAIT_OBJECT_0 + nCount)
			{
				// process window messages
				//OutputDebugStringA("DoEvents.\n");
				DoEvents();
			}
			else if (dwRet == WAIT_TIMEOUT)
			{
				// timed out !
				//OutputDebugStringA("timed out!\n");
				return dwRet;
			}
			else if (dwRet == WAIT_FAILED)
			{
				//OutputDebugStringA("wait failed!\n");
				return dwRet;
			}
			else{
				return dwRet;
			}

		}
	}

	static DWORD WaitForMultiEvent(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds)
	{
		DWORD dwRet = WAIT_FAILED;

		DWORD dwMaxTick = (dwMilliseconds == INFINITE) ? INFINITE : GetTickCount() + dwMilliseconds;

		while (1)
		{
			DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
			// wait for event or message, if it's a message, process it and return to waiting state
			dwRet = WaitForMultipleObjects(nCount, hEvent, FALSE, dwTimeOut);
			if (dwRet == WAIT_OBJECT_0 + nCount - 1)
			{
				//OutputDebugStringA("WaitWithMessageLoop() event triggered.\n");
				return dwRet;
			}
			else if (dwRet == WAIT_TIMEOUT)
			{
				// timed out !
				//OutputDebugStringA("timed out!\n");
				return dwRet;
			}
			else if (dwRet == WAIT_FAILED)
			{
				//OutputDebugStringA("wait failed!\n");
				return dwRet;
			}
			else{
				return dwRet;
			}

		}
	}

	static void SerializationInstruct(const Instruct* inst, Pickle& pick)
	{

	}

	static void ObjectInstruct(const Pickle& pick, Instruct* inst)
	{

	}

	Instruct::Instruct()
	{

	}

	Instruct::~Instruct()
	{

	}

	CombinThreadComit::CombinThreadComit(ThreadType type)
	{
		threadType_ = type;
	}


	CombinThreadComit::~CombinThreadComit()
	{
	}

	void CombinThreadComit::pushEvent(std::shared_ptr<RequestContext>& events)
	{
		std::unique_lock<std::mutex> lock(eventStackMutex_);
		eventStack_.push_front(events);
	}

	void CombinThreadComit::popEvent()
	{
		std::unique_lock<std::mutex> lock(eventStackMutex_);
		eventStack_.pop_front();
	}

	void CombinThreadComit::procRecvRequest(std::shared_ptr<Instruct> parm)
	{
		//处理另一个线程发来的请求
	}

	void CombinThreadComit::Request(Instruct& parm, std::shared_ptr<Instruct> response_val)
	{
		std::shared_ptr<RequestContext> sp(new RequestContext());
		//sp->outval_ = &response_val;
		pushEvent(sp);
		//向另一个线程请求
		//ipc_send		
		//这里放ipc的发送操作
		parm.setInstructType(InstructType::INSTRUCT_REQUEST);

		Pickle pick;
		SerializationInstruct(&parm, pick);

		//pick.data(), pick.size()
		/////ipc
		while (true)
		{
			DWORD dwWait = 0;
			if ( threadType_ == THREAD_UI )
			{
				dwWait = WaitWithMessageLoop(sp->events_, 2, INFINITE);
			}else if ( threadType_ == THREAD_WORK )
			{
				dwWait = WaitForMultiEvent(sp->events_, 2, INFINITE);
			}

			if (dwWait == WAIT_OBJECT_0)
			{
				//收到返馈
				response_val = sp->outval_;
				break;
			}else if ( dwWait == WAIT_OBJECT_0 + 1 )
			{
				//收到请求
				procRecvRequest(sp->parm_);
			}
		}
		popEvent();
	}

	void CombinThreadComit::Response(Instruct& resp)
	{
		resp.setInstructType(InstructType::INSTRUCT_RESPONSE);
		Pickle pick;
		SerializationInstruct(&resp, pick);
		//pick.data(), pick.size()
	}

	void CombinThreadComit::RecvData(const byte* data, DWORD len)
	{
		cyjh::Pickle pick(reinterpret_cast<const char*>(data), len);
		std::shared_ptr<Instruct> spInstruct(new Instruct());
		ObjectInstruct(pick, spInstruct.get()); //对像化
		//ObjectInstruct( pick,  )
		//
		//Instruct::Object(data, len, spInstruct.get());
		if (!eventStack_.empty())
		{
			RequestContext* context = eventStack_.front().get();
			if (spInstruct->getInstructType() == INSTRUCT_RESPONSE)
			{
				context->outval_ = spInstruct;
				SetEvent(context->events_[0]);
			}
			else if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				context->parm_ = spInstruct;
				SetEvent(context->events_[1]);
			}
		}
		else{
			if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				procRecvRequest(spInstruct);
			}
		}
		
		
	}
}