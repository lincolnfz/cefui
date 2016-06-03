#ifndef sockBase_h
#define sockBase_h
#include <WinSock2.h>
#include <process.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>
//#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <mutex>

namespace cyjh{

#define _BUFFER_SIZE 4096

class SockBase
{
public:
	SockBase();
	virtual ~SockBase();

	virtual void Run() = 0;

protected:
	virtual bool InitEventBase();
	virtual void Loop(){}
	virtual void Cleanup(){}
	static void stop_cb(evutil_socket_t sig, short which,void *arg);

protected:
	static bool s_bInit;
	static int s_numInsts;
	struct event_base* m_base;
	SOCKET m_sock;
	//boost::mutex m_base_mutex_;
	std::mutex m_base_mutex_;
};

}

#endif