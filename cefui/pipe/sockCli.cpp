#include "stdafx.h"
#include "sockCli.h"


namespace cyjh{

	SockCli::SockCli()
	{
		m_read_event = NULL;
		m_bConnect = false;
		m_bev = NULL;
	}

	SockCli::~SockCli()
	{

	}

	void SockCli::Connect(const char* ip, int port)
	{
		connect_parm* parm = new connect_parm(this, ip, port);
		unsigned int id;
		HANDLE ht = (HANDLE)_beginthreadex( NULL, 0, workThread, parm, 0, &id );
		CloseHandle(ht);
	}

	unsigned int __stdcall SockCli::workThread(void * args)
	{
		connect_parm* parm = (connect_parm*)(args);
		parm->cli_->Working(parm->ip_, parm->port_);
		delete parm;
		return 0;
	}

	void SockCli::Working(const char* ip, int port)
	{
		//m_base_mutex_.lock();
		bool bCreate = InitEventBase();
		//m_base_mutex_.unlock();
		if(bCreate){
			evthread_make_base_notifiable(m_base);
			int on = 1;  
			m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
			if (m_sock == INVALID_SOCKET){
				return;
			}
			//evutil_make_socket_nonblocking(m_sock);  
#ifdef WIN32  
			setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof(on));  
#else  
			setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));  
#endif  
			evutil_make_socket_nonblocking(m_sock);

			//buffer
			m_bev = bufferevent_socket_new(m_base, m_sock, BEV_OPT_CLOSE_ON_FREE);
			bufferevent_setcb(m_bev, read_cb, NULL, event_cb, this);
			bufferevent_enable(m_bev, EV_READ|EV_PERSIST);

			struct sockaddr_in serverAddr; 
			serverAddr.sin_family = AF_INET;  
			serverAddr.sin_port = htons(port);  
#ifdef WIN32  
			serverAddr.sin_addr.S_un.S_addr = inet_addr(ip);  
#else  
			serverAddr.sin_addr.s_addr = inet_addr(ECHO_SERVER);  
#endif  
			memset(serverAddr.sin_zero, 0x00, sizeof(serverAddr.sin_zero));			

			/*if( connect(m_sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR ){
				if( WSAEWOULDBLOCK != WSAGetLastError()){
					if ( m_errorCb )
					{
						m_errorCb(ERROR_CODE_EOF);
					}
				}
			}else{
				if ( m_connectCb )
				{
					m_connectCb();
				}
			}*/
			if(bufferevent_socket_connect(m_bev, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0)
				event_base_dispatch(m_base);

			m_base_mutex_.lock();
			Cleanup();
			event_base_free(m_base);
			m_base = NULL;
			m_base_mutex_.unlock();
		}

	}

	void SockCli::read_cb(struct bufferevent *bev, void *arg)
	{
		SockCli* cli = (SockCli*)arg;
		int n = 0;
		evutil_socket_t fd = bufferevent_getfd(bev);

		static unsigned char recvBuf[_BUFFER_SIZE+1];
		while (n = bufferevent_read(bev, recvBuf, _BUFFER_SIZE), n > 0)
		{
			if ( cli->m_recvCb )
			{
				cli->m_recvCb(recvBuf, n);
			}
		}
	}

	void SockCli::write_cb(struct bufferevent *bev, void *arg)
	{
		//bufferevent_disable(bev, EV_WRITE);
	}

	void SockCli::event_cb(struct bufferevent *bev, short events, void *arg)
	{
		SockCli* cli = (SockCli*)(arg);
		if (events & BEV_EVENT_EOF) {
			closesocket( cli->m_sock );
			if ( cli->m_errorCb )
			{
				cli->m_errorCb(ERROR_CODE_EOF);
			}
			printf("Connection closed.\n");
		  } else if (events & BEV_EVENT_ERROR) {
			  closesocket( cli->m_sock );
			  if ( cli->m_errorCb )
			  {
				  cli->m_errorCb(ERROR_CODE_ERROR);
			  }
			printf("Got an error on the connection");/*XXX win32*/
		  } else if (events & BEV_EVENT_TIMEOUT) {
			  closesocket( cli->m_sock );
			  if ( cli->m_errorCb )
			  {
				  cli->m_errorCb(ERROR_CODE_TIMEOUT);
			  }
			  printf("Connection time out.\n");
		  } else if ( events & BEV_EVENT_CONNECTED )
		  {
			  if ( cli->m_connectCb )
			  {
				  cli->m_connectCb();
			  }
			  return;
		  }else {
			  return;
		  }
		  /* None of the other events can happen here, since we haven't enabled
		   * timeouts */
		  bufferevent_free(bev);
		  event_base_loopexit(cli->m_base, NULL);
	}

	void SockCli::write_event_cb(evutil_socket_t writefd, short event, void *arg)
	{
		_cli_send_context* context = (_cli_send_context*)arg;
		int remind = context->len_;
		unsigned char* buf = context->data_;
		int nSend = (std::min)(remind, _BUFFER_SIZE);
		while ( remind > 0 )
		{
			if( bufferevent_write(context->cli_->m_bev , buf, nSend ) == 0 ){
				remind -= nSend;
				buf += nSend;
			}else{
				assert(false);
				break;
			}
			nSend = (std::min)(remind, _BUFFER_SIZE);
		}
		assert(remind == 0);
		event_free(context->send_sig_);
		delete context;
	}

	bool SockCli::SendData(const unsigned char* data, int len)
	{
		bool bret = false;
		std::unique_lock<std::mutex> lock_scope(m_base_mutex_);
		if ( m_base )
		{
			_cli_send_context* context = new _cli_send_context(this, data, len);
			struct event* ev_write = event_new(m_base,  -1, EV_WRITE, write_event_cb, context);
			context->send_sig_ = ev_write;
			event_base_set(m_base, ev_write);
			timeval tv = {0,0};
			event_add(ev_write, &tv);
			bret = true;
		}
		return bret;
	}


	void SockCli::Cleanup()
	{
		SockBase::Cleanup();
	}

	void SockCli::sig_disconnst_cb(evutil_socket_t sig, short which,void *arg)
	{
		_cli_disconnect_context* context = (_cli_disconnect_context*)arg;
		event_free(context->ev_);
		closesocket( context->cli_->m_sock );
		bufferevent_free(context->cli_->m_bev);
		event_base_loopexit(context->cli_->m_base, NULL);
		delete context;		
		
	}

	bool SockCli::Disconnect()
	{
		bool bret = false;
		std::unique_lock<std::mutex> lock_scope(m_base_mutex_);
		if ( m_base )
		{
			_cli_disconnect_context* context = new _cli_disconnect_context(this);
			struct event* ev_disconnect = event_new(m_base,  -1, EV_WRITE, sig_disconnst_cb, context);
			context->ev_ = ev_disconnect;
			event_base_set(m_base, ev_disconnect);
			timeval tv = {0,0};
			event_add(ev_disconnect, &tv);
		}
		return true;
	}
}