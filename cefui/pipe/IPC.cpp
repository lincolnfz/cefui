#include "stdafx.h"
#include "IPC.h"
#include <algorithm>

#define  CHECK_HANDLE(handle_) handle_ != NULL && handle_ != INVALID_HANDLE_VALUE

namespace cyjh{

	class SignEventHelper
	{
	public:
		SignEventHelper(HANDLE hEvent){
			//m_hEvent = hEvent;
			DuplicateHandle(GetCurrentProcess(), hEvent,
				GetCurrentProcess(), &m_hEvent,
				0, FALSE, DUPLICATE_SAME_ACCESS);
		}

		~SignEventHelper(){
			SetEvent(m_hEvent);
			CloseHandle(m_hEvent);
		}

	private:
		HANDLE m_hEvent;
	};


	static volatile int  ipc_unit_id = 0;
	void _IPC_MESSAGE_ITEM::Append(const unsigned char* data, const DWORD len)
	{
		DWORD outRemind = len;
		while ( outRemind )
		{
			DWORD nCopySize = min(total_msg_size_ - write_size_, len);
			memcpy_s(curr_, nCopySize, data, nCopySize);
			curr_ += nCopySize;
			write_size_ += nCopySize;
			outRemind -= nCopySize;
			if ( write_size_ == total_msg_size_ )
			{
				ipc_->RecvData(&msg_);
				write_size_ = 0;
				curr_ = reinterpret_cast<unsigned char*>(&msg_);
			}
		}
		
	}

	IPC::IPC()
	{
		srvpipe_ = remotepipe_ = NULL;
		memset(&op_, 0, sizeof(op_));
		hFin_ = CreateEvent(NULL, TRUE, TRUE, NULL);
		hEvents_[0] = CreateEvent(NULL, TRUE, TRUE, NULL);   // OVERLPPED‘s event
		hEvents_[1] = CreateEvent(NULL, TRUE, FALSE, NULL);  // exit event
		op_.hEvent = hEvents_[0];
		seriaNum_ = 0;
		msg_item_ = new _IPC_MESSAGE_ITEM(this);
	}


	IPC::~IPC()
	{
		if (CHECK_HANDLE(srvpipe_))
		{
			CloseHandle(srvpipe_);
		}

		if (CHECK_HANDLE(remotepipe_))
		{
			CloseHandle(remotepipe_);
		}

		if (CHECK_HANDLE(hEvents_[0]))
		{
			CloseHandle(hEvents_[0]);
		}

		if (CHECK_HANDLE(hEvents_[1]))
		{
			CloseHandle(hEvents_[1]);
		}
		delete msg_item_;
		//WaitForSingleObject(hFin_, INFINITE);
		CloseHandle(hFin_);
	}

	bool IPC::Send(const unsigned char* data, DWORD len, DWORD nTimeout)
	{		
		if (len <= 0)
		{
			return false;
		}
		return proxy_send(data, len, nTimeout);
	}

	const int& IPC::generateID()
	{
		std::unique_lock<std::mutex> lock(seriaNumberMutex_);
		if (seriaNum_ == 0x7fffffff){
			seriaNum_ = 0;
		}
		return ++seriaNum_;
	}

	bool IPC::proxy_send(const unsigned char* data, DWORD len, DWORD nTimeout)
	{
		//把data的数据发送到对方.如果数据的总长度大与发送缓存,会进行分包发送
		bool bRet = true;
		//if (unit && unit->m_state == IPC_STATE_ESTABLISHED)
		{
			//int iID = unif_int(1, 0xfffffff);
			int iID = generateID();
			int remind = len;
			unsigned char *curr = const_cast<unsigned char*>(data);
			unsigned int nTotalPack = (len + MAX_IPC_BUF - 1) / MAX_IPC_BUF; //计算共要多少次分包发送
			unsigned int nRemindPack = nTotalPack;
			unsigned int nOrder = 0;
			unsigned int nOffset = 0;
			while (remind > 0)
			{
				_IPC_MESSAGE * msg = new _IPC_MESSAGE(
					iID, //
					len, //数据总长度
					nOrder,
					nTotalPack//--nRemindPack
					, nOffset
					);
				--nRemindPack;
				int nsend = min(remind, MAX_IPC_BUF);
				memcpy_s(msg->m_buf, MAX_IPC_BUF, curr, nsend);
				msg->m_nEffectLen = nsend; //当前包数据长度

				//数据包准备完毕,请求发送
				if (!SubmitPack(msg, nTimeout))
				{
					bRet = false;
					break;
				}
				curr += nsend;
				nOffset += nsend;
				remind -= nsend;
				++nOrder;
			}
		}
		//else{
		//	bRet = false;
		//}
		return bRet;
	}

	BOOL IPC::SubmitPack( _IPC_MESSAGE * data, unsigned int nTimeout)
	{
		/*_SENDPACKET *pack = new _SENDPACKET;
		pack->m_IPC_Data = data;
		std::shared_ptr<_SENDPACKET> spPack(pack);*/
		std::shared_ptr<_SENDPACKET> spPack(new _SENDPACKET);
		spPack->m_IPC_Data = data;
		return Submit(spPack, NORMAL_LEVEL, nTimeout);
	}

	BOOL IPC::ProcDataPack(std::shared_ptr<_SENDPACKET> data)
	{
		BOOL bResult = FALSE;
		DWORD dwCurr = 0;
		DWORD dwRemin = sizeof(_IPC_MESSAGE);
		unsigned char* byte_begin = reinterpret_cast<unsigned char*>(data->m_IPC_Data);
		while (true)
		{
			if (dwRemin == 0)
			{
				bResult = TRUE;
				break;
			}
			DWORD dwTrans = 0;
			BOOL bSucc = WriteFile(srvpipe_, byte_begin + dwCurr, dwRemin, &dwTrans, NULL);
			if (bSucc)
			{
				dwCurr += dwTrans;
				dwRemin -= dwTrans;
				//OutputDebugString(L"IPC::ProcDataPack WriteFile true");
			}
			else{
				DWORD err = GetLastError();
				if (err == ERROR_PIPE_LISTENING)
				{
					SleepEx(20, true);
					continue;
				}
#ifdef _DEBUG
				WCHAR sz[128];
				wsprintfW(sz, L"IPC::ProcDataPack WriteFile fail = %d", err);
				OutputDebugStringW(sz);
#endif
				break;
			}
		}
		assert(bResult);
		return bResult;
	}

	void IPC::CombinPack(unsigned char* recv_buf, DWORD len)
	{
		msg_item_->Append(recv_buf, len);
	}

	/*void IPC::RecvData(_IPC_MESSAGE* pData)
	{
		auto it = m_cacheMap.find(pData->m_iID);
		if (pData->m_nRemindPack > 0)//没有收到完整的包	
		{
			if (it != m_cacheMap.end())
			{
				it->second.get()->WriteData(pData->m_buf, pData->m_nEffectLen, pData->m_nOffset);
#ifdef _DEBUG
				int check = pData->m_nOrder - it->second.get()->m_test;
				//assert(check == 1);
				it->second.get()->m_test = pData->m_nOrder;
#endif
			}
			else
			{
				_CACHEDATA* pCache = new _CACHEDATA(pData->m_nLen);
				pCache->WriteData(pData->m_buf, pData->m_nEffectLen, pData->m_nOffset);
#ifdef _DEBUG
				pCache->m_test = pData->m_nOrder;
#endif
				std::shared_ptr<_CACHEDATA> spChche(pCache);
				bool bInsert = m_cacheMap.insert(std::make_pair(pData->m_iID, spChche)).second;
				assert(bInsert);
			}
		}
		else
		{
			//已收到完整的包
			if (it != m_cacheMap.end())
			{
				it->second.get()->WriteData(pData->m_buf, pData->m_nEffectLen, pData->m_nOffset);

				//使用缓存中的数据
				NotifyRecvData(
					const_cast<unsigned char*>(it->second.get()->GetData()),
					it->second.get()->m_nLen
					);
				m_cacheMap.erase(it);
			}
			else
			{
				NotifyRecvData(pData->m_buf, pData->m_nLen);
			}
		}
	}*/

	void IPC::RecvData(_IPC_MESSAGE* pData)
	{
		if ( pData->m_nTotalPack > 1 )
		{
			auto it = m_cacheMap.find(pData->m_iID);
			if (it != m_cacheMap.end())
			{
				it->second.get()->WriteData(pData->m_buf, pData->m_nEffectLen, pData->m_nOffset);
#ifdef _DEBUG
				int check = pData->m_nOrder - it->second.get()->m_test;
				//assert(check == 1);
				it->second.get()->m_test = pData->m_nOrder;
#endif
				if ( it->second.get()->complate() )
				{
					//使用缓存中的数据
					NotifyRecvData(
						const_cast<unsigned char*>(it->second.get()->GetData()),
						it->second.get()->m_nLen
						);
					m_cacheMap.erase(it);
				}
			}
			else
			{
				_CACHEDATA* pCache = new _CACHEDATA(pData->m_nLen);
				pCache->WriteData(pData->m_buf, pData->m_nEffectLen, pData->m_nOffset);
#ifdef _DEBUG
				pCache->m_test = pData->m_nOrder;
#endif
				std::shared_ptr<_CACHEDATA> spChche(pCache);
				bool bInsert = m_cacheMap.insert(std::make_pair(pData->m_iID, spChche)).second;
				assert(bInsert);
			}
		}
		else
		{
			NotifyRecvData(pData->m_buf, pData->m_nLen);
		}
	}

	void IPC::RecvData(unsigned char* recv_buf, DWORD len)
	{
		CombinPack(recv_buf, len);

	}



	/////////////////////
	//// IPCPipeSrv实现
	//////////////////////////////////////////////////////////////////////////

	IPCPipeSrv::IPCPipeSrv(const WCHAR* name, IPCUnit* unit) : unit_(unit)
	{
		bPendingIO_ = false;
		succCreate_ = false;
		wcscpy_s(name_, name);		

		//assert(succCreate_);
		//if ( succCreate_ )
		{
			unsigned int nTid = 0;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkThread, this, 0, &nTid);
			CloseHandle(hThread);
		}
	}

	IPCPipeSrv::~IPCPipeSrv()
	{
		
	}

	bool IPCPipeSrv::Close(){
		//DisconnectNamedPipe(srvpipe_);
		SetEvent(hEvents_[1]);
		WaitForSingleObject(hFin_, INFINITE);
		return true;
	}

	unsigned int __stdcall IPCPipeSrv::WorkThread(void* parm)
	{
		IPCPipeSrv* inst = reinterpret_cast<IPCPipeSrv*>(parm);
		SignEventHelper evnetHelp(inst->hFin_);
		// Prepare the security attributes  
		// If lpSecurityAttributes of CreateNamedPipe is NULL, the named pipe   
		// gets a default security descriptor and the handle cannot be inherited.   
		// The ACLs in the default security descriptor for a named pipe grant   
		// full control to the LocalSystem account, administrators, and the   
		// creator owner. They also grant read access to members of the Everyone   
		// group and the anonymous account. In other words, with NULL as the   
		// security attributes, the named pipe cannot be connected with the   
		// WRITE permission across the network, or from a local client running   
		// as a lower integiry level. Here, we fill the security attributes to   
		// grant EVERYONE all access (not just the connect access) to the server   
		// This solves the cross-network and cross-IL issues, but it creates   
		// a security hole right there: the clients have WRITE_OWNER access and   
		// then the server just lose the control of the pipe object. 
		SECURITY_ATTRIBUTES sa;
		sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
		// ACL is set as NULL in order to allow all access to the object.  
		SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		inst->srvpipe_ = CreateNamedPipeW(inst->name_, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
			1, BUF_SIZE, BUF_SIZE, 0, &sa);

		if (inst->srvpipe_ != INVALID_HANDLE_VALUE)
		{
			//等待别的管道联进来
			ConnectNamedPipe(inst->srvpipe_, &inst->op_);
			DWORD dwErr = GetLastError();
			switch (dwErr)
			{
			case ERROR_IO_PENDING:
				{
					inst->bPendingIO_ = true;
					inst->succCreate_ = true;
				}
				break;
			case ERROR_PIPE_CONNECTED:
				{
					SetEvent(inst->hEvents_[0]);
					inst->succCreate_ = true;
				}
				break;
			default:
				break;
			}
		}
		if ( !inst->succCreate_ )
		{
			return 0;
		}

		DWORD dwTrans = 0;
		BOOL bSuccess = false;
		_state state = CONNECTING_STATE;
		
		while (true)
		{
			DWORD dwRet = WaitForMultipleObjects(2, inst->hEvents_, FALSE, INFINITE);
			if (0 == dwRet - WAIT_OBJECT_0)
			{
				if ( inst->bPendingIO_ )
				{
					bSuccess = GetOverlappedResult(inst->srvpipe_, &inst->op_, &dwTrans, FALSE);
					switch (state)
					{
					case cyjh::CONNECTING_STATE:
					{
						if ( !bSuccess )
						{
							//OutputDebugString(L"IPCPipeSrv DisconnectNamedPipe");
							DisconnectNamedPipe(inst->srvpipe_);
							return 0;
						}
						inst->unit_->StateChange(IPC_STATE_SEND_RDY);
						state = READING_STATE;
					}
					break;
					case cyjh::READING_STATE:
					{
						if (!bSuccess || dwTrans == 0){
							//OutputDebugString(L"IPCPipeSrv DisconnectNamedPipe");
							DisconnectNamedPipe(inst->srvpipe_);
							return 0;
						}
					}
					break;
					default:
						break;
					}
				}else{
					inst->unit_->StateChange(IPC_STATE_SEND_RDY);
					state = READING_STATE;
				}


				/*bSuccess = ReadFile(inst->pipe_, inst->recv_buf_, sizeof(inst->recv_buf_), &dwTrans, &inst->op_);
				if (bSuccess && dwTrans != 0)
				{
					inst->bPendingIO_ = FALSE;
					//收到数据了
					inst->RecvData(inst->recv_buf_, dwTrans);

					continue;
				}
				DWORD dwErr = GetLastError();
				if (!bSuccess && (dwErr == ERROR_IO_PENDING)) {
					inst->bPendingIO_ = TRUE;
					continue;
				}*/

				break;
			}
			else{
				//结束工作线程
				DisconnectNamedPipe(inst->srvpipe_);
				break;
			}
		}
		//OutputDebugString(L"IPCPipeSrv::WorkThread 结束");
		return 0;
	}

	/*bool IPCPipeSrv::Send(char* data, int len)
	{
		return true;
	}*/

	/*void IPCPipeSrv::RecvData(_IPC_MESSAGE* pack)
	{
		IPC::RecvData(pack);
	}

	void IPCPipeSrv::RecvData(unsigned char* recv_buf, DWORD len)
	{
		IPC::RecvData(recv_buf, len);
		
	}

	void IPCPipeSrv::NotifyRecvData(unsigned char* byte, DWORD len)
	{
		//no_imple
	}*/



//////////////////////////////////////////////////////////////////////////
	//// 实现IPCPipeClient
/////////////

	IPCPipeClient::IPCPipeClient(const WCHAR* name, IPCUnit* unit) : unit_(unit)
	{
		wcscpy_s(name_, name);
		//ResetEvent(hFin_);
		unsigned int nTid = 0;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WorkThread, this, 0, &nTid);
		CloseHandle(hThread);
	}

	IPCPipeClient::~IPCPipeClient()
	{
		//SetEvent(hEvents_[1]);
		WaitForSingleObject(hFin_, INFINITE);
	}

	bool IPCPipeClient::Close(){
		//SetEvent(hEvents_[1]);
		//WaitForSingleObject(hFin_, INFINITE);
		return true;
	}

	unsigned int __stdcall IPCPipeClient::WorkThread(void* parm)
	{
		IPCPipeClient* inst = reinterpret_cast<IPCPipeClient*>(parm);
		SignEventHelper evnetHelp(inst->hFin_);

		//去连接另一个进程，线程创建的命名管道
		while (true)
		{
			if (!WaitNamedPipeW(inst->name_, NMPWAIT_USE_DEFAULT_WAIT))
			{
				DWORD dwErr = GetLastError();
				if (dwErr = ERROR_FILE_NOT_FOUND){
					//OutputDebugString(L"IPCPipeClient::WorkThread sleepex");
					SleepEx(10, TRUE);
					continue;
				}
				else{
					//OutputDebugString(L"IPCPipeClient::WorkThread 失败");
					//SetEvent(inst->hEvents_[1]);
					return 0;
				}
				
			}

			inst->remotepipe_ = CreateFileW(
				inst->name_,   // pipe name 
				GENERIC_READ |  // read and write access 
				GENERIC_WRITE,
				0,              // no sharing 
				NULL,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				0,              // default attributes 
				NULL);          // no template file 

			if (CHECK_HANDLE(inst->remotepipe_))
			{
				break;
			}
			else{
				//SetEvent(inst->hEvents_[1]);
				return 0;
			}


		}
		
		DWORD dwTrans = 0;
		BOOL bSuccess = FALSE;
		BOOL bPendingIO = FALSE;

		DWORD   dwMode = PIPE_READMODE_MESSAGE;
		bSuccess = SetNamedPipeHandleState(
			inst->remotepipe_,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 

		if ( !bSuccess )
		{
			//SetEvent(inst->hEvents_[1]);
			return 0;
		}
		//OutputDebugString(L"IPCPipeClient::ReadFile start");
		inst->unit_->StateChange(IPC_STATE_RECV_RDY);
		while (true)
		{
			bSuccess = ReadFile(inst->remotepipe_, inst->recv_buf_, sizeof(inst->recv_buf_), &dwTrans, NULL);
			if (bSuccess && dwTrans != 0)
			{
				inst->RecvData(inst->recv_buf_, dwTrans);
			}
			else{
				break;
			}
		}
		inst->unit_->StateChange(0);
		if (inst->disconstCB_)
		{
			inst->disconstCB_();
		}
		//SetEvent(inst->hFin_);
		return 0;
	}

	/*BOOL IPCPipeClient::ProcDataPack(std::shared_ptr<_SENDPACKET> data)
	{
		return false;
	}
	
	bool IPCPipeClient::Send(char* data, int len)
	{
		return true;
	}*/



	void IPCPipeClient::RecvData(_IPC_MESSAGE* pack)
	{
		IPC::RecvData(pack);
	}

	void IPCPipeClient::RecvData(unsigned char* recv_buf, DWORD len)
	{
		IPC::RecvData(recv_buf, len);
	}

	void IPCPipeClient::NotifyRecvData(unsigned char* byte, DWORD len)
	{
		if ( cb_ )
		{
			cb_(byte, len);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//// 实现IPCUnit
	/////////////
	IPCUnit::IPCUnit(const WCHAR* srv_ame, const WCHAR* cli_name)
	{
		state_ = 0;
		close_ = false;
		wcscpy_s(m_szSrvName, srv_ame);
		wcscpy_s(m_szCliName, cli_name);
		id_ = InterlockedIncrement((long*)&ipc_unit_id);
		cli_ = NULL;
		srv_ = NULL;
		//srv_ = new IPCPipeSrv(srv_ame, this);
		//cli_ = new IPCPipeClient(cli_name, this);
		//cli_->BindDisconstCallback(&IPCUnit::NotifyDisconst, this);
	}

	IPCUnit::~IPCUnit()
	{
		delete srv_;
		delete cli_;
	}

	void IPCUnit::MakeUnitCreated()
	{
		if ( !srv_ )
		{
			srv_ = new IPCPipeSrv(m_szSrvName, this);
		}
		if ( !cli_ )
		{
			cli_ = new IPCPipeClient(m_szCliName, this);
		}
	}

	void IPCUnit::Launch()
	{
		//id_ = InterlockedIncrement((long*)&ipc_unit_id);
		//srv_ = new IPCPipeSrv(m_szSrvName, this);
		//cli_ = new IPCPipeClient(m_szCliName, this);
		MakeUnitCreated();
		cli_->BindDisconstCallback(&IPCUnit::NotifyDisconst, this);
	}

	void IPCUnit::Close()
	{
		close_ = true;
		srv_->Close();
		cli_->Close();
	}

	bool IPCUnit::Send(const unsigned char* data, DWORD len, DWORD nTimeout)
	{
		if ( close_ || srv_  == NULL )
		{
			return false;
		}
		return srv_->Send(data, len, nTimeout);
	}

	void IPCUnit::NotifyDisconst()
	{
		sIPC_Manager.Destruct(id_);
	}

	void IPCUnit::StateChange(const int state){
		state_mutex_.lock();
		state_ |= state;
		state_mutex_.unlock();
		if ( state_cb_ )
		{
			state_cb_(state_);
		}
	}

	const int IPCUnit::GetState()
	{
		std::unique_lock<std::mutex> lock( state_mutex_ );
		return state_;
	}

	////////////////////////////////////
	/////IPC_Manager
	//////////////////////////////////////////////////////////////////////////
	volatile int IPC_Manager::id_ = 0;

	IPC_Manager::~IPC_Manager()
	{
		std::map<int, std::shared_ptr<IPCUnit>>::iterator it = ipcs_.begin();
		for (; it != ipcs_.begin(); ++it)
		{
			it->second->Close();
		}
	}

	std::shared_ptr<IPCUnit> IPC_Manager::GenerateIPC(const WCHAR* srv, const WCHAR* client)
	{
		std::shared_ptr<IPCUnit> unit(new IPCUnit(srv, client));
		id_ = InterlockedIncrement((long*)&id_);
		ipcs_.insert(std::make_pair(id_, unit));
		//ipcs_.push_back(unit);
		return unit;
	}

	std::shared_ptr<IPCUnit> IPC_Manager::GetIpc(const int& id)
	{
		std::shared_ptr<IPCUnit> spUnit;
		std::map<int, std::shared_ptr<IPCUnit>>::iterator it = ipcs_.find(id);
		if ( it != ipcs_.end() )
		{
			spUnit = it->second;
		}
		return spUnit;
	}

	int IPC_Manager::MatchIpc(const WCHAR* srv, const WCHAR* cli)
	{
		int idx = 0;
		std::map<int, std::shared_ptr<IPCUnit>>::iterator it = ipcs_.begin();
		for ( ; it != ipcs_.end(); ++it)
		{
			if ((wcscmp(it->second->getSrvName(), srv) == 0) && (wcscmp(it->second->getCliName(), cli) == 0))
			{
				idx = it->first;
				break;
			}
		}
		return idx;
	}

	void IPC_Manager::Destruct(int id)
	{
		/*std::vector<std::shared_ptr<IPCUnit>>::iterator it = ipcs_.begin();
		for (; it != ipcs_.end(); ++it)
		{
			if ( it->get()->getID() == id )
			{
				ipcs_.erase(it);
				break;
			}
		}*/
		std::map<int, std::shared_ptr<IPCUnit>>::iterator it = ipcs_.find(id);
		if (it != ipcs_.end())
		{
			it->second->Close();
			ipcs_.erase(it);
		}
	}
}