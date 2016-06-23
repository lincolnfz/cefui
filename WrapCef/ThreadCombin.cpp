#include "stdafx.h"
#include "ThreadCombin.h"
#include "WebViewFactory.h"
#include "BrowserIdentifier.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "WebViewFactory.h"
#include "MainProcQueue.h"
#include "NormalWebFactory.h"

namespace cyjh{

	UIThreadCombin::UIThreadCombin() :CombinThreadComit(THREAD_UI), block_(this)
	{
		blockThread_ = &block_;
		MainProcQueue::getInst().setCombinThread(this);
	}


	UIThreadCombin::~UIThreadCombin()
	{

	}

	void UIThreadCombin::AsyncRequest(CefRefPtr<CefBrowser> browser, Instruct& parm)
	{
		CEF_REQUIRE_UI_THREAD();
		parm.setBrowserID(browser->GetIdentifier());
		std::shared_ptr<IPCUnit> unit;
		CefRefPtr<WebItem>item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
		if (item.get())
		{
			unit = IPC_Manager::getInstance().GetIpc(item->m_ipcID);		
		}
		else{
			CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
			if ( control.get() )
			{
				unit = IPC_Manager::getInstance().GetIpc(control->getIpcID());
			}
		}

		if (unit.get())
		{
			SendAsyncRequest(unit.get(), parm);
		}
	}

	void UIThreadCombin::Request(CefRefPtr<CefBrowser> browser, Instruct& parm, std::shared_ptr<Instruct>& val)
	{
		CEF_REQUIRE_UI_THREAD();
		parm.setBrowserID(browser->GetIdentifier());
		CefRefPtr<WebItem>item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
		if ( item.get() )
		{
			std::shared_ptr<IPCUnit> unit = IPC_Manager::getInstance().GetIpc(item->m_ipcID);
			if ( unit.get() )
			{
				SendRequest(unit.get(), parm, val);
			}
		}
	}

	void UIThreadCombin::RecvData(const unsigned char* data, DWORD len)
	{
		CombinThreadComit::RecvData(data, len);
	}

	void UIThreadCombin::postInstruct(std::shared_ptr<Instruct> spInfo){
		CefPostTask(TID_UI, base::Bind(&UIThreadCombin::procRecvRequest, this, spInfo));
	}

	void UIThreadCombin::procRecvData(const std::shared_ptr<Instruct> spData)
	{
		if (!CefCurrentlyOn(TID_UI)){
			CefPostTask(TID_UI, base::Bind(&UIThreadCombin::procRecvData, this, spData));
			return;
		}
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowser(spData->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spData->getName().c_str());
		spOut->setBrowserID(spData->getBrowserID());
		if (browser.get() == NULL)
		{
			browser = NormalWebFactory::getInstance().GetBrowser(spData->getBrowserID());
			if (browser.get() == NULL)
			{
				assert(false);
				return;
			}
		}
		bool bOut = handle_.handleQuest(browser, spData, spOut);
	}

	void UIThreadCombin::procRecvRequest(const std::shared_ptr<Instruct> spReq)
	{
		if (!CefCurrentlyOn(TID_UI)){
			CefPostTask(TID_UI, base::Bind(&UIThreadCombin::procRecvRequest, this, spReq));
			return;
		}
#ifdef _DEBUG1
		OutputDebugStringA("---calc begin");
#endif
		//CombinThreadComit::procRecvRequest(spReq);
		if (!CombinThreadComit::prepareResponse(spReq)){
			return;
		}
#ifdef _DEBUG1
		char szTmp[256] = { 0 };
		sprintf_s(szTmp, "----calc response data name = %s ; id = %d ; browser = %d ; theadID=%d ; %s\n", spReq->getName().c_str(),
			spReq->getID(), spReq->getBrowserID(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
		OutputDebugStringA(szTmp);
#endif
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowser(spReq->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spReq->getName().c_str());
		spOut->setBrowserID(spReq->getBrowserID());
		if ( browser.get() == NULL )
		{
			browser = NormalWebFactory::getInstance().GetBrowser(spReq->getBrowserID());
			/*spOut->setSucc(false);
			if ( spReq->getName().compare("RegisterBrowser") == 0 )
			{
				std::wstring szSrvPipe = spReq->getList().GetWStrVal(0);
				std::wstring szCliPipe = spReq->getList().GetWStrVal(1);
				int ipcID = cyjh::IPC_Manager::getInstance().MatchIpc(szSrvPipe.c_str(), szCliPipe.c_str());
				std::shared_ptr<IPCUnit> ipc = IPC_Manager::getInstance().GetIpc(ipcID);
				if (ipc.get())
				{
					Response(ipc.get(), spOut, spReq->getID(), spReq->getAtom());
				}
			}
			return;*/
			if ( browser.get() == NULL )
			{				
				popRecvRequestID(spReq->getID(), spReq->getAtom());
				assert(false);
				return;
			}
		}
		bool bOut = handle_.handleQuest(browser, spReq, spOut);
		spOut->setSucc(bOut);
		int ipcID = 0;
		if ( browser.get() )
		{
			CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
			if ( item.get() )
			{
				ipcID = item->m_ipcID;
			}
			else{
				CefRefPtr<WebkitControl> control = NormalWebFactory::getInstance().GetWebkitControlByID(browser->GetIdentifier());
				if ( control.get() )
				{
					ipcID = control->getIpcID();
				}
			}
		}
		std::shared_ptr<IPCUnit> ipc = IPC_Manager::getInstance().GetIpc(ipcID);
		Response(ipc.get(), spOut, spReq->getID(), spReq->getAtom() );
		
		//Response(spOut, spReq->getID());
	}


	void UIThreadCombin::RejectReqHelp(std::shared_ptr<Instruct> spInfo)
	{
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowser(spInfo->getBrowserID());
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
			spOut->setName(spInfo->getName().c_str());
			spOut->setBrowserID(spInfo->getBrowserID());
			spOut->setSucc(false);
			spOut->setID(spInfo->getID());
			spOut->setAtom(spInfo->getAtom());
			spOut->setInstructType(InstructType::INSTRUCT_RESPONSE);
			Pickle pick;
			Instruct::SerializationInstruct(spOut.get(), pick);
			ipc->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		}
		else{
			assert(false);
		}
	}

	void UIThreadCombin::RejectReq(std::shared_ptr<Instruct> spInfo)
	{
		if (!CefCurrentlyOn(TID_UI)){
			CefPostTask(TID_UI, base::Bind(&UIThreadCombin::RejectReqHelp, this, spInfo));
			return;
		}
		RejectReqHelp(spInfo);
	}


	////////////RenderThreadCombin//////
	/////实现
	RenderThreadCombin::RenderThreadCombin() :CombinThreadComit(THREAD_RENDER), block_(this)
	{
		blockThread_ = &block_;
		bNeedClose_ = false;
		bClosed_ = false;
		MainProcQueue::getInst().setCombinThread(this);
	}

	RenderThreadCombin::~RenderThreadCombin()
	{

	}

	void RenderThreadCombin::AsyncRequest(CefRefPtr<CefBrowser> browser, Instruct& parm)
	{
		CEF_REQUIRE_RENDERER_THREAD();
	}

	void RenderThreadCombin::Request(CefRefPtr<CefBrowser> browser, Instruct& parm, std::shared_ptr<Instruct>& val)
	{
		CEF_REQUIRE_RENDERER_THREAD();
		/*if ( bNeedClose_ )
		{
			//当前已收到关闭请求,如果是新的请求不在发送。只允许发送已存在的响应
			if ( !haveResponse() )
			{
				std::shared_ptr<Instruct> tmp(new Instruct);
				tmp->setSucc(false);
				val = tmp;
				return;
			}
		}*/
		if ( disableBrowserSet_.find( browser->GetIdentifier() ) != disableBrowserSet_.end() )
		{
			std::shared_ptr<Instruct> tmp(new Instruct);
			tmp->setSucc(false);
			val = tmp;
			return;
		}

		parm.setBrowserID(browser->GetIdentifier());
		SendRequest(ipc_.get(), parm, val);

		/*if ( bNeedClose_ )
		{
			//请求完毕,尝试关闭通讯
			if ( !bClosed_ && spCloseInstruct_.get())
			{
				CloseIpcHelp(spCloseInstruct_);
			}
		}*/
	}

	void RenderThreadCombin::RecvData(const unsigned char* data, DWORD len)
	{
		CombinThreadComit::RecvData(data, len);
	}

	void RenderThreadCombin::postInstruct(std::shared_ptr<Instruct> spInfo){
		CefPostTask(TID_UI, base::Bind(&RenderThreadCombin::procRecvRequest, this, spInfo));
	}

	void RenderThreadCombin::procRecvData(const std::shared_ptr<Instruct> spData)
	{
		if (!CefCurrentlyOn(TID_RENDERER)){
			CefPostTask(TID_RENDERER, base::Bind(&RenderThreadCombin::procRecvData, this, spData));
			return;
		}
		CefRefPtr<CefBrowser> browser = BrowserIdentifier::GetInst().GetBrowser(spData->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spData->getName().c_str());
		spOut->setBrowserID(spData->getBrowserID());
		bool bOut = handle_.handleQuest(browser, spData, spOut);
	}

	void RenderThreadCombin::procRecvRequest(const std::shared_ptr<Instruct> spReq)
	{
		if (!CefCurrentlyOn(TID_RENDERER)){
			CefPostTask(TID_RENDERER, base::Bind(&RenderThreadCombin::procRecvRequest, this, spReq));
			return;
		}
#ifdef _DEBUG1
		OutputDebugStringA("---calc begin");
#endif
		if (spReq->getName().compare("closeBrowser") == 0)
		{
			//closeBrowser指令不要进栈			
			int browserID = spReq->getList().GetIntVal(0);
			disableBrowserSet_.insert(browserID);
			CloseBrowserHelp(spReq, browserID);
			return;
		}

		if (disableBrowserSet_.find(spReq->getBrowserID()) != disableBrowserSet_.end())
		{
			std::shared_ptr<Instruct> spOut(new Instruct);
			spOut->setName(spReq->getName().c_str());
			spOut->setBrowserID(spReq->getBrowserID());
			spOut->setSucc(false);
			spOut->setID(spReq->getID());
			spOut->setAtom(spReq->getAtom());
			spOut->setInstructType(InstructType::INSTRUCT_RESPONSE);
			Pickle pick;
			Instruct::SerializationInstruct(spOut.get(), pick);
			ipc_->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);

			return;
		}

		//CombinThreadComit::procRecvRequest(spReq);
		if (!CombinThreadComit::prepareResponse(spReq)){
			return;
		}
#ifdef _DEBUG1
		char szTmp[256] = { 0 };
		sprintf_s(szTmp, "----calc response data name = %s ; id = %d ; browser = %d ; theadID=%d ; %s\n", spReq->getName().c_str(),
			spReq->getID(), spReq->getBrowserID(), GetCurrentThreadId(), threadType_ == THREAD_UI ? "ui" : "render");
		OutputDebugStringA(szTmp);
#endif
		CefRefPtr<CefBrowser> browser = BrowserIdentifier::GetInst().GetBrowser(spReq->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spReq->getName().c_str());
		spOut->setBrowserID(spReq->getBrowserID());
		bool bOut = handle_.handleQuest(browser, spReq, spOut);
		spOut->setSucc(bOut);		
		//Response(spOut, spReq->getID());
		Response(ipc_.get(),spOut, spReq->getID(), spReq->getAtom());

		/*if (bNeedClose_)
		{
			//响应完毕,尝试关闭通讯
			if (!bClosed_ && spCloseInstruct_.get())
			{
				CloseIpcHelp(spCloseInstruct_);
			}
		}*/
	}

	void RenderThreadCombin::RejectReqHelp(std::shared_ptr<Instruct> spInfo)
	{
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spInfo->getName().c_str());
		spOut->setBrowserID(spInfo->getBrowserID());
		spOut->setSucc(false);
		spOut->setID(spInfo->getID());
		spOut->setAtom(spInfo->getAtom());
		spOut->setInstructType(InstructType::INSTRUCT_RESPONSE);
		Pickle pick;
		Instruct::SerializationInstruct(spOut.get(), pick);
		ipc_->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
	}

	void RenderThreadCombin::RejectReq(std::shared_ptr<Instruct> spInfo)
	{
		if (!CefCurrentlyOn(TID_RENDERER)){
			CefPostTask(TID_RENDERER, base::Bind(&RenderThreadCombin::RejectReqHelp, this, spInfo));
			return;
		}
		RejectReqHelp(spInfo);
	}

	void RenderThreadCombin::CloseIpc(std::shared_ptr<Instruct> spInfo)
	{
		//CloseIpcHelp(spInfo);
	}

	void RenderThreadCombin::CloseBrowserHelp(std::shared_ptr<Instruct> spInfo, int browserID)
	{
		if (!CefCurrentlyOn(TID_RENDERER)){
			CefPostTask(TID_RENDERER, base::Bind(&RenderThreadCombin::CloseBrowserHelp, this, spInfo, browserID));
			return;
		}
		
		/*bNeedClose_ = true;
		if ( haveRequest() || haveResponse() ){
			if ( !spCloseInstruct_.get() )
			{
				spCloseInstruct_ = spInfo;
				manTriggerReqEvent();
			}
			return;
		}

		//bClosed_ = true;*/
		manTriggerReqEvent(browserID);
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spInfo->getName().c_str());
		spOut->setBrowserID(spInfo->getBrowserID());
		spOut->setSucc(true);
		spOut->setID(spInfo->getID());
		spOut->setAtom(spInfo->getAtom());
		spOut->setInstructType(InstructType::INSTRUCT_RESPONSE);
		Pickle pick;
		Instruct::SerializationInstruct(spOut.get(), pick);
		ipc_->Send(static_cast<const unsigned char*>(pick.data()), pick.size(), 0);
		//ipc_->Close();
	}

	void RenderThreadCombin::AttachNewBrowserIpc()
	{
		ipc_->Attach();
	}

	void RenderThreadCombin::DetchBrowserIpc()
	{
		ipc_->Detch();
	}

}

