/*
brief.
�����̽���״̬
*/
#ifndef DETECTPROCESS_H
#define DETECTPROCESS_H
#pragma once

#include <Windows.h>
#include <set>
#include <mutex>
#include <functional>
#include <assert.h>

typedef std::function<void(DWORD,DWORD)> ALERT_CB;

class CDetectProcess
{
public:
	CDetectProcess();
	virtual ~CDetectProcess();

	//����Ҫ���Ľ���id
	bool Detect(DWORD dwProcessID);

	template<typename T>
	void RegisterAlertFun( void (T::*alertFunction)(DWORD,DWORD) , T* obj){
		assert( m_alertCallback == NULL );
		m_alertCallback = std::bind(alertFunction, obj, std::placeholders::_1, std::placeholders::_2);
	}

	void RegisterAlertFun(ALERT_CB _CallBack){
		assert( m_alertCallback == NULL );
		m_alertCallback = _CallBack;
	}

	//��ȡ�ж��ٸ����̱�����
	unsigned int GetProcessNumber();

protected:
	static unsigned int _stdcall WaitExitProcess( void* parm );
	void RemoveThread(HANDLE);
	void NotifyAlert( DWORD , DWORD );
	void Quit();

private:
	ALERT_CB m_alertCallback;
	std::set<HANDLE> m_Threads;
	std::mutex m_thread_lock;
};


#endif