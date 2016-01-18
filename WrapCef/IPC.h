#ifndef _IPC_H
#define _IPC_H
#pragma once
#include <windows.h>
#include "DataProcessQueue.h"
#include "ipcio.h"
#include <functional>
#include "cjpickle.h"

#define BUF_SIZE 8192

namespace cyjh{

	class IPC;
	typedef std::function< void(const byte*, DWORD)> recv_cb;
	enum _state{
		CONNECTING_STATE,
		READING_STATE,
	};

	struct _SENDPACKET
	{
		_IPC_MESSAGE* m_IPC_Data;
		_SENDPACKET()
		{
			m_IPC_Data = NULL;
		}
		~_SENDPACKET()
		{
			if (m_IPC_Data)
			{
				delete m_IPC_Data;
			}
		}
	};

	//缓存数据,存放需要封包的数据
	struct _CACHEDATA
	{
		unsigned int m_nLen; //数据长度
		unsigned int m_nCurr; //当前存放多少数据
		byte *m_pData; //缓存空间
#ifdef _DEBUG
		unsigned int m_writeTimes;
		unsigned int m_test;
#endif
		

		_CACHEDATA(unsigned int nLen)
		{
#ifdef _DEBUG
			m_writeTimes = 0;
			m_test = 0;
#endif			
			m_nCurr = 0;
			m_nLen = nLen;
			m_pData = new byte[nLen];
		}

		~_CACHEDATA()
		{
			delete[]m_pData;
		}

		const byte * GetData()
		{
			return m_pData;
		}

		bool WriteData(byte* pSrc, unsigned int nlen, unsigned int nOffset)
		{
#ifdef _DEBUG
			++m_writeTimes;
#endif
			//errno_t err = memcpy_s(m_pData + m_nCurr, m_nLen - m_nCurr, pSrc, nlen);
			errno_t err = memcpy_s(m_pData + nOffset, nlen, pSrc, nlen);
			if (err == 0)
			{
				m_nCurr += nlen;
			}
			return (err == 0);
		}

		const bool complate(){
			return m_nCurr == m_nLen;
		}
	};

	typedef std::map<int, std::shared_ptr<_CACHEDATA> > CACHEDATA_MAP; //组装数据包

	class  _IPC_MESSAGE_ITEM
	{
	public:
		_IPC_MESSAGE_ITEM(IPC* inst) :ipc_(inst), total_msg_size_(sizeof(_IPC_MESSAGE))
		{
			memset(&msg_, 0, sizeof(msg_));
			curr_ = reinterpret_cast<byte*>(&msg_);
			//total_msg_size_ = sizeof(_IPC_MESSAGE);
			write_size_ = 0;
		}
		virtual ~_IPC_MESSAGE_ITEM(){}

		void Append(const byte* data, const DWORD len);

		const _IPC_MESSAGE* GetMsg(){
			return &msg_;
		}

	private:
		_IPC_MESSAGE msg_;
		byte *curr_;
		IPC* ipc_;
		DWORD write_size_;
		const DWORD total_msg_size_;

	};

	class IPC : public CDataProcessQueue<_SENDPACKET>
	{		
		friend _IPC_MESSAGE_ITEM;
	public:
		IPC(IPC* inst);
		virtual ~IPC();
		/*const bool isValidHandle(const HANDLE& handle)
		{
			return handle != NULL && handle != INVALID_HANDLE_VALUE;
		}*/
		virtual bool Send(const byte* data, DWORD len, DWORD nTimeout);
	protected:
		
		bool IPC::proxy_send(const byte* data, DWORD len, DWORD nTimeout);
		virtual BOOL ProcDataPack(std::shared_ptr<_SENDPACKET>) override;
		BOOL SubmitPack(_IPC_MESSAGE * data, unsigned int nTimeout);

		virtual void RecvData(_IPC_MESSAGE*);

		virtual void RecvData(byte* recv_buf_, DWORD len);

		//还回true,表示收到完整_IPC_MESSAGE数据包,
		void CombinPack(byte* recv_buf_, DWORD len);

		virtual void NotifyRecvData(byte*, DWORD len) = 0;

 		const int& generateID();

	protected:
		OVERLAPPED op_;
		HANDLE srvpipe_, remotepipe_;
		HANDLE hEvents_[2];
		byte recv_buf_[BUF_SIZE];
		_IPC_MESSAGE_ITEM msg_item_;
		CACHEDATA_MAP m_cacheMap;
		int seriaNum_; //发送id
		std::mutex seriaNumberMutex_;
	};


	class IPCPipeSrv : public IPC
	{
	public:
		IPCPipeSrv(const WCHAR* name);
		virtual ~IPCPipeSrv();
		//virtual bool Send(char* data, int len) override;		
	protected:
		//virtual BOOL ProcDataPack(std::shared_ptr<_SENDPACKET>) override;

		static unsigned int __stdcall WorkThread(void*);

		virtual void RecvData(_IPC_MESSAGE*) override;

		virtual void RecvData(byte* recv_buf_, DWORD len) override;

		virtual void NotifyRecvData(byte*, DWORD len) override;
	private:		
		bool succCreate_;
		bool bPendingIO_;
	};

	class IPCPipeClient : public IPC
	{
	public:
		IPCPipeClient(const WCHAR* name);
		virtual ~IPCPipeClient();

		void SetRecvCallback(recv_cb cb){
			cb_ = cb;
		}

		//virtual bool Send(char* data, int len) override;
	protected:
		//virtual BOOL ProcDataPack(std::shared_ptr<_SENDPACKET>) override;

		static unsigned int __stdcall WorkThread(void*);

		virtual void RecvData(_IPC_MESSAGE*) override;

		virtual void RecvData(byte* recv_buf_, DWORD len) override;

		virtual void NotifyRecvData(byte*, DWORD len) override;

		
	private:
//		HANDLE pipe_;
		WCHAR name_[64];
		recv_cb cb_;
	};

	class IPCUnit{
	public:
		IPCUnit(const WCHAR* srv_ame, const WCHAR* cli_name);
		~IPCUnit();

		bool Send(const byte* data, DWORD len, DWORD nTimeout);

		template<typename T>
		void BindRecvCallback(void (T::*function)(const byte*, DWORD), T* obj){
			recv_cb cb = std::bind(function, obj, std::placeholders::_1, std::placeholders::_2);
			cli_.SetRecvCallback(cb);
		}

	private:
		IPCPipeSrv srv_;
		IPCPipeClient cli_;
	};

}
#endif