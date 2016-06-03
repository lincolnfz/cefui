#include "stdafx.h"
#include "sockBase.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
//#ifdef _DEBUG
//#pragma comment(lib, "../3rd-party/libevent/lib/debug/libevent.lib")
//#else
//#pragma comment(lib, "../3rd-party/libevent/lib/release/libevent.lib")
//#endif

namespace cyjh{

bool SockBase::s_bInit = false;
int SockBase::s_numInsts = 0;

std::mutex sock_contruct_mutex;

SockBase::SockBase()
{
	std::unique_lock<std::mutex> lock_scope(sock_contruct_mutex);
	if ( !s_bInit )
	{
		WORD sockVersion = MAKEWORD(2,2);  
		WSADATA data;   
		s_bInit = (WSAStartup(sockVersion, &data) == 0);
		evthread_use_windows_threads();
	}
	++s_numInsts;
	m_base = NULL;
	m_sock = INVALID_SOCKET;
}

SockBase::~SockBase()
{
	std::unique_lock<std::mutex> lock_scope(sock_contruct_mutex);
	--s_numInsts;
	if ( m_sock != INVALID_SOCKET )
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	if ( s_numInsts == 0 && s_bInit )
	{
		WSACleanup();
	}
}

bool SockBase::InitEventBase()
{
	//boost::mutex::scoped_lock lock_scope(m_base_mutex_);
	std::unique_lock<std::mutex> lock_scope(m_base_mutex_);
	m_base = event_base_new();
	if ( !m_base )
	{
		std::wcout << L"Could not initialize libevent!";
		return false;
	}
	return true;
}

}