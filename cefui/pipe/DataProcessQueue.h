/*
brief.
消息线程处理模板
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
			DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
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
			DWORD dwTimeOut = (dwMaxTick < GetTickCount()) ? 0 : dwMaxTick - GetTickCount(); //记算还要等待多秒微秒
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


//不供用户使用
#define CRITICAL_LEVEL    4

//用户使用级别
#define ULTRA_LEVEL    7
#define HIGHE_LEVEL     8
#define NORMAL_LEVEL   9
#define LOW_LEVEL      10
#define IDLE_LEVEL     11

/*
用来以队列的方式处理数据
*/
#define TIME_OUT 15000

template<typename MessageType>
class CDataProcessQueue
{
private:	
	//消息类型
	typedef enum
	{
		QUEUE_MSG_BEGIN,
		QUEUE_MSG_NORMAL,
		QUEUE_MSG_STOP,
		QUEUE_MSG_LAST,
	} QUEUE_MSG;

	//-------------------------cr comment
	// 这里以前写的QUEUEMSG_BEGIN，前两个单词不断开，容易误解，今后注意避免

	struct _DATAPACK 
	{
		QUEUE_MSG m_MessageType;
		std::shared_ptr<MessageType> m_spCoreData; //用户数据
		HANDLE m_hSyncEvent; //同步通知事件
		BOOL m_bSync; //是否需要同步
		BOOL *m_pbResult; //处理的结果,异步发送的情况下保存的是否正常投递到处理对列.同步情况下返回处理的结果
						 //返回处理的结果，是子类处理过程返回的值。既是virtual BOOL ProcDataPack(MessageType*)的返回值

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
		// 写代码要注意让别人能一眼看懂，你多写几个字母，别人读的时候就少费很多时间。FinEvent别人是很难理解的，要尽量写完整
		// 包括这里的h_Events，我怎么知道这里的Events是做什么的，一眼看不出来，要花费很多时间去看上下文，再去猜测

		unsigned int nTid = 0;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, LoopThread , this ,0 , &nTid);
		CloseHandle( hThread );
	}

	virtual ~CDataProcessQueue()
	{
		//Cancel(); //在这里不要cancel
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
			//fix 此处使用SyncTools::WaitWithMessageLoop，当对像依附在ui线程的对像时，会引起无法退出
			WaitForSingleObject( m_hLoopThreadFinishEvent , TIME_OUT );
			//SyncTools::WaitWithMessageLoop(m_hLoopThreadFinishEvent, TIME_OUT);
		}
		
	}

protected:
	/*
	pMessage 消息的指针
	bSync 是否同步
	返回值 ：异步状态,返回数据是否提交成功。同步状态返回ProcDataPack的处理结果
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
		m_MessageQueueMutex.lock(); //fix. 分段解鑜，解决同步发送，产生死锁的问题
		if ( m_bCancelIO )
		{
			m_MessageQueueMutex.unlock();
			return FALSE;
		}

		//初始化值
		HANDLE hSyncEvent = NULL;
		std::shared_ptr<_DATAPACK> spDataPack(new _DATAPACK(spMessage, &bRet, bSync, msgType));
		if ( bSync && spDataPack.get()->m_hSyncEvent )
		{
			DuplicateHandle( GetCurrentProcess() , spDataPack.get()->m_hSyncEvent ,
				GetCurrentProcess() , &hSyncEvent ,
				0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
		}

		//增加入对列
		if ( m_MessageQueue.insert(
			std::make_pair(GenerateSerialNumber(dwLevel),spDataPack)).second == false )
		{
			m_MessageQueueMutex.unlock();
			assert(FALSE);
			return FALSE;
		}		
		m_MessageQueueMutex.unlock();
		SetEvent( m_hEvents[0] ); //通知有新的数据要处理		

		if ( bSync && hSyncEvent )
		{
			//SyncTools::WaitWithMessageLoop( hSyncEvent , TIME_OUT );
			//fix.等待api选择?
			//WAIT_SING_OBJ(hSyncEvent , nTimeout);
			DWORD dwRet = SyncTools::Wait2Event(hSyncEvent, m_hNotifyNoBlockWrite, nTimeout);
			if ( dwRet == WAIT_OBJECT_0 )
			{
				CloseHandle(hSyncEvent);
			}			
		}
		else
		{
			bRet = TRUE; //异步状态下返回成功投递到处理线程
		}		
		return bRet;
	}

	void ProcData( _DATAPACK* pData )
	{
		BOOL bRet = ProcDataPack( pData->m_spCoreData ); //交给子类处理数据
		if ( pData->m_bSync && pData->m_hSyncEvent )
		{
			*(pData->m_pbResult) = bRet;
			SetEvent( pData->m_hSyncEvent );
		}
	}

	/*生成流水号 (消息级别)4bit + (流水号)28bit
	消息级别数值小,优先执行
	*/
	unsigned long GenerateSerialNumber(unsigned long dwMessageLevel)
	{
		assert( dwMessageLevel < 16  );
		std::unique_lock<std::mutex> lock_scope(m_SeriaNumberMutex);
		if ( (m_ulSerialNumber^0xfffffff) == 0 )
		{
			//流水号共28位,当增加到0xfffffff时，流水号要重新开始增长，防止溢出
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
			//有新的数据包要处理			
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