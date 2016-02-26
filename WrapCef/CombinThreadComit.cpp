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

	DWORD  CombinThreadComit::s_tid_ = 0;
	CombinThreadComit::CombinThreadComit(ThreadType type) //:requestQueue_(this)
	{
		threadType_ = type;
		requestID_ = 0;
		blockThread_ = NULL;
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

	bool CombinThreadComit::popRequestEvent(int reqid)
	{
		bool ret = false;
		std::unique_lock<std::mutex> lock(eventRequestStackMutex_);
		std::deque<std::shared_ptr<RequestContext>>::iterator it = eventRequestStack_.begin();
		for ( ; it != eventRequestStack_.end(); ++it )
		{
			std::shared_ptr<RequestContext> context = *it;
			if ( context->id_ == reqid )
			{
#ifdef _DEBUG
				//std::shared_ptr<RequestContext> context = eventRequestStack_.front();
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "---- popReq id = %d , theadID=%d ; %s\n", context->id_, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
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

	void CombinThreadComit::pushRecvRequestID(int id, int atom)
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
#ifdef _DEBUG
		char szTmp[256] = { 0 };
		sprintf_s(szTmp, "---- pushRecvReq id = %d, theadID=%d ; %s\n", id, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
		OutputDebugStringA(szTmp);
#endif
		RecvReqItem item(id, atom);
#ifdef _SINGLE_INSTRUCT_PROC
		eventResponsStack_.push_front(item);
#else
		eventResponsStack_.push_back(item);
#endif
		
	}

	bool CombinThreadComit::popRecvRequestID(int id, int atom)
	{
		std::unique_lock<std::mutex> lock(eventResponseStackMutex_);
#ifdef _DEBUG
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
#ifdef _DEBUG
				char szTmp[256] = { 0 };
				sprintf_s(szTmp, "---- popResponse id = %d, theadID=%d ; %s\n", id, GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
				OutputDebugStringA(szTmp);
#endif
				break;
			}
		}
		//eventResponsStack_.pop_front();
		assert(ret);
		return ret;
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

	void CombinThreadComit::procRecvRequest(const std::shared_ptr<Instruct> parm)
	{
		pushRecvRequestID(parm->getID(), parm->getAtom());
	}

	void CombinThreadComit::SendRequest(IPCUnit* ipc, Instruct& parm, std::shared_ptr<Instruct>& response_val)
	{
		//requestQueue_.ResetEvent();
		newSessinBlockMutex_.lock();
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

#ifdef _DEBUG
		if (parm.getName().compare("invokedJSMethod") == 0)
		{
			char szTmp[8192] = { 0 };
			sprintf_s(szTmp, "----name = %s ; %s | %s | %s ; id = %d ; new = %d ; theadID=%d ; %s\n", parm.getName().c_str(),
				parm.getList().GetStrVal(0).c_str(), parm.getList().GetStrVal(1).c_str(), parm.getList().GetStrVal(2).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
		}
		else if (parm.getName().compare("crossInvokeWebMethod") == 0)
		{
			WCHAR szTmp[8192] = { 0 };
			swprintf_s(szTmp, L"----name = %s ; %s | %s | %s ; id = %d ; new = %d ; theadID=%d ; %s\n", L"crossInvokeWebMethod",
				parm.getList().GetWStrVal(1).c_str(), parm.getList().GetWStrVal(2).c_str(), parm.getList().GetWStrVal(3).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? L"ui" : L"render");
			OutputDebugStringW(szTmp);
		}
		else if (parm.getName().compare("invokeMethod") == 0)
		{
			WCHAR szTmp[8192] = { 0 };
			swprintf_s(szTmp, L"----name = %s ; %s | %s | %s ; id = %d ; new = %d ; theadID=%d ; %s\n", L"invokeMethod",
				parm.getList().GetWStrVal(0).c_str(), parm.getList().GetWStrVal(1).c_str(), parm.getList().GetWStrVal(2).c_str(),
				parm.getID(), parm.newSession(), GetCurrentThreadId(), threadType_ == THREAD_UI ? L"ui" : L"render");
			OutputDebugStringW(szTmp);
		}
		else if (parm.getName().compare("crossInvokeWebMethod2") == 0)
		{
			WCHAR szTmp[8192] = { 0 };
			swprintf_s(szTmp, L"----name = %s ; %s | %s | %s ; id = %d ; new = %d ; theadID=%d ; %s\n", L"crossInvokeWebMethod2",
				parm.getList().GetWStrVal(2).c_str(), parm.getList().GetWStrVal(3).c_str(), parm.getList().GetWStrVal(4).c_str(),
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
#ifdef _DEBUG
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

#ifdef _DEBUG
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
		assert(response_val.get());
		popRequestEvent(sp->id_);
		if (parm.getInstructType() != INSTRUCT_REGBROWSER && parm.newSession()){
			//UnRegisterReqID(ipc, reqeustid);
		}
		//requestQueue_.SetEvent();		
#ifdef _DEBUG
		if (response_val->getProcState() != PROC_STATE_FIN)
		{
			char szTmp[8192] = { 0 };
			sprintf_s(szTmp, "---- !!!!reject name = %s ; id = %d ; new = %d ; theadID=%d ; procstate=%d ; %s\n", response_val->getName().c_str(),
				response_val->getID(), response_val->newSession(), GetCurrentThreadId(), response_val->getProcState(), threadType_ == THREAD_UI ? "ui" : "render");
			OutputDebugStringA(szTmp);
		}
#endif		
	}

	void CombinThreadComit::Response(IPCUnit* ipc, std::shared_ptr<Instruct> resp, const int& req_id, const int& req_atom)
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
		resp->setAtom(req_atom);
		resp->setInstructType(InstructType::INSTRUCT_RESPONSE);
		resp->setProcState(PROC_STATE_FIN);
		popRecvRequestID(req_id, req_atom);		
		Pickle pick;
		Instruct::SerializationInstruct(resp.get(), pick);
		//pick.data(), pick.size()
		ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
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

#ifdef _DEBUG
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
#ifdef _DEBUG
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
#ifdef _DEBUG
			//OutputDebugStringW(L"-----no block");
#endif			
			ProcTrunkReq(spInstruct);
		}
		else{
#ifdef _DEBUG
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
#ifdef _DEBUG
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
			top = getReqStackNearlTopID(spInstruct->getID());
			assert(top.get());
		}
		else if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
		{
			top = getReqStackTop();
		}
#endif
		if (top.get())
		{
			if (spInstruct->getInstructType() == INSTRUCT_RESPONSE)
			{
#ifdef _DEBUG
				bool match = (top->id_ == spInstruct->getID()) && (top->atom_ == spInstruct->getAtom());
				if (!match)
				{
					int i = 0;
				}
				assert(match);
#endif				
				top->outval_ = spInstruct;
				SetEvent(top->events_[0]);
			}
			else if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				top->parm_ = spInstruct;
				//pushRecvRequestID(spInstruct->getID(), spInstruct->getAtom()); //移到ui线程或render线程中处理
				SetEvent(top->events_[1]);
			}
		}
		else{
			if (spInstruct->getInstructType() == INSTRUCT_REQUEST)
			{
				//pushRecvRequestID(spInstruct->getID(), spInstruct->getAtom()); //移到ui线程或render线程中处理
				procRecvRequest(spInstruct);
			}
		}
	}
}