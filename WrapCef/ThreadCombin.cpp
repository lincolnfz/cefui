#include "stdafx.h"
#include "ThreadCombin.h"
#include "WebViewFactory.h"
#include "BrowserIdentifier.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

namespace cyjh{

	UIThreadCombin::UIThreadCombin() :CombinThreadComit(THREAD_UI)
	{

	}


	UIThreadCombin::~UIThreadCombin()
	{

	}

	void UIThreadCombin::Request(CefRefPtr<CefBrowser> browser, Instruct& parm, std::shared_ptr<Instruct> val)
	{
		parm.setBrowserID(browser->GetIdentifier());
	}

	void UIThreadCombin::RecvData(const unsigned char* data, DWORD len)
	{
		CombinThreadComit::RecvData(data, len);
	}

	void UIThreadCombin::procRecvRequest(const std::shared_ptr<Instruct> spReq)
	{
		if (!CefCurrentlyOn(TID_UI)){
			CefPostTask(TID_UI, base::Bind(&UIThreadCombin::procRecvRequest, this, spReq));
			return;
		}
		CombinThreadComit::procRecvRequest(spReq);
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowser(spReq->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spReq->getName().c_str());
		spOut->setBrowserID(spReq->getBrowserID());
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
		}
		std::shared_ptr<IPCUnit> ipc = IPC_Manager::getInstance().GetIpc(ipcID);
		Response(ipc.get(), spOut, spReq->getID());
		
		//Response(spOut, spReq->getID());
	}







	////////////RenderThreadCombin//////
	/////ʵ��
	RenderThreadCombin::RenderThreadCombin() :CombinThreadComit(THREAD_RENDER)
	{

	}

	RenderThreadCombin::~RenderThreadCombin()
	{

	}

	void RenderThreadCombin::Request(CefRefPtr<CefBrowser> browser, Instruct& parm, std::shared_ptr<Instruct> val)
	{
		parm.setBrowserID(browser->GetIdentifier());
		SendRequest(ipc_.get(), parm, val);
	}

	void RenderThreadCombin::RecvData(const unsigned char* data, DWORD len)
	{
		CombinThreadComit::RecvData(data, len);
	}

	void RenderThreadCombin::procRecvRequest(const std::shared_ptr<Instruct> spReq)
	{
		if (!CefCurrentlyOn(TID_RENDERER)){
			CefPostTask(TID_RENDERER, base::Bind(&RenderThreadCombin::procRecvRequest, this, spReq));
			return;
		}		
		CombinThreadComit::procRecvRequest(spReq);
		CefRefPtr<CefBrowser> browser = BrowserIdentifier::GetInst().GetBrowser(spReq->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		spOut->setName(spReq->getName().c_str());
		spOut->setBrowserID(spReq->getBrowserID());
		bool bOut = handle_.handleQuest(browser, spReq, spOut);
		spOut->setSucc(bOut);		
		//Response(spOut, spReq->getID());
		Response(ipc_.get(),spOut, spReq->getID());
	}

}