#ifndef sockSrv_h
#define sockSrv_h
#pragma once

#include "sockBase.h"
#include <map>

namespace cyjh{

	struct  ev_item
	{
		int fd_;
		struct bufferevent *bev_;
		struct event_base* base_;
		ev_item(int fd, struct event_base* base, struct bufferevent* bev)
		{
			fd_ = fd;
			base_ = base;
			bev_ = bev;
		}
	};

	struct send_context
	{
		struct bufferevent* bev_;
		unsigned char* data_;
		struct event* event_;
		int len_;
		send_context(struct bufferevent* bev, const unsigned char* data, const int len){
			bev_ = bev;
			data_ = new unsigned char[len + 1];
			memcpy_s(data_, len,  data, len );
			len_ = len;
		}

		~send_context(){
			delete []data_;
		}
	};

	struct stop_context
	{
		struct event* event_;
		struct event_base* base_;
	};

	typedef boost::function<void(int)> NotifyNewFd;
	typedef boost::function<void(int)> NotifyRemoveFd;
	typedef boost::function<void(const int, const unsigned char*, const int)> NotifyRecvData;

	class SockSrv : public SockBase
	{
	public:
		struct _srv_context{
			int fd_;
			SockSrv* srv_;
			event_base* base_;
			_srv_context(SockSrv* srv,int fd)
			{
				srv_ = srv;
				fd_ = fd;
				base_ = NULL;
			}

			~_srv_context(){
			}
		};

		struct _srv_arg 
		{
			struct event_base* base_;
			SockSrv* srv_;
			_srv_arg(struct event_base* base, SockSrv* srv){
				base_ = base;
				srv_ = srv;
			}
		};

		typedef std::map<int, struct bufferevent*> BUFFER_EVENT_MAP;

	public:
		SockSrv();
		virtual ~SockSrv();
		virtual bool Init(const int port, int& outPort);
		virtual void Run() override;
		void WatiEnd();
		void Send(int fd, const unsigned char* , const int);
		void Stop();

		template<typename T>
		void RegisterNewFdFunction( void (T::*function)(int) , T* obj  )
		{
			m_new_fd_cb = boost::bind( function , obj , _1 );
		}

		template<typename T>
		void RegisterRemoveFdFunction( void (T::*function)(int) , T* obj  )
		{
			m_remove_fd_cb = boost::bind( function , obj , _1 );
		}

		template<typename T>
		void RegisterRecvDataFunction( void (T::*function)(const int, const unsigned char*, const int) , T* obj  )
		{
			m_recv_cb = boost::bind( function , obj , _1, _2, _3);
		}

	protected:
		static void accept_cb(evutil_socket_t listener, short event, void *arg);
		static void write_event_cb(evutil_socket_t writefd, short event, void *arg);
		static void stop_event_cb(evutil_socket_t writefd, short event, void *arg);
		static void read_cb(struct bufferevent *bev, void *arg);
		static void write_cb(struct bufferevent *bev, void *arg);
		static void error_cb(struct bufferevent *bev, short event, void *arg);
		static unsigned int __stdcall listenThread(void *);
		virtual void Cleanup() override;
		virtual void Loop() override;
		//static unsigned int __stdcall clientWorkThread(void *);

	private:
		int m_port;
		_srv_arg* m_args;
		struct event *m_listen_event;
		HANDLE m_endEvent;
		std::map<int, ev_item> m_ev_items_;
		boost::mutex m_ev_items_mutex_;
		NotifyNewFd m_new_fd_cb;
		NotifyRemoveFd m_remove_fd_cb;
		NotifyRecvData m_recv_cb;
	};

}
#endif