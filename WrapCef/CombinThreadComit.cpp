#include "stdafx.h"
#include "CombinThreadComit.h"
#include "DataProcessQueue.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "BrowserIdentifier.h"
#include "WebViewFactory.h"
#include "MainProcQueue.h"
#include "BlockThread.h"
#include "ShareHelper.h"

namespace cyjh{

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
		pick.WriteInt(inst->atom_);
		pick.WriteInt(inst->browserID_);
		pick.WriteInt(inst->procState_);
		pick.WriteBool(inst->succ_);
		pick.WriteBool(inst->newSession_);
		pick.WriteBool(inst->procTimeout_);
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

			int atom = 0;
			if (!pick.ReadInt(&itor, &atom)){
				bret = false;
				break;
			}
			inst->setAtom(atom);

			int browseid = 0;
			if (!pick.ReadInt(&itor, &browseid)){
				bret = false;
				break;
			}
			inst->setBrowserID(browseid);

			int procState = 0;
			if (!pick.ReadInt(&itor, &procState)){
				bret = false;
				break;
			}
			inst->setProcState(procState);

			bool succ = false;
			if (!pick.ReadBool(&itor, &succ)){
				bret = false;
				break;
			}
			inst->setSucc(succ);

			bool newSession = true;
			if (!pick.ReadBool(&itor, &newSession)){
				bret = false;
				break;
			}
			inst->setNewSession(newSession);

			bool timeout = false;
			if (!pick.ReadBool(&itor, &timeout)){
				bret = false;
				break;
			}
			inst->setProcTimeout(timeout);

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
		newSession_ = true;
		procState_ = PROC_STATE_NIL;
		procTimeout_ = false;
	}

	Instruct::~Instruct()
	{

	}

	BOOL SyncRequestQueue::ProcDataPack(std::shared_ptr<ReqInfo> sp){
		WaitForSingleObject(hEvent_, INFINITE);
		ResetEvent();
		obj_->RecvData(sp->pData_, sp->len_);
		return TRUE;
	}

	BOOL SyncRequestQueue::SubmitPack(std::shared_ptr<ReqInfo> pack)
	{
		return Submit(pack);
	}

	unsigned int __stdcall MaybeLockItem::WaitReqTimeOut(void * parm)
	{
		/*MaybeLockItem* obj = reinterpret_cast<MaybeLockItem*>(parm);
		DWORD dwRet = WaitForSingleObject(obj->hEvent_, 1500);
		if (dwRet == WAIT_TIMEOUT)
		{
			obj->srv_->pushProcedQueue(obj->spRemote_Req_->getID(), obj->spRemote_Req_->getAtom());
			Pickle pick;
			Instruct::SerializationInstruct(obj->spRemote_Req_.get() , pick);
			obj->srv_->RecvData(static_cast<const unsigned char*>(pick.data()), pick.size());
		}*/
		return 0;
	}

	void __stdcall MaybeLockItem::WaitOrTimerCallback(
		PVOID   lpParameter,
		BOOLEAN TimerOrWaitFired){
		MaybeLockItem* obj = reinterpret_cast<MaybeLockItem*>(lpParameter);
		if ( obj->bTimeout_ )
		{
#ifdef _DEBUG1
			char szTmp[256] = { 0 };
			sprintf_s(szTmp, "----time out proc name = %s ; id = %d ; theadID=%d ; %s\n", obj->spRemote_Req_->getName().c_str(),
				obj->spRemote_Req_->getID(), GetCurrentThreadId(), obj->srv_->threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
#endif
			obj->spRemote_Req_->setProcTimeout(true);
			obj->srv_->pushProcedQueue(obj->spRemote_Req_->getID(), obj->spRemote_Req_->getAtom());
			Pickle pick;
			Instruct::SerializationInstruct(obj->spRemote_Req_.get(), pick);
			obj->srv_->RecvData(static_cast<const unsigned char*>(pick.data()), pick.size());
		}
	}

	MaybeLockItem::MaybeLockItem(std::shared_ptr<Instruct>& remote_req, CombinThreadComit* srv){
		spRemote_Req_ = remote_req;
		srv_ = srv;
		//hEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		//unsigned int id;
		bTimeout_ = true;
		//hThread_ = (HANDLE)_beginthreadex(nullptr, 0, WaitReqTimeOut, this, 0, &id);
		//CloseHandle(hThread);
		//CreateTimerQueueTimer(&hTimer_, srv->m_hTimeQueue, WaitOrTimerCallback, this, 1500, 0, WT_EXECUTEDEFAULT);
	}

	MaybeLockItem::~MaybeLockItem(){
		//SetEvent(hEvent_);
		//WaitForSingleObject(hThread_, INFINITE);
		//CloseHandle(hThread_);
		
	}

	void MaybeLockItem::cancelTimer()
	{
		bTimeout_ = false;
		DeleteTimerQueueTimer(srv_->m_hTimeQueue, hTimer_, INVALID_HANDLE_VALUE);
	}

	DWORD  CombinThreadComit::s_tid_ = 0;
	CombinThreadComit::CombinThreadComit(ThreadType type) //:requestQueue_(this)
	{
		threadType_ = type;
		requestID_ = 0;
		blockThread_ = NULL;
		CombinThreadComit::s_tid_ = GetCurrentThreadId();
		m_hTimeQueue = CreateTimerQueue();
	}


	CombinThreadComit::~CombinThreadComit()
	{
		DeleteTimerQueue(m_hTimeQueue);
	}

	void CombinThreadComit::pushRequestEvent(std::shared_ptr<RequestContext>& events)
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		eventRequestStack_.push_front(events);
	}

	bool CombinThreadComit::popRequestEvent(int reqid, int atom)
	{
		bool ret = false;
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		std::deque<std::shared_ptr<RequestContext>>::iterator it = eventRequestStack_.begin();
		for ( ; it != eventRequestStack_.end(); ++it )
		{
			std::shared_ptr<RequestContext> context = *it;
			if (context->id_ == reqid && context->atom_ == atom)
			{
#ifdef _DEBUG1
				//std::shared_ptr<RequestContext> context = eventRequestStack_.front();
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "---- popReq id = %d , atom = %d theadID=%d ; %s\n", context->id_, context->atom_, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szTmp);
#endif
				eventRequestStack_.erase(it);
				ret = true;
				break;
			}
		}
		//eventRequestStack_.pop_front();
		assert(ret);
		return ret;
	}

	bool CombinThreadComit::pushRecvRequestID(int id, int atom)
	{
		bool ret = true;
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);

		RecvReqItem item(id, atom);
#ifdef _SINGLE_INSTRUCT_PROC
		eventResponsStack_.push_front(item);
#else
		if ( !eventResponsStack_.empty() )
		{
			std::deque<RecvReqItem>::iterator it = eventResponsStack_.begin();
			for (; it != eventResponsStack_.end(); ++it)
			{
				if ( it->id_ == id && it->atom_ == atom )
				{
					eventResponsStack_.erase(it);
					assert(false);
					break;
				}
			}
		}


#ifdef _DEBUG1
		char szTmp[256] = { 0 };
		sprintf_s(szTmp, "---- pushRecvReq id = %d, theadID=%d ; %s\n", id, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
		OutputDebugStringA(szTmp);
#endif

		//eventResponsStack_.push_back(item);
		eventResponsStack_.push_front(item);
#endif
		return ret;
	}

	bool CombinThreadComit::popRecvRequestID(int id, int atom)
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
#ifdef _DEBUG1
		RecvReqItem item = eventResponsStack_.front();
		//bool match = item.id_ == id && item.atom_ == atom;
		bool match = item.id_ == id;
		if ( !match )
		{
			WCHAR szbuf[1024] = {0};
			swprintf_s(szbuf, L"-----popRecvRequestID error id: %d ; queue: ", id);
			std::deque<RecvReqItem>::iterator it = eventResponsStack_.begin();
			for (; it != eventResponsStack_.end(); ++it)
			{
				WCHAR item[64] = {0};
				swprintf_s(item, L",[%d, %d]", it->id_, it->atom_);
				wcscat_s(szbuf, item);
			}
			OutputDebugStringW(szbuf);
		}
		//assert(item.id_ == id && item.atom_ == atom);
		assert(item.id_ == id);
#endif
		bool ret = false;
		std::deque<RecvReqItem>::iterator it = eventResponsStack_.begin();
		for (; it != eventResponsStack_.end(); ++it)
		{
			if ( it->id_ == id && it->atom_ == atom )
			{
				eventResponsStack_.erase(it);
				ret = true;
#ifdef _DEBUG1
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "---- popResponse id = %d, theadID=%d ; %s\n", id, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szTmp);
#endif
				break;
			}
		}
		//eventResponsStack_.pop_front();
		assert(ret);
		//popOrderRequestID(id, atom);
		//removePendingReq(id, atom);
		return ret;
	}

	bool CombinThreadComit::matchRecvRequestID(int id)
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
		bool ret = true;
		if (!eventResponsStack_.empty())
		{
			ret = (eventResponsStack_.front().id_ == id);
		}
		return ret;
	}

	void CombinThreadComit::pushPengingRequest(std::shared_ptr<Instruct> sp)
	{
		std::unique_lock<std::mutex> lock(pendingProcReqQueue_Mutex_);
		bool bFind = false;
		std::deque<std::shared_ptr<Instruct>>::iterator it = pendingProcReqQueue_.begin();
		for (; it != pendingProcReqQueue_.end(); ++it)
		{
			if (it->get()->getID() == sp->getID() && it->get()->getAtom() == sp->getAtom() )
			{
				bFind = true;
				break;
			}
		}
		if ( !bFind )
		{
			pendingProcReqQueue_.push_back(sp);
			//pushOrderRequestID(sp->getID(), sp->getAtom());
#ifdef _DEBUG1
			{
				char szBuf[256] = { 0 };
				sprintf_s(szBuf, "----in penging id = %d; atom = %d; %s",
					sp->getID(), sp->getAtom(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szBuf);
			}
#endif
		}
		

	}

	std::shared_ptr<Instruct> CombinThreadComit::getTopPengingRequest()
	{
		std::unique_lock<std::mutex> lock(pendingProcReqQueue_Mutex_);
		std::shared_ptr<Instruct> data;
		if (!pendingProcReqQueue_.empty())
		{
			data = pendingProcReqQueue_.front();
			pendingProcReqQueue_.pop_front();
		}
		return data;
	}

	bool CombinThreadComit::removePendingReq(int id, int atom)
	{
		bool ret = true;
		std::unique_lock<std::mutex> lock(pendingProcReqQueue_Mutex_);
		if (!pendingProcReqQueue_.empty())
		{
			std::shared_ptr<Instruct> instruct = pendingProcReqQueue_.front();
			if ( instruct->getID() == id && instruct->getAtom()  )
			{
				pendingProcReqQueue_.pop_front();
			}
			else{
#ifdef _DEBUG1
			char szBuf[256] = { 0 };
			sprintf_s(szBuf, "----error!!!!!! removePendingReq id = %d; atom = %d; %s",
				id, atom, threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szBuf);
#endif
				ret = false;
			}
		}
		assert(ret);
		return ret;
	}

	bool CombinThreadComit::isEmptyPengingReq()
	{
		std::unique_lock<std::mutex> lock(pendingProcReqQueue_Mutex_);
		return pendingProcReqQueue_.empty();
	}


	bool CombinThreadComit::isRecvRequestEmpty()
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
		return eventResponsStack_.empty();
	}

	bool CombinThreadComit::isSendRenderImmedNoBlockEmpty()
	{
		std::unique_lock<std::mutex> lock(sendRenderImmedNoBlockQueue_Mutex_);
		return sendRenderImmedNoBlockQueue_.empty();
	}

	bool CombinThreadComit::isSendRegBrowserEmpty()
	{
		std::unique_lock<std::mutex> lock(sendRegBrowserQueue_Mutex_);
		return sendRegBrowserQueue_.empty();
	}

	int CombinThreadComit::generateID()
	{
		std::unique_lock<std::mutex> lock(generateIDMutex_);
		//++requestID_;
		//return s_tid_ * 100 + requestID_;
		return unif_int(1, 0xfffffff);
	}

	//void CombinThreadComit::procRecvRequest(const std::shared_ptr<Instruct> parm)
	//{
	//	pushRecvRequestID(parm->getID(), parm->getAtom());
	//}

	bool CombinThreadComit::prepareResponse(const std::shared_ptr<Instruct> parm)
	{
		if (hitProcedQueue(parm->getID(), parm->getAtom()))
		{
			return false;
		}

		std::unique_lock<std::mutex> lock(newSessinBlockMutex_);

		bool ret = true;
		removeMaybelockQueue(parm);
		pushRecvRequestID(parm->getID(), parm->getAtom());
		return ret;
	}

	void CombinThreadComit::SendRequest(IPCUnit* ipc, Instruct& parm, std::shared_ptr<Instruct>& response_val)
	{
		//requestQueue_.ResetEvent();
		newSessinBlockMutex_.lock();
		bool bLock = true;
		static volatile int s_atom = 0;
		int reqeustid = 0;
		eventResponseStackMutex_.lock();
		if (!eventResponsStack_.empty())
		{
			reqeustid = eventResponsStack_.front().id_;
		}
		eventResponseStackMutex_.unlock();
		parm.setNewSession(reqeustid == 0);
		reqeustid = parm.newSession() ? generateID() : reqeustid;
		parm.setID(reqeustid);

#ifdef _DEBUG1
#define buf_size 10240
		if (parm.getName().compare("invokedJSMethod") == 0)
		{
			char szTmp[buf_size] = { 0 };
			sprintf_s(szTmp, "----name = %s ; %s | %s  ; id = %d ; new = %d ; theadID=%d ; %s\n", parm.getName().c_str(),
				parm.getList().GetStrVal(0).c_str(), parm.getList().GetStrVal(1).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
		}
		else if (parm.getName().compare("crossInvokeWebMethod") == 0)
		{
			WCHAR szTmp[buf_size] = { 0 };
			swprintf_s(szTmp, L"----name = %s ; %s | %s  ; id = %d ; new = %d ; theadID=%d ; %s\n", L"crossInvokeWebMethod",
				parm.getList().GetWStrVal(1).c_str(), parm.getList().GetWStrVal(2).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? L"ui" : L"render");
			OutputDebugStringW(szTmp);
		}
		else if (parm.getName().compare("invokeMethod") == 0)
		{
			WCHAR szTmp[buf_size] = { 0 };
			swprintf_s(szTmp, L"----name = %s ; %s | %s ; id = %d ; new = %d ; theadID=%d ; %s\n", L"invokeMethod",
				parm.getList().GetWStrVal(0).c_str(), parm.getList().GetWStrVal(1).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? L"ui" : L"render");
			OutputDebugStringW(szTmp);
		}
		else if (parm.getName().compare("crossInvokeWebMethod2") == 0)
		{
			WCHAR szTmp[buf_size] = { 0 };
			swprintf_s(szTmp, L"----name = %s ; %s | %s ; id = %d ; new = %d ; theadID=%d ; %s\n", L"crossInvokeWebMethod2",
				parm.getList().GetWStrVal(2).c_str(), parm.getList().GetWStrVal(3).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? L"ui" : L"render");
			OutputDebugStringW(szTmp);
		}
		else{
			char szTmp[256] = { 0 };
			sprintf_s(szTmp, "----name = %s ; id = %d ; new = %d ; theadID=%d ; %s\n", parm.getName().c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
		}
#endif
		if (parm.getInstructType() != INSTRUCT_REGBROWSER && parm.newSession())
		{
			//注册新的一次请求（可能是会话中的请求）
			/*bool trylock = newSessinBlockMutex_.try_lock();
			if ( !trylock )
			{
#ifdef _DEBUG1
				OutputDebugStringW(L" fail trylock ");
				//assert(false);
#endif
				newSessinBlockMutex_.lock();
			}

			bool inProcQueue = RegisterReqID(ipc, parm.getBrowserID(), reqeustid);
			if (!inProcQueue){
				assert(blockThread_);
				blockThread_->Push();
			}
			//newSessinBlockMutex_.unlock();
			if(threadType_ == THREAD_RENDER) {
				cyjh::Instruct tmp_parm;
				tmp_parm.setBrowserID(parm.getBrowserID());
				tmp_parm.setID(parm.getID());
				tmp_parm.setInstructType(InstructType::INSTRUCT_INQUEUE);
				Pickle pick;
				Instruct::SerializationInstruct(&tmp_parm, pick);
				ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
			}

			bool bHaveRequest = !isRecvRequestEmpty() || !isSendRenderImmedNoBlockEmpty() || !isSendRegBrowserEmpty();
			newSessinBlockMutex_.unlock();
			if (!inProcQueue){
				blockThread_->block(false); //bHaveRequest
			}*/
		}

		int atom = InterlockedIncrement((long*)&s_atom);
		parm.setAtom(atom);
		std::shared_ptr<RequestContext> sp(new RequestContext());
		sp->id_ = reqeustid;
		sp->atom_ = parm.getAtom();
		pushRequestEvent(sp);
		newSessinBlockMutex_.unlock();
		//向另一个线程请求
		//ipc_send		
		//这里放ipc的发送操作
		if (parm.getInstructType() == INSTRUCT_NULL)
		{
			parm.setInstructType(InstructType::INSTRUCT_REQUEST);
		}

		Pickle pick;
		Instruct::SerializationInstruct(&parm, pick);

#ifdef _DEBUG1
		//char szTmp[8192] = { 0 };
		//sprintf_s(szTmp, "---- continue name = %s ; id = %d ; new = %d ; theadID=%d ; %s\n", parm.getName().c_str(),
		//	parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
		//OutputDebugStringA(szTmp);
#endif

		//pick.data(), pick.size()
		ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		
		/////ipc
		while (true)
		{
			//checkMaybelockQueue();
			if (bLock){
				bLock = false;
				//newSessinBlockMutex_.unlock();
			}
			DWORD dwWait = 0;
			if ( threadType_ == THREAD_UI )
			{
				//dwWait = WaitWithMessageLoop(sp->events_, 2, INFINITE);
				dwWait = WaitForMultiEvent(sp->events_, 2, INFINITE);
			}else if ( threadType_ == THREAD_RENDER )
			{
				dwWait = WaitForMultiEvent(sp->events_, 2, INFINITE);
			}

			if ( dwWait == WAIT_OBJECT_0 + 1 )
			{
				//收到请求
				procRecvRequest(sp->parm_);
				sp->bResponse_ = true;
				//sp->parm_.reset();
			}
			else if (dwWait == WAIT_OBJECT_0)
			{
				//收到返馈
				response_val = sp->outval_;
				break;
			}
		}

		assert(response_val.get());
		//popRequestEvent(sp->id_); //放到数据接收线程中处理??????????????,避免接收线程太快，同时处理收到响应，与请求指令
		
		if ( !sp->bResponse_ /*sp->parm_.get()*/ )
		{
#ifdef _DEBUG1
			char szTmp[512] = { 0 };
			sprintf_s(szTmp, "---- !!!!!!lost proc instruct id = %d ; new = %d ; theadID=%d ;  %s\n",
				sp->parm_->getID(), sp->parm_->newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
#endif
			//checkMaybeLostInstruct(sp->parm_);
			postInstruct(sp->parm_);
		}
		
		if (parm.getInstructType() != INSTRUCT_REGBROWSER && parm.newSession()){
			//UnRegisterReqID(ipc, reqeustid);
		}
		//requestQueue_.SetEvent();		
#ifdef _DEBUG1
		if (response_val->getProcState() != PROC_STATE_FIN)
		{
			char szTmp[8192] = { 0 };
			sprintf_s(szTmp, "---- !!!!reject name = %s ; id = %d ; new = %d ; theadID=%d ; procstate=%d ; %s\n", response_val->getName().c_str(),
				response_val->getID(), response_val->newSession(), GetCurrentThreadId(), response_val->getProcState(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
		}
#endif
		if (true/*parm.newSession()*/){
			//如果自身是新的请求就是检测等带队列中的数据。
			//如果是一组会话中的某次请求，在response中会去检测等待队列中的数据
			//????
			//proxy_checkPendingReq();
		}
	}

	void CombinThreadComit::proxy_checkPendingReq()
	{
		std::unique_lock<std::mutex> lock(newSessinBlockMutex_);
		checkPendingReq();
	}

	struct TmpSwapData
	{
		TmpSwapData(){
		}

		~TmpSwapData(){
			delete []buf_;
		}
		CombinThreadComit* obj_;
		unsigned char* buf_;
		unsigned int len_;

	};

	unsigned int __stdcall CombinThreadComit::ProcForkReq(void * parm)
	{
		TmpSwapData* data = reinterpret_cast<TmpSwapData*>(parm);
		data->obj_->RecvData(data->buf_, data->len_);
		delete data;
		return 0;
	}

	void CombinThreadComit::checkPendingReq()
	{
		std::shared_ptr<Instruct>data = getTopPengingRequest();
		if ( data.get() )
		{
			//if (!matchRecvRequestID(data->getID())){
			//	return;
			//}
			Pickle pick;
			Instruct::SerializationInstruct(data.get(), pick);
			TmpSwapData* tmpdata = new TmpSwapData;
			tmpdata->obj_ = this;
			tmpdata->len_ = pick.size();
			tmpdata->buf_ = new unsigned char[tmpdata->len_];
			memcpy_s(tmpdata->buf_, tmpdata->len_,
				static_cast<const unsigned char*>(pick.data()), pick.size());
#ifdef _DEBUG1
			char szTmp[256] = { 0 };
			sprintf_s(szTmp, "----proc penging queue name = %s ; id = %d ; new = %d ; theadID=%d ; %s\n", data->getName().c_str(),
				data->getID(), data->newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
#endif
			unsigned int id;
			HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, ProcForkReq, tmpdata, 0, &id);
			CloseHandle(hThread);
		}
	}

	void CombinThreadComit::pushMaybelockQueue(std::shared_ptr<Instruct>& spReq)
	{
		std::unique_lock<std::mutex> lock(maybeLockReqQueue_Mutex_);
		std::shared_ptr<MaybeLockItem> spitem( new MaybeLockItem(spReq, this));
		maybeLockReqQueue_.push_back(spitem);
		//assert(maybeLockReqQueue_.size() > 1);
	}

	void CombinThreadComit::removeMaybelockQueue(const std::shared_ptr<Instruct>& spReq)
	{
		std::unique_lock<std::mutex> lock(maybeLockReqQueue_Mutex_);
		std::deque<std::shared_ptr<MaybeLockItem>>::iterator it = maybeLockReqQueue_.begin();
		for (; it != maybeLockReqQueue_.end(); ++it)
		{
			if (it->get()->spRemote_Req_->getID() == spReq->getID() && it->get()->spRemote_Req_->getAtom() == spReq->getAtom())
			{
				//it->get()->cancelTimer();
				maybeLockReqQueue_.erase(it);
				break;
			}
		}
	}

	bool CombinThreadComit::checkMaybelockQueue()
	{
		bool ret = false;
		std::unique_lock<std::mutex> lock(maybeLockReqQueue_Mutex_);
		if (!maybeLockReqQueue_.empty())
		{
			std::shared_ptr<MaybeLockItem> item = maybeLockReqQueue_.front();
			pushProcedQueue(item->spRemote_Req_->getID(), item->spRemote_Req_->getAtom());
			item->spRemote_Req_->setProcTimeout(true);

			Pickle pick;
			Instruct::SerializationInstruct(item->spRemote_Req_.get(), pick);
			TmpSwapData* tmpdata = new TmpSwapData;
			tmpdata->obj_ = this;
			tmpdata->len_ = pick.size();
			tmpdata->buf_ = new unsigned char[tmpdata->len_];
			memcpy_s(tmpdata->buf_, tmpdata->len_,
				static_cast<const unsigned char*>(pick.data()), pick.size());
#ifdef _DEBUG1
			char szTmp[256] = { 0 };
			sprintf_s(szTmp, "----proc may lock queue name = %s ; id = %d ; new = %d ; theadID=%d ; %s\n", item->spRemote_Req_->getName().c_str(),
				item->spRemote_Req_->getID(), item->spRemote_Req_->newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
#endif
			unsigned int id;
			HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, ProcForkReq, tmpdata, 0, &id);
			CloseHandle(hThread);

			maybeLockReqQueue_.pop_front();
			ret = true;
		}
		return ret;
	}

	void CombinThreadComit::pushProcedQueue(int id, int atom)
	{
		std::unique_lock<std::mutex> lock(hasProcedQueue_Mutex_);
		MaybeProcItem item(id, atom);
		hasProcedQueue_.push_back(item);
	}

	bool CombinThreadComit::removeProcedQueue(int id, int atom)
	{
		bool ret = false;
		std::unique_lock<std::mutex> lock(hasProcedQueue_Mutex_);
		std::deque<MaybeProcItem>::iterator it = hasProcedQueue_.begin();
		for (; it != hasProcedQueue_.end(); ++it)
		{
			if (it->reqContext_.id_ == id && it->reqContext_.atom_ == atom)
			{
				hasProcedQueue_.erase(it);
				ret = true;
				break;
			}
		}
		return ret;
	}

	bool CombinThreadComit::hitProcedQueue(int id, int atom)
	{
		bool ret = false;
		std::unique_lock<std::mutex> lock(hasProcedQueue_Mutex_);
		std::deque<MaybeProcItem>::iterator it = hasProcedQueue_.begin();
		for (; it != hasProcedQueue_.end(); ++it)
		{
			if (it->reqContext_.id_ == id && it->reqContext_.atom_ == atom)
			{
				ret = it->hitProc_;
				if (it->hitProc_ == false)
				{
					it->hitProc_ = true;
				}
				if (ret){
					hasProcedQueue_.erase(it);
#ifdef _DEBUG1
					char szTmp[256] = { 0 };
					sprintf_s(szTmp, "----has proc request  id = %d ; atom = %d; theadID=%d ; %s\n", id,
						atom, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
					OutputDebugStringA(szTmp);
#endif
				}
				break;
			}
		}
		return ret;
	}

	void CombinThreadComit::checkMaybeLostInstruct(std::shared_ptr<Instruct>& spReq)
	{
		Pickle pick;
		Instruct::SerializationInstruct(spReq.get(), pick);
		TmpSwapData* tmpdata = new TmpSwapData;
		tmpdata->obj_ = this;
		tmpdata->len_ = pick.size();
		tmpdata->buf_ = new unsigned char[tmpdata->len_];
		memcpy_s(tmpdata->buf_, tmpdata->len_,
			static_cast<const unsigned char*>(pick.data()), pick.size());
#ifdef _DEBUG1
		char szTmp[256] = { 0 };
		sprintf_s(szTmp, "----proc lost instruct name = %s ; id = %d ; new = %d ; theadID=%d ; %s\n", spReq->getName().c_str(),
			spReq->getID(), spReq->newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
		OutputDebugStringA(szTmp);
#endif
		unsigned int id;
		HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, ProcForkReq, tmpdata, 0, &id);
		CloseHandle(hThread);
	}

	void CombinThreadComit::Response(IPCUnit* ipc, std::shared_ptr<Instruct> resp, const int& req_id, const int& req_atom)
	{
		assert(ipc);
		if ( !ipc )
		{
			return;
		}
		//std::unique_lock<std::mutex> lock(newSessinBlockMutex_);
		/*eventResponseStackMutex_.lock();
		if (!eventResponsStack_.empty())
		{
			responseID = eventResponsStack_.front();
		}
		eventResponseStackMutex_.unlock();
		assert(responseID!=0);*/
		resp->setID(req_id);
		resp->setAtom(req_atom);
		resp->setInstructType(InstructType::INSTRUCT_RESPONSE);
		resp->setProcState(PROC_STATE_FIN);
		popRecvRequestID(req_id, req_atom);
		//newSessinBlockMutex_.unlock();
		Pickle pick;
		Instruct::SerializationInstruct(resp.get(), pick);
		//pick.data(), pick.size()
		ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		//checkPendingReq();
	}

	std::shared_ptr<RequestContext> CombinThreadComit::getReqStackNearlTopID(int id)
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		std::shared_ptr<RequestContext> ret;

		/*if (!eventRequestStack_.empty()){
			ret = eventRequestStack_.front();
			eventRequestStack_.pop_front();
		}*/
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

	std::shared_ptr<RequestContext> CombinThreadComit::triggerReqStackID(int id, int atom)
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		std::shared_ptr<RequestContext> ret;

		std::deque<std::shared_ptr<RequestContext>>::iterator it = eventRequestStack_.begin();
		for (; it != eventRequestStack_.end(); ++it){
			if (it->get()->id_ == id && it->get()->atom_ == atom)
			{
				//it->get()->single = true;
				ret = *it;
				break;
			}
		}
		return ret;
	}

	std::shared_ptr<RequestContext> CombinThreadComit::getReqStackTop()
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		std::shared_ptr<RequestContext> ret;

		if (!eventRequestStack_.empty()){
			ret = eventRequestStack_.front();
		}
		return ret;
	}

	bool CombinThreadComit::isSameMyReqID(int id)
	{
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		bool ret = true;
		if (!eventRequestStack_.empty()){
			ret = eventRequestStack_.front()->id_ == id;

#ifdef _DEBUG1
			if (!ret)
			{
				OutputDebugStringW(L"----reqID no match 1111 ==========");
			}			
#endif
		}
		//assert(ret);
		return ret;
	}

	void CombinThreadComit::RegisertBrowserHelp(std::shared_ptr<Instruct> spInfo)
	{
		std::unique_lock<std::mutex> lock(sendRegBrowserQueue_Mutex_);
		if (!CefCurrentlyOn(TID_UI)){
			CefPostTask(TID_UI, base::Bind(&UIThreadCombin::RegisertBrowserHelp, this, spInfo));
			return;
		}
		sendRegBrowserQueue_.push_back(spInfo->getID());
		bool ret = false;
		std::wstring szSrvPipe = spInfo->getList().GetWStrVal(0);
		std::wstring szCliPipe = spInfo->getList().GetWStrVal(1);
		int ipcID = cyjh::IPC_Manager::getInstance().MatchIpc(szSrvPipe.c_str(), szCliPipe.c_str());
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(spInfo->getBrowserID());
		//OutputDebugString(L"---- rsp_RegisterBrowser succ");
		if (item.get())
		{
			item->m_ipcID = ipcID;
			ret = true;
		}
		else{
			assert(false);
			//OutputDebugString(L"----------rsp_RegisterBrowser fail");
		}

#ifdef _DEBUG1
		char szTmp[256] = { 0 };
		sprintf_s(szTmp, "----reg browser = %d ; theadID=%d ; %s\n", spInfo->getBrowserID(),
			GetCurrentThreadId(),  threadType_ == THREAD_UI ? "ui" : "render");
		OutputDebugStringA(szTmp);
#endif

		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spInfo->getName().c_str());
		spOut->setBrowserID(spInfo->getBrowserID());
		spOut->setSucc(ret);
		/*int ipcID = 0;
		CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(spInfo->getBrowserID());
		if (item.get())
		{
			ipcID = item->m_ipcID;
		}*/
		std::shared_ptr<IPCUnit> ipc = IPC_Manager::getInstance().GetIpc(ipcID);
		if ( ipc.get() )
		{
			spOut->setID(spInfo->getID());
			spOut->setAtom(spInfo->getAtom());
			spOut->setInstructType(InstructType::INSTRUCT_RESPONSE);
			spOut->setProcState(PROC_STATE_FIN);
			Pickle pick;
			Instruct::SerializationInstruct(spOut.get(), pick);
			//pick.data(), pick.size()
			ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		}
		else{
			assert(false);
		}
		sendRegBrowserQueue_.pop_front();
	}

	void CombinThreadComit::SendRenderWakeUpHelp(int browserID/*, int reqid, int atom*/)
	{
		std::unique_lock<std::mutex> lock(sendRenderImmedNoBlockQueue_Mutex_);
		if (!CefCurrentlyOn(TID_UI)){
			CefPostTask(TID_UI, base::Bind(&CombinThreadComit::SendRenderWakeUpHelp, this, browserID/*, reqid, atom*/));
			return;
		}
		sendRenderImmedNoBlockQueue_.push_back(browserID);
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowser(browserID);
		int ipcID = 0;
		if (browser.get())
		{
			CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
			if (item.get())
			{
				ipcID = item->m_ipcID;
			}
		}
		std::shared_ptr<IPCUnit> ipc = IPC_Manager::getInstance().GetIpc(ipcID);
		if (ipc.get())
		{
			std::shared_ptr<Instruct> spOut(new Instruct);
			spOut->setInstructType(InstructType::INSTRUCT_WAKEUP);
			Pickle pick;
			Instruct::SerializationInstruct(spOut.get(), pick);
			ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		}
		//popRecvRequestID(reqid, atom);
		sendRenderImmedNoBlockQueue_.pop_front();
	}

	bool CombinThreadComit::RegisterReqID(IPCUnit* ipc, const int browser_id, const int req_id)
	{
		//先询问是否可以请求
		bool ret = false;
		if ( threadType_ == THREAD_RENDER )
		{
			/*cyjh::Instruct parm;
			parm.setBrowserID(browser_id);
			parm.setID(req_id);
			parm.setInstructType(InstructType::INSTRUCT_INQUEUE);
			Pickle pick;
			Instruct::SerializationInstruct(&parm, pick);
			ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);*/
		}else if ( threadType_ == THREAD_UI )
		{
			if (MainProcQueue::getInst().pushReq(0, req_id)){
				return true;
			}
		}
		//assert(blockThread_);
		//blockThread_->block();
		return ret;
	}

	void CombinThreadComit::UnRegisterReqID(IPCUnit* ipc, int req_id)
	{
		if (threadType_ == THREAD_RENDER)
		{
			cyjh::Instruct parm;
			parm.setID(req_id);
			parm.setInstructType(InstructType::INSTRUCT_OUTQUEUE);
			Pickle pick;
			Instruct::SerializationInstruct(&parm, pick);
			ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		}
		else if (threadType_ == THREAD_UI)
		{
			MainProcQueue::getInst().popReq(req_id);
		}
	}

	void CombinThreadComit::WakeUp()
	{
		blockThread_->WakeUp();
	}

	void procRecvDataHelp(std::shared_ptr<Instruct> spInfo)
	{
	}

	void CombinThreadComit::RecvData(const unsigned char* data, DWORD len)
	{
		cyjh::Pickle pick(reinterpret_cast<const char*>(data), len);
		std::shared_ptr<Instruct> spInstruct(new Instruct());
		bool objected = Instruct::ObjectInstruct(pick, spInstruct.get()); //对像化
		assert(objected);

		std::unique_lock<std::mutex> lock(newSessinBlockMutex_);
		//newSessinBlockMutex_.lock();
		if (spInstruct->getInstructType() == INSTRUCT_OUTQUEUE)
		{
			assert(threadType_ == THREAD_UI);
			int req = spInstruct->getID();
			MainProcQueue::getInst().popReq(req);
			//newSessinBlockMutex_.unlock();
			return;
		}

		//newSessinBlockMutex_.lock();
		if (spInstruct->getInstructType() == INSTRUCT_WAKEUP)
		{
			assert(threadType_ == THREAD_RENDER);
#ifdef _DEBUG1
			OutputDebugStringW(L"-----wake up");
#endif
			WakeUp();
			//newSessinBlockMutex_.unlock();
			return;
		}

		//newSessinBlockMutex_.lock();
		if (spInstruct->getInstructType() == INSTRUCT_INQUEUE)
		{
			assert(threadType_ == THREAD_UI);
			int browserid = spInstruct->getBrowserID();
			int req = spInstruct->getID();
			/*if (MainProcQueue::getInst().pushReq(browserid, req))
			{
			//通知渲染线程可以直接向下执行 wake up
			//CefPostTask(TID_UI, base::Bind(&CombinThreadComit::SendRenderWakeUpHelp, this, browserid));
			this->SendRenderWakeUpHelp(browserid);
			}*/
			if (!MainProcQueue::getInst().pushReq(browserid, req))
			{
				//newSessinBlockMutex_.unlock();
				return;
			}
			//成功立可向渲染线程发送解除block
			//return;
		}
		if (!blockThread_->ProcTrunk(spInstruct)){
#ifdef _DEBUG1
			//OutputDebugStringW(L"-----no block");
#endif			
			ProcTrunkReq(spInstruct);
		}
		else{
#ifdef _DEBUG1
			OutputDebugStringW(L"-----in block");
#endif			
		}
		//newSessinBlockMutex_.unlock();
	}

	void CombinThreadComit::ProcTrunkReq(std::shared_ptr<Instruct> spInstruct)
	{
		if (spInstruct->getInstructType() == INSTRUCT_INQUEUE)
		{
			//assert(threadType_ == THREAD_UI);
			//pushRecvRequestID(spInstruct->getID(), spInstruct->getAtom());
			int browserid = spInstruct->getBrowserID();
			//int req = spInstruct->getID();
			//if (MainProcQueue::getInst().pushReq(browserid, req))
			{
				//通知渲染线程可以直接向下执行 wake up
				//CefPostTask(TID_UI, base::Bind(&CombinThreadComit::SendRenderWakeUpHelp, this, browserid));				
				this->SendRenderWakeUpHelp(browserid);
			}
			return;
		}

		/*if (spInstruct->getInstructType() == INSTRUCT_OUTQUEUE)
		{
			assert(threadType_ == THREAD_UI);
			int req = spInstruct->getID();
			MainProcQueue::getInst().popReq(req);
			return;
		}*/

		/*if (spInstruct->getInstructType() == INSTRUCT_WAKEUP)
		{
			assert(threadType_ == THREAD_RENDER);
			WakeUp();
			return;
		}*/

		//正式处理流程
		if (//spInstruct->getName().compare("RegisterBrowser") == 0 &&
			spInstruct->getInstructType() == INSTRUCT_REGBROWSER)
		{
			assert(threadType_ == THREAD_UI);
			//CefPostTask(TID_UI, base::Bind(&CombinThreadComit::RegisertBrowserHelp, this, spInstruct));
			this->RegisertBrowserHelp(spInstruct);
			return;
		}

#ifdef _SINGLE_INSTRUCT_PROC
		bool match = isSameMyReqID(spInstruct->getID());
		if (!match){
			//std::shared_ptr<ReqInfo> info(new ReqInfo());
			//info->set(data, len);
			//requestQueue_.SubmitPack(info);
			//return;
#ifdef _DEBUG1
			//OutputDebugStringW(L"----reqID no match 2==========");
#endif			
			spInstruct->setProcState(PROC_STATE_REJ);
			RejectReq(spInstruct);
			//assert(match);
			return;
		}
#else
#endif

		std::shared_ptr<RequestContext> top;
#ifdef _SINGLE_INSTRUCT_PROC
		top = getReqStackNearlTopID(spInstruct->getID());
#else
		if (spInstruct->getInstructType() == INSTRUCT_RESPONSE)
		{
			//top = getReqStackNearlTopID(spInstruct->getID());
			top = triggerReqStackID(spInstruct->getID(), spInstruct->getAtom());
#ifdef _DEBUG1
			char szTmp[256] = { 0 };
			sprintf_s(szTmp, "-----recv response name = %s ; id = %d ; new = %d ; theadID=%d ; %s\n", spInstruct->getName().c_str(),
				spInstruct->getID(), spInstruct->newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
#endif
			if ( !top.get() )
			{
				//收到的反馈没有在，自已的请求列表。
				//出现这样的原因可能是关闭的时候,主线程已经结束。所以请求列表已经不存在了
#ifdef _DEBUG1
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "----- fail recv response id = %d ; theadID=%d ; %s\n",
					spInstruct->getID(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szTmp);
#endif
				assert(top.get());
				return;
			}			

		}
		else if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
		{
			top = getReqStackTop();
		}
#endif

#ifdef _SINGLE_INSTRUCT_PROC
#else
		//先叛断指令是否有在待处理序列中，如果有检查是否是第一个要处理的。否则进入待处理队列。
		//这个是保证指令没有乱序的现像
		/*if ( !isEmptyPengingReq())
		{
			if (!matchRecvRequestID(spInstruct->getID()))
			{
				pushPengingRequest(spInstruct);
				return;
			}
		}*/

		/*if ( !matchRecvRequestID(spInstruct->getID()))
		{
			pushPengingRequest(spInstruct);
			return;
		}*/
#endif
		if (top.get())
		{
			if (spInstruct->getInstructType() == INSTRUCT_RESPONSE)
			{
#ifdef _DEBUG1
				bool match = (top->id_ == spInstruct->getID()) && (top->atom_ == spInstruct->getAtom());
				if (!match)
				{
					int i = 0;
				}
				assert(match);
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "----- tigle response event id = %d ; theadID=%d ; %s\n",
					spInstruct->getID(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szTmp);
#endif				
				top->outval_ = spInstruct;
				top->single_ = true;
				popRequestEvent(top->id_, top->atom_); //从主线程移过来处理
				SetEvent(top->events_[0]);
			}
			else if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				//从主线程移这里处理，因为当数据接收线程处理很快时，会出现两次接收到请求事件（在主线程阻塞的情况下）
				//pushRecvRequestID(spInstruct->getID(), spInstruct->getAtom()); //!!必顺移到ui线程或render线程中处理					
#ifdef _DEBUG1
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "----recv req in block name = %s ; id = %d ; theadID=%d ; %s\n", spInstruct->getName().c_str(),
					spInstruct->getID(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szTmp);
#endif
				top->parm_ = spInstruct;
				top->bResponse_ = false;
				SetEvent(top->events_[1]);
			}
		}
		else{
			if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				
				if (spInstruct->procTimeout())
				{
					//这里因为有可能用户在调试代码的情况,不作处理
					removeProcedQueue(spInstruct->getID(), spInstruct->getAtom());
					assert(spInstruct->procTimeout());
					return;
				}
				else{
				}
				//pushRecvRequestID(spInstruct->getID(), spInstruct->getAtom()); //!!必顺移到ui线程或render线程中处理
				pushMaybelockQueue(spInstruct);
				procRecvRequest(spInstruct);
			}
		}
	}
}