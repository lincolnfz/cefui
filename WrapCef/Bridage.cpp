#include "stdafx.h"
#include "Bridage.h"

/*Bridage Bridage::s_bridage;

Bridage& Bridage::getInstance() {
	return s_bridage;
}*/

void DoEvents()
{
	MSG msg;

	// window message         
	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	/*while (true)
	{
		Sleep(0);
		CefDoMessageLoopWork();
	}*/	
}

/********************************************************************/
/*																	*/
/* Function name : WaitWithMessageLoop								*/
/* Description   : Pump messages while waiting for event			*/
/*																	*/
/********************************************************************/
bool WaitWithMessageLoop(HANDLE hEvent, DWORD dwMilliseconds)
{
	DWORD dwRet;

	DWORD dwMaxTick = dwMilliseconds == INFINITE ? INFINITE : GetTickCount() + dwMilliseconds;

	while (1)
	{
		DWORD dwNow = GetTickCount();
		DWORD dwTimeOut = (dwMaxTick < dwNow) ? 0 : dwMaxTick - dwNow; //记算还要等待多秒微秒
		// wait for event or message, if it's a message, process it and return to waiting state
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeOut, QS_ALLINPUT);
		if (dwRet == WAIT_OBJECT_0)
		{
			//TRACE0("WaitWithMessageLoop() event triggered.\n");
			return true;
		}
		else if (dwRet == WAIT_OBJECT_0 + 1)
		{
			// process window messages
			//TRACE0("DoEvents.\n");
			DoEvents();
		}
		else if (dwRet == WAIT_TIMEOUT)
		{
			// timed out !
			//TRACE0("timed out!\n");
			return false;
		}
		else if (dwRet == WAIT_FAILED)
		{
			//TRACE0("wait failed!\n");
			return false;
		}
		else{
			return false;
		}

	}
}

Bridage::Bridage()
{
	m_id = 0;
}


Bridage::~Bridage()
{
}

const int Bridage::generateID()
{
	base::AutoLock lock_scope(lock_);
	++m_id;
	return m_id;
}

bool Bridage::_SendRequest(CefRefPtr<CefBrowser> browser, CefProcessId id, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue>& response, int timeout/* = 0*/)
{
	bool ret = true;
	unsigned int request_id = 0;
	HANDLE hEvent = NULL;
	std::pair<REQUEST_MAP::iterator, bool> ins_ret;
	if ( timeout > 0 )
	{
		request_id = generateID();		
		CefRefPtr<RequestContext> val = new RequestContext();
		hEvent = val->m_hAck;
		ins_ret = request_.insert(std::make_pair(request_id, val));
		ret = ins_ret.second;
	}
	
	browser->SendProcessMessageEx(id, msg, true, -1, request_id, timeout>0? true : false);
	if ( timeout > 0 )
	{
		if ( id == PID_RENDERER )
		{
			WaitWithMessageLoop(hEvent, timeout);
		}
		else{
			//DWORD dwMaxTick = timeout == INFINITE ? INFINITE : GetTickCount() + timeout;
			DWORD dwBegin = GetTickCount();
			/*while (true)
			{
				Sleep(0);
				CefDoMessageLoopWork();
				if (GetTickCount() - dwBegin > timeout){
					break;
				}
			}*/
		}
		response = ins_ret.first->second->m_val;
		request_.erase(ins_ret.first);
	}
	return ret;
}

bool Bridage::ProcRequest(CefRefPtr<ClientApp> app, CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> msg, CefRefPtr<CefListValue> response, bool& responseAck)
{
	bool bRet = false;
	boost::hash<std::string> string_hash;
	std::string name("rsp_");
	name.append(msg->GetName().ToString());
	size_t key = string_hash(name);
	ResponseMap::iterator it = responseMap_.find(key);
	if ( it != responseMap_.end() )
	{
		//it->second(app, browser, msg, response, responseAck);
		ResponseCallback cbFun = it->second;
		if ( cbFun )
		{
			bRet = true;
			cbFun(app, browser, msg,  response, responseAck);
		}
	}
	else{
		assert(false);
	}
	return bRet;
}

bool Bridage::ProcResponse(CefRefPtr<CefBrowser> browser, int request_id, bool succ, CefRefPtr<CefListValue>& response)
{
	bool bRet = false;
	CefString ss = response->GetString(0);
	REQUEST_MAP::iterator it = request_.find(request_id);
	if ( it != request_.end() )
	{
		bRet = true;
		it->second->m_val = response->Copy();
		SetEvent(it->second->m_hAck);
	}
	return bRet;
}