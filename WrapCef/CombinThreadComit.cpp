#include "stdafx.h"
#include "CombinThreadComit.h"
#include "DataProcessQueue.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "BrowserIdentifier.h"
#include "WebViewFactory.h"

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

	double unif_rand(){  //生成(0,1)的实数均匀分布
		static unsigned int stal = 0;
		++stal;
		unsigned int processid = GetProcessId(GetCurrentProcess()) + GetTickCount();
		srand((unsigned)time(NULL) + stal + processid);
		return(rand() + 0.5) / (RAND_MAX + 1.0);
	}

	int unif_int(int a, int b){   //生成a到b-1的整数均匀分布
		return int(floor(a + (b - a)*unif_rand()));
	}

	void Instruct::SerializationInstruct(const Instruct* inst, Pickle& pick)
	{
		pick.WriteInt(inst->type_);
		pick.WriteInt(inst->id_);
		pick.WriteInt(inst->browserID_);
		pick.WriteBool(inst->succ_);
		pick.WriteString(inst->name_);
		int len = inst->list_.GetSize();
		pick.WriteInt(len);
		for (int idx = 0; idx < len; ++idx)
		{
			const cyjh_value::Type type = inst->list_.GetType(idx);
			pick.WriteInt(type);
			switch (type)
			{
			case cyjh_value::TYPE_BOOLEAN:
			{
				const bool val = inst->list_.GetBooleanVal(idx);
				pick.WriteBool(val);
			}
			break;
			case cyjh_value::TYPE_INTEGER:
			{
				const int val = inst->list_.GetIntVal(idx);
				pick.WriteInt(val);
			}
			break;
			case cyjh_value::TYPE_DOUBLE:
			{
				const double val = inst->list_.GetDoubleVal(idx);
				pick.WriteDouble(val);
			}
			break;
			case cyjh_value::TYPE_STRING:
			{
				const std::string val = inst->list_.GetStrVal(idx);
				pick.WriteString(val);
			}
			break;
			case cyjh_value::TYPE_WSTRING:
			{
				const std::wstring val = inst->list_.GetWStrVal(idx);
				pick.WriteWString(val);
			}
			break;
			default:
				break;
			}
		}
	}

	bool Instruct::ObjectInstruct(const Pickle& pick, Instruct* inst)
	{
		bool bret = true;
		cyjh::PickleIterator itor(pick);		
		while (true){
			int inst_type = 0;
			if (!pick.ReadInt(&itor, &inst_type)){
				bret = false;
				break;
			}
			inst->setInstructType((InstructType)inst_type);

			int id = 0;
			if (!pick.ReadInt(&itor, &id)){
				bret = false;
				break;
			}
			inst->setID(id);

			int browseid = 0;
			if (!pick.ReadInt(&itor, &browseid)){
				bret = false;
				break;
			}
			inst->setBrowserID(browseid);

			bool succ = false;
			if (!pick.ReadBool(&itor, &succ)){
				bret = false;
				break;
			}
			inst->setSucc(succ);

			std::string name;
			if ( !pick.ReadString(&itor, &name) ){
				bret = false;
				break;
			}
			inst->setName(name.c_str());

			int len = 0;
			pick.ReadInt(&itor, &len);
			for (int idx = 0; idx < len; ++idx)
			{
				int val = 0;
				pick.ReadInt(&itor, &val);
				cyjh_value::Type val_type = static_cast<cyjh_value::Type>(val);
				switch (val_type)
				{
				case cyjh::cyjh_value::TYPE_NULL:
					break;
				case cyjh::cyjh_value::TYPE_BOOLEAN:
				{
					bool val = false;
					if ( !pick.ReadBool(&itor, &val) ){
						bret = false;
					}
					inst->getList().AppendVal(val);
				}
					break;
				case cyjh::cyjh_value::TYPE_INTEGER:
				{
					int val = 0;
					if (!pick.ReadInt(&itor, &val)){
						bret = false;
					}
					inst->getList().AppendVal(val);
				}
					break;
				case cyjh::cyjh_value::TYPE_DOUBLE:
				{
					double val = 0.0;
					if (!pick.ReadDouble(&itor, &val)){
						bret = false;
					}
					inst->getList().AppendVal(val);
				}
					break;
				case cyjh::cyjh_value::TYPE_STRING:
				{
					std::string val;
					if (!pick.ReadString(&itor, &val)){
						bret = false;
					}
					inst->getList().AppendVal(val);
				}
				break;
				case cyjh::cyjh_value::TYPE_WSTRING:
				{
					std::wstring val;
					if (!pick.ReadWString(&itor, &val)){
						bret = false;
					}
					inst->getList().AppendVal(val);
				}
					break;
				case cyjh::cyjh_value::TYPE_BINARY:
					break;
				case cyjh::cyjh_value::TYPE_DICTIONARY:
					break;
				case cyjh::cyjh_value::TYPE_LIST:
					break;
				default:
					break;
				}

				if ( bret == false )
				{
					break;
				}
			}

			break;
		}
		return bret;
	}

	Instruct::Instruct()
	{
		browserID_ = 0;
		id_ = 0;
		type_ = INSTRUCT_NULL;
	}

	Instruct::~Instruct()
	{

	}

	DWORD  CombinThreadComit::s_tid_ = 0;
	CombinThreadComit::CombinThreadComit(ThreadType type)
	{
		threadType_ = type;
		requestID_ = 0;
		CombinThreadComit::s_tid_ = GetCurrentThreadId();
	}


	CombinThreadComit::~CombinThreadComit()
	{
	}

	void CombinThreadComit::pushRequestEvent(std::shared_ptr<RequestContext>& events)
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		eventRequestStack_.push_front(events);
	}

	void CombinThreadComit::popRequestEvent()
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		eventRequestStack_.pop_front();
	}

	void CombinThreadComit::pushRecvRequestID(int id)
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
		eventResponsStack_.push_front(id);
	}

	bool CombinThreadComit::popRecvRequestID(int id)
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
		bool ret = false;
		std::deque<int>::iterator it = eventResponsStack_.begin();
		for (; it != eventResponsStack_.end(); ++it)
		{
			if ( *it == id )
			{
				eventResponsStack_.erase(it);
				ret = true;
				break;
			}
		}
		//eventResponsStack_.pop_front();
		assert(ret);
		return ret;
	}

	int CombinThreadComit::generateID()
	{
		std::unique_lock<std::mutex> lock(generateIDMutex_);
		//++requestID_;
		//return s_tid_ * 100 + requestID_;
		return unif_int(1, 0xfffffff);
	}

	void CombinThreadComit::procRecvRequest(const std::shared_ptr<Instruct> parm)
	{
		pushRecvRequestID(parm->getID());
	}

	void CombinThreadComit::SendRequest(IPCUnit* ipc, Instruct& parm, std::shared_ptr<Instruct>& response_val)
	{
		int reqeustid = 0;
		eventResponseStackMutex_.lock();
		if (!eventResponsStack_.empty())
		{
			reqeustid = eventResponsStack_.front();
		}
		eventResponseStackMutex_.unlock();
		reqeustid = reqeustid == 0 ? generateID() : reqeustid;
		parm.setID(reqeustid);		
		std::shared_ptr<RequestContext> sp(new RequestContext());
		//sp->outval_ = &response_val;
		//sp->id_ = requestID_;
		sp->id_ = reqeustid;
		pushRequestEvent(sp);
		//向另一个线程请求
		//ipc_send		
		//这里放ipc的发送操作
		parm.setInstructType(InstructType::INSTRUCT_REQUEST);

		Pickle pick;
		Instruct::SerializationInstruct(&parm, pick);

		//pick.data(), pick.size()
		ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		/////ipc
		while (true)
		{
			DWORD dwWait = 0;
			if ( threadType_ == THREAD_UI )
			{
				dwWait = WaitWithMessageLoop(sp->events_, 2, INFINITE);
			}else if ( threadType_ == THREAD_RENDER )
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
		popRequestEvent();
	}

	void CombinThreadComit::Response(IPCUnit* ipc, std::shared_ptr<Instruct> resp, const int& req_id)
	{
		assert(ipc);
		if ( !ipc )
		{
			return;
		}
		int responseID = 0;
		/*eventResponseStackMutex_.lock();
		if (!eventResponsStack_.empty())
		{
			responseID = eventResponsStack_.front();
		}
		eventResponseStackMutex_.unlock();
		assert(responseID!=0);*/
		resp->setID(req_id);
		popRecvRequestID(req_id);
		resp->setInstructType(InstructType::INSTRUCT_RESPONSE);
		Pickle pick;
		Instruct::SerializationInstruct(resp.get(), pick);
		//pick.data(), pick.size()
		ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
	}

	std::shared_ptr<RequestContext> CombinThreadComit::getReqStackTop(int id)
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		std::shared_ptr<RequestContext> ret;
		//if (!eventRequestStack_.empty()){
		//	ret = eventRequestStack_.front();
		//}
		std::deque<std::shared_ptr<RequestContext>>::iterator it = eventRequestStack_.begin();
		for (; it != eventRequestStack_.end(); ++it){
			if ( it->get()->id_ == id )
			{
				ret = *it;
				break;
			}
		}
		return ret;
	}

	void CombinThreadComit::RecvData(const unsigned char* data, DWORD len)
	{
		cyjh::Pickle pick(reinterpret_cast<const char*>(data), len);
		std::shared_ptr<Instruct> spInstruct(new Instruct());
		bool objected = Instruct::ObjectInstruct(pick, spInstruct.get()); //对像化
		assert(objected);

		std::shared_ptr<RequestContext> top = getReqStackTop(spInstruct->getID());
		if ( top.get() )
		{
			if (spInstruct->getInstructType() == INSTRUCT_RESPONSE)
			{
				top->outval_ = spInstruct;
				SetEvent(top->events_[0]);
			}
			else if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				top->parm_ = spInstruct;
				//pushRecvRequestID(spInstruct->getID()); //移到ui线程或render线程中处理
				SetEvent(top->events_[1]);
			}
		}
		else{
			if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				//pushRecvRequestID(spInstruct->getID()); //移到ui线程或render线程中处理
				procRecvRequest(spInstruct);
			}
		}
		
		
	}
}