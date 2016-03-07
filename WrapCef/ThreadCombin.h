#ifndef THREAD_COMBIN_H
#define THREAD_COMBIN_H
#pragma once
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
		virtual void RecvData(const unsigned char*, DWORD) override;
		virtual void postInstruct(std::shared_ptr<Instruct> spInfo) override;
	protected:
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) override;

		virtual void RejectReq(std::shared_ptr<Instruct> spInfo) override;

		void RejectReqHelp(std::shared_ptr<Instruct> spInfo);

	protected:
		ResponseUI handle_;
		UIBlockThread block_;
		IMPLEMENT_REFCOUNTING(UIThreadCombin);
	};


	class RenderThreadCombin : public CombinThreadComit
	{
	public:
		RenderThreadCombin();
		virtual ~RenderThreadCombin();
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
	protected:
		virtual void procRecvRequest(const std::shared_ptr<Instruct>) override;
		virtual void RejectReq(std::shared_ptr<Instruct> spInfo) override;
		void RejectReqHelp(std::shared_ptr<Instruct> spInfo);

		std::shared_ptr<IPCUnit> ipc_;

	protected:
		ResponseRender handle_;
		RenderBlockThread block_;
		IMPLEMENT_REFCOUNTING(RenderThreadCombin);
	};

}
#endif