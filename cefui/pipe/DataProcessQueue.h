/*
brief.
��Ϣ�̴߳���ģ��
*/
#ifndef _DATAPROCESSQUEUE_H_
#define _DATAPROCESSQUEUE_H_
#pragma once

#include <Windows.h>
#include <process.h>
#include <list>
#include <map>
#include <memory>
#include <assert.h>
#include <mutex>
//#include <syncobject.h>

//#define WAIT_SING_OBJ(handle,time) (WaitForSingleObject( handle , time ) == WAIT_OBJECT_0)

#define MSG_WAIT_SING_OBJ(handle,time) SyncTools::WaitWithMessageLoop(\
	handle , time )

#define WAIT_SING_OBJ(handle,time) (WaitForSingleObject( handle , time ) == WAIT_OBJECT_0)

class SyncTools{
public:
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


	/********************************************************************/
	/*																	*/
	/* Function name : WaitWithMessageLoop								*/
	/* Description   : Pump messages while waiting for event			*/
	/*																	*/
	/********************************************************************/
	static BOOL WaitWithMessageLoop(HANDLE hEvent, DWORD dwMilliseconds)
	{
		DWORD dwRet;

		DWORD dwMaxTick = dwMilliseconds == INFINITE ? INFINITE : GetTickCount() + dwMilliseconds;

		while (1)
		{
			DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //���㻹Ҫ�ȴ�����΢��
			// wait for event or message, if it's a message, process it and return to waiting state
			dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeOut/*dwMaxTick - GetTickCount()*/, QS_ALLINPUT);
			if (dwRet == WAIT_OBJECT_0)
			{
				//OutputDebugStringA("WaitWithMessageLoop() event triggered.\n");
				return TRUE;
			}
			else if (dwRet == WAIT_OBJECT_0 + 1)
			{
				// process window messages
				//OutputDebugStringA("DoEvents.\n");
				DoEvents();
			}
			else if (dwRet == WAIT_TIMEOUT)
			{
				// timed out !
				//OutputDebugStringA("timed out!\n");
				return FALSE;
			}
			else if (dwRet == WAIT_FAILED)
			{
				//OutputDebugStringA("wait failed!\n");
				return FALSE;
			}
			else{
				return FALSE;
			}

		}
	}

	static DWORD WaitWithMessageLoop(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds)
	{
		DWORD dwRet = WAIT_FAILED;

		DWORD dwMaxTick = dwMilliseconds == INFINITE ? INFINITE : GetTickCount() + dwMilliseconds;

		while (1)
		{
			DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //���㻹Ҫ�ȴ�����΢��
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

	static DWORD Wait2Event(HANDLE& hEnv1, HANDLE& hEnv2, DWORD dwMilliseconds)
	{
		HANDLE hEvents[2];
		hEvents[0] = hEnv1;
		hEvents[2] = hEnv1;
		DWORD dw = WaitForMultipleObjects(2, hEvents, FALSE, dwMilliseconds);
		return dw;
	}
};


//�����û�ʹ��
#define CRITICAL_LEVEL    4

//�û�ʹ�ü���
#define ULTRA_LEVEL    7
#define HIGHE_LEVEL     8
#define NORMAL_LEVEL   9
#define LOW_LEVEL      10
#define IDLE_LEVEL     11

/*
�����Զ��еķ�ʽ��������
*/
#define TIME_OUT 15000

template<typename MessageType>
class CDataProcessQueue
{
private:	
	//��Ϣ����
	typedef enum
	{
		QUEUE_MSG_BEGIN,
		QUEUE_MSG_NORMAL,
		QUEUE_MSG_STOP,
		QUEUE_MSG_LAST,
	} QUEUE_MSG;

	//-------------------------cr comment
	// ������ǰд��QUEUEMSG_BEGIN��ǰ�������ʲ��Ͽ���������⣬���ע�����

	struct _DATAPACK 
	{
		QUEUE_MSG m_MessageType;
		std::shared_ptr<MessageType> m_spCoreData; //�û�����
		HANDLE m_hSyncEvent; //ͬ��֪ͨ�¼�
		BOOL m_bSync; //�Ƿ���Ҫͬ��
		BOOL *m_pbResult; //����Ľ��,�첽���͵�����±�����Ƿ�����Ͷ�ݵ��������.ͬ������·��ش���Ľ��
						 //���ش���Ľ���������ദ����̷��ص�ֵ������virtual BOOL ProcDataPack(MessageType*)�ķ���ֵ

		_DATAPACK(std::shared_ptr<MessageType> spCoreData, BOOL *pbRet, BOOL bSync = FALSE, QUEUE_MSG MsgType = QUEUE_MSG_NORMAL)
		{
			m_spCoreData = spCoreData;
			m_hSyncEvent = NULL;
			m_bSync = bSync;
			m_MessageType = MsgType;
			m_pbResult = pbRet;
			*m_pbResult = FALSE;

			if ( bSync )
				m_hSyncEvent = CreateEvent( NULL , FALSE , FALSE , NULL );
			else
				m_hSyncEvent = NULL;
		}
		~_DATAPACK()
		{
			if ( m_hSyncEvent )
			{
				//CloseHandle(m_hSyncEvent);
				//m_hSyncEvent = NULL;
			}
		}
	};
public:
	CDataProcessQueue()
	{
		m_ulSerialNumber = 0;
		m_bCancelIO = FALSE;
		m_hEvents[0] = CreateEvent( NULL , FALSE , FALSE , NULL );
		m_hLoopThreadFinishEvent = CreateEvent( NULL , FALSE , FALSE , NULL );
		m_hNotifyNoBlockWrite = CreateEvent(NULL, FALSE, FALSE, NULL);

		//-------------------------cr comment
		// д����Ҫע���ñ�����һ�ۿ��������д������ĸ�����˶���ʱ����ٷѺܶ�ʱ�䡣FinEvent�����Ǻ������ģ�Ҫ����д����
		// ���������h_Events������ô֪�������Events����ʲô�ģ�һ�ۿ���������Ҫ���Ѻܶ�ʱ��ȥ�������ģ���ȥ�²�

		unsigned int nTid = 0;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, LoopThread , this ,0 , &nTid);
		CloseHandle( hThread );
	}

	virtual ~CDataProcessQueue()
	{
		//Cancel(); //�����ﲻҪcancel
		CloseHandle( m_hEvents[0] );
		CloseHandle(m_hLoopThreadFinishEvent);
	}

	virtual void Cancel()
	{
		if ( !m_bCancelIO )
		{
			SetEvent(m_hNotifyNoBlockWrite);
			std::shared_ptr<MessageType> spMsg;
			WriteMsg( spMsg , 0 , QUEUE_MSG_STOP , CRITICAL_LEVEL );
			//fix �˴�ʹ��SyncTools::WaitWithMessageLoop��������������ui�̵߳Ķ���ʱ���������޷��˳�
			WaitForSingleObject( m_hLoopThreadFinishEvent , TIME_OUT );
			//SyncTools::WaitWithMessageLoop(m_hLoopThreadFinishEvent, TIME_OUT);
		}
		
	}

protected:
	/*
	pMessage ��Ϣ��ָ��
	bSync �Ƿ�ͬ��
	����ֵ ���첽״̬,���������Ƿ��ύ�ɹ���ͬ��״̬����ProcDataPack�Ĵ�����
	*/
	BOOL Submit(std::shared_ptr<MessageType> spMessage, DWORD level = NORMAL_LEVEL, unsigned int nTimeout = 0)
	{	
		return WriteMsg( spMessage , nTimeout , QUEUE_MSG_NORMAL , level );
	}

	void RemoveAll()
	{
		std::unique_lock<std::mutex> lock_scope(m_MessageQueueMutex);
		m_bCancelIO = TRUE;

		for (auto it = m_MessageQueue.begin() ; it != m_MessageQueue.end() ; )
		{
			if ( it->second.get()->m_bSync && it->second.get()->m_hSyncEvent )
			{
				SetEvent( it->second.get()->m_hSyncEvent );
			}
			it = m_MessageQueue.erase( it );
		}
	}

	std::shared_ptr<_DATAPACK> RemoveHead()
	{
		std::unique_lock<std::mutex> lock_scope(m_MessageQueueMutex);
		std::shared_ptr<_DATAPACK> spDataPack;
		auto it = m_MessageQueue.begin();
		if ( it != m_MessageQueue.end() )
		{
			spDataPack = it->second;
			m_MessageQueue.erase(it);
		}
		return spDataPack;
	}

	BOOL WriteMsg(std::shared_ptr<MessageType> spMessage, unsigned int nTimeout, QUEUE_MSG msgType, DWORD dwLevel = NORMAL_LEVEL)
	{
		BOOL bRet = FALSE;
		BOOL bSync = (nTimeout == 0) ? FALSE : TRUE;
		m_MessageQueueMutex.lock(); //fix. �ֶν��l�����ͬ�����ͣ���������������
		if ( m_bCancelIO )
		{
			m_MessageQueueMutex.unlock();
			return FALSE;
		}

		//��ʼ��ֵ
		HANDLE hSyncEvent = NULL;
		std::shared_ptr<_DATAPACK> spDataPack(new _DATAPACK(spMessage, &bRet, bSync, msgType));
		if ( bSync && spDataPack.get()->m_hSyncEvent )
		{
			DuplicateHandle( GetCurrentProcess() , spDataPack.get()->m_hSyncEvent ,
				GetCurrentProcess() , &hSyncEvent ,
				0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
		}

		//���������
		if ( m_MessageQueue.insert(
			std::make_pair(GenerateSerialNumber(dwLevel),spDataPack)).second == false )
		{
			m_MessageQueueMutex.unlock();
			assert(FALSE);
			return FALSE;
		}		
		m_MessageQueueMutex.unlock();
		SetEvent( m_hEvents[0] ); //֪ͨ���µ�����Ҫ����		

		if ( bSync && hSyncEvent )
		{
			//SyncTools::WaitWithMessageLoop( hSyncEvent , TIME_OUT );
			//fix.�ȴ�apiѡ��?
			//WAIT_SING_OBJ(hSyncEvent , nTimeout);
			DWORD dwRet = SyncTools::Wait2Event(hSyncEvent, m_hNotifyNoBlockWrite, nTimeout);
			if ( dwRet == WAIT_OBJECT_0 )
			{
				CloseHandle(hSyncEvent);
			}			
		}
		else
		{
			bRet = TRUE; //�첽״̬�·��سɹ�Ͷ�ݵ������߳�
		}		
		return bRet;
	}

	void ProcData( _DATAPACK* pData )
	{
		BOOL bRet = ProcDataPack( pData->m_spCoreData ); //�������ദ������
		if ( pData->m_bSync && pData->m_hSyncEvent )
		{
			*(pData->m_pbResult) = bRet;
			SetEvent( pData->m_hSyncEvent );
		}
	}

	/*������ˮ�� (��Ϣ����)4bit + (��ˮ��)28bit
	��Ϣ������ֵС,����ִ��
	*/
	unsigned long GenerateSerialNumber(unsigned long dwMessageLevel)
	{
		assert( dwMessageLevel < 16  );
		std::unique_lock<std::mutex> lock_scope(m_SeriaNumberMutex);
		if ( (m_ulSerialNumber^0xfffffff) == 0 )
		{
			//��ˮ�Ź�28λ,�����ӵ�0xfffffffʱ����ˮ��Ҫ���¿�ʼ��������ֹ���
			m_ulSerialNumber = 0;
		}
		InterlockedIncrement( &m_ulSerialNumber );
		return (dwMessageLevel<<28) | m_ulSerialNumber;
	}

	static unsigned int __stdcall LoopThread(void*);
	static void __stdcall APCProc( DWORD dwParam){}

protected:
	virtual BOOL ProcDataPack(std::shared_ptr<MessageType>) = 0;
	HANDLE m_hEvents[1];
	HANDLE m_hLoopThreadFinishEvent;
	HANDLE m_hNotifyNoBlockWrite;
	std::map<unsigned long, std::shared_ptr<_DATAPACK>> m_MessageQueue;
	std::mutex m_MessageQueueMutex, m_SeriaNumberMutex;
	BOOL m_bCancelIO;
	volatile unsigned long m_ulSerialNumber;
};

template<typename MessageType>
unsigned int CDataProcessQueue<MessageType>::LoopThread( void* pContact )
{
	CDataProcessQueue *pQueueClass = reinterpret_cast<CDataProcessQueue*>(pContact);
	while(TRUE)
	{
		DWORD dwRet = WaitForMultipleObjectsEx( sizeof(pQueueClass->m_hEvents)/sizeof(pQueueClass->m_hEvents[0]) ,
			pQueueClass->m_hEvents , FALSE , INFINITE , TRUE );
		if ( 0 == dwRet - WAIT_OBJECT_0)
		{
			//���µ����ݰ�Ҫ����			
			while( TRUE )
			{
				std::shared_ptr<_DATAPACK> spDataPack;
				spDataPack = pQueueClass->RemoveHead();
				if ( !spDataPack.get() )
				{
					break;
				}
				if ( spDataPack.get()->m_MessageType == QUEUE_MSG_STOP )
				{
					pQueueClass->RemoveAll();
				}
				else if ( spDataPack.get()->m_MessageType == QUEUE_MSG_NORMAL )
				{
					pQueueClass->ProcData( spDataPack.get() );
				}
				
			}
			if ( pQueueClass->m_bCancelIO )
			{
				assert( pQueueClass->m_MessageQueue.size() == 0 );
				break;
			}
		}
		else if ( WAIT_IO_COMPLETION == dwRet )
		{
			//apc call
			break;
		}
	}
	SetEvent(pQueueClass->m_hLoopThreadFinishEvent);
	return 0;
}


#endif