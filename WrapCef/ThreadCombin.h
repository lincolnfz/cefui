#ifndef THREAD_COMBIN_H
#define THREAD_COMBIN_H
#pragma once

#include <set>
#include "CombinThreadComit.h"
#include "ResponseRender.h"
#include "ResponseUI.h"
#include "IPC.h"
//#include "BlockThread.h"

namespace cyjh{

	class UIThreadCombin : public CombinThreadComit
	{
	public:
		UIThreadCombin();
		virtual ~UIThreadCombin();
		virtual void Request(CefRefPtr<CefBrowser>, Instruct& parm, std::shared_ptr<Instruct>& val) override;
		virtual void AsyncRequest(CefRefPtr<CefBrowser>, Instruct& parm) override;
		virtual void RecvData(const unsigned char*, DWORD) override;
		virtual void postInstruct(std::shared_ptr<Instruct> spInfo) override;
		void DisableSendBrowser(int id);
	protected:
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) override;
		virtual void procRecvData(const std::shared_ptr<Instruct>) override;

		virtual void RejectReq(std::shared_ptr<Instruct> spInfo) override;

		void RejectReqHelp(std::shared_ptr<Instruct> spInfo);

	protected:
		ResponseUI handle_;
		UIBlockThread block_;
		std::set<int> disableBrowserSet_;
		IMPLEMENT_REFCOUNTING(UIThreadCombin);
	};


	class RenderThreadCombin : public CombinThreadComit
	{
	public:
		RenderThreadCombin();
		virtual ~RenderThreadCombin();
		virtual void AsyncRequest(CefRefPtr<CefBrowser>, Instruct& parm) override;
		virtual void Request(CefRefPtr<CefBrowser>, Instruct& parm, std::shared_ptr<Instruct>& val) override;
		void SetIpc(std::shared_ptr<IPCUnit> ipc)
		{
			ipc_ = ipc;
		}

		const std::shared_ptr<IPCUnit> getIpc() const{
			return ipc_;
		}

		virtual void RecvData(const unsigned char*, DWORD) override;
		virtual void postInstruct(std::shared_ptr<Instruct> spInfo) override;

		void AttachNewBrowserIpc();
		void DetchBrowserIpc();
	protected:
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) override;
		virtual void procRecvData(const std::shared_ptr<Instruct>) override;
		virtual void RejectReq(std::shared_ptr<Instruct> spInfo) override;
		void RejectReqHelp(std::shared_ptr<Instruct> spInfo);

		virtual void CloseIpc(std::shared_ptr<Instruct> spInfo) override;
		void CloseBrowserHelp(std::shared_ptr<Instruct> spInfo, int browserID);

		std::shared_ptr<IPCUnit> ipc_;

	protected:
		ResponseRender handle_;
		RenderBlockThread block_;
		bool bNeedClose_, bClosed_;
		std::shared_ptr<Instruct> spCloseInstruct_;
		std::set<int> disableBrowserSet_;
		IMPLEMENT_REFCOUNTING(RenderThreadCombin);
	};

}
#endif