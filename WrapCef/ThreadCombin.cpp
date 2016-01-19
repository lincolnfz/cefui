#include "stdafx.h"
#include "ThreadCombin.h"
#include "WebViewFactory.h"
#include "BrowserIdentifier.h"

namespace cyjh{

	UIThreadCombin::UIThreadCombin() :CombinThreadComit(THREAD_UI)
	{

	}


	UIThreadCombin::~UIThreadCombin()
	{

	}


	void UIThreadCombin::procRecvRequest(const std::shared_ptr<Instruct> spReq)
	{
		CombinThreadComit::procRecvRequest(spReq);
		CefRefPtr<CefBrowser> browser = WebViewFactory::getInstance().GetBrowser(spReq->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		bool bOut = handle_.handleQuest(browser, spReq, spOut);
		spReq->setSucc(bOut);
		spOut->setBrowserID(spReq->getBrowserID());
		Response(spOut, spReq->getID());
	}







	////////////RenderThreadCombin//////
	/////й╣ож
	RenderThreadCombin::RenderThreadCombin() :CombinThreadComit(THREAD_RENDER)
	{

	}

	RenderThreadCombin::~RenderThreadCombin()
	{

	}

	void RenderThreadCombin::procRecvRequest(const std::shared_ptr<Instruct> spReq)
	{
		CombinThreadComit::procRecvRequest(spReq);
		CefRefPtr<CefBrowser> browser = BrowserIdentifier::GetInst().GetBrowser(spReq->getBrowserID());
		std::shared_ptr<Instruct> spOut(new Instruct);
		bool bOut = handle_.handleQuest(browser, spReq, spOut);
		spReq->setSucc(bOut);
		spOut->setBrowserID(spReq->getBrowserID());
		Response(spOut, spReq->getID());
	}

}