#include "stdafx.h"
#include "detectProcess.h"
#include <process.h>

//#define _WIN32_WINNT _WIN32_WINNT_WINXP

struct DetectThreadParm 
{
	DWORD dwProcessID;
	HANDLE hProcess;
	HANDLE hThread;
	CDetectProcess* self;
};

void _stdcall APCProc( ULONG_PTR dwParam )
{
}

CDetectProcess::CDetectProcess()
{
	//m_alertCallback = NULL;
}

CDetectProcess::~CDetectProcess()
{
	Quit();
}

unsigned int _stdcall CDetectProcess::WaitExitProcess( void* vparm )
{
	DetectThreadParm* parm = (DetectThreadParm*)vparm;
	DWORD dwRet = WaitForSingleObjectEx( parm->hProcess , INFINITE , TRUE );
	if ( WAIT_OBJECT_0 == dwRet )
	{
#ifdef _DEBUG1
		OutputDebugStringW(L"----------------main process exit!!!!!!!!!!!");
#endif
		DWORD dwCode = 0;
		GetExitCodeProcess( parm->hProcess , &dwCode );
		parm->self->NotifyAlert( parm->dwProcessID , dwCode );
		parm->self->RemoveThread( parm->hThread );
	}else if ( WAIT_IO_COMPLETION == dwRet )
	{
	}
	
	delete parm;
	return 0;
}

bool CDetectProcess::Detect(DWORD dwProcessID)
{
	bool bRet = false;
	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS , FALSE , dwProcessID );
	if ( hProcess )
	{
#ifdef _DEBUG1
		OutputDebugStringA( "CDetectProcess::Detect--------------\n" );
#endif
		bRet = true;
		DetectThreadParm* parm = new DetectThreadParm;
		parm->dwProcessID = dwProcessID;
		parm->hProcess = hProcess;
		parm->self = this;
		parm->hThread = (HANDLE)_beginthreadex( NULL , 0 , WaitExitProcess , parm , 0 , NULL );
		m_thread_lock.lock();
		m_Threads.insert( parm->hThread );
		m_thread_lock.unlock();
	}else
	{
#ifdef _DEBUG1
		DWORD dwCode = GetLastError();
		char szMsg[128]={0};
		sprintf_s( szMsg , "CDetectProcess::Detect--------fail---- %d --\n" , dwCode );
		OutputDebugStringA( szMsg);
#endif
	}
	
	return bRet;
}

void CDetectProcess::NotifyAlert( DWORD dwProcessID , DWORD dwExitCode )
{
	if ( m_alertCallback )
	{
		m_alertCallback( dwProcessID , dwExitCode );
	}
}

void CDetectProcess::RemoveThread(HANDLE hThread)
{
	if( m_thread_lock.try_lock() )
	{
		m_Threads.erase(hThread);
		CloseHandle(hThread);
		m_thread_lock.unlock();
	}

}

void CDetectProcess::Quit()
{
	std::unique_lock<std::mutex> lock(m_thread_lock);
	std::set<HANDLE>::iterator it = m_Threads.begin();
	for ( ; it != m_Threads.end() ; )
	{
		HANDLE hThread = *it;
		QueueUserAPC( APCProc , hThread , NULL );
		WaitForSingleObject( hThread , INFINITE );		
		CloseHandle(hThread);
		it = m_Threads.erase(it);
	}
}

unsigned int CDetectProcess::GetProcessNumber()
{
	std::unique_lock<std::mutex> lock(m_thread_lock);
	return m_Threads.size();
}