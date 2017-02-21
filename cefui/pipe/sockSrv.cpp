#include "stdafx.h"
#include "sockSrv.h"

#define _BACKLOG 5

namespace cyjh{

SockSrv::SockSrv()
{
	m_args = NULL;
	m_listen_event = NULL;
	m_endEvent = INVALID_HANDLE_VALUE;
}

SockSrv::~SockSrv()
{
	if ( m_args )
	{
		delete m_args;
	}
	if ( m_endEvent != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_endEvent);
	}
	
}

bool SockSrv::Init(const int port, int& outPort)
{
	bool ret = false;
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (m_sock == INVALID_SOCKET){
		return false;
	}
	evutil_make_listen_socket_reuseable(m_sock);

	m_port = port;
	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(m_port);
	my_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//INADDR_ANY;

	int iResult = SOCKET_ERROR;
	while( iResult == SOCKET_ERROR){
		iResult = ::bind(m_sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));		
		if ( !iResult )
		{
			break;
		}
		++m_port;
		if ( m_port > 65533 )
		{
			return false;
		}
		my_addr.sin_port = htons(m_port);
	}

	outPort = m_port;
	ret = true;
	return ret;
}

unsigned int __stdcall SockSrv::listenThread(void * parm)
{
	SockSrv* srv = (SockSrv*)parm;
	listen(srv->m_sock, _BACKLOG);

	evutil_make_socket_nonblocking(srv->m_sock);

	if(srv->InitEventBase()){
		evthread_make_base_notifiable(srv->m_base);
		srv->m_args = new _srv_arg(srv->m_base, srv);
		srv->m_listen_event = event_new(srv->m_base, srv->m_sock, EV_READ|EV_PERSIST, accept_cb, (void*)srv->m_args);
		event_base_set( srv->m_base, srv->m_listen_event );
		event_add(srv->m_listen_event, NULL);
	}
	srv->Loop();
	SetEvent(srv->m_endEvent);
	return 0;
}

void SockSrv::Run()
{
	unsigned int id;
	if ( m_sock != INVALID_SOCKET )
	{
		m_endEvent = CreateEventW(NULL, TRUE, FALSE, NULL );
		HANDLE ht = (HANDLE)_beginthreadex( NULL, 0, listenThread, this, 0, &id );
		CloseHandle(ht);
	}
}

void SockSrv::Loop()
{
	if ( m_base )
	{
		event_base_dispatch(m_base);
		Cleanup();
		m_base_mutex_.lock();
		event_base_free(m_base);
		m_base = NULL;
		m_base_mutex_.unlock();
	}
}

void SockSrv::Cleanup()
{
	if ( m_listen_event )
	{
		event_free(m_listen_event);
	}
	SockBase::Cleanup();
}

void SockSrv::accept_cb(evutil_socket_t listener, short event, void *arg)
{
	_srv_arg *context = (_srv_arg*)arg;
	evutil_socket_t fd;
	struct sockaddr_in sin;
	int slen = sizeof(sin);
	fd = accept(listener, (struct sockaddr *)&sin, &slen);
	if (fd < 0) {
		perror("accept");
		return;
	}
	//if (fd > FD_SETSIZE) { //这个if是参考了那个ROT13的例子，貌似是官方的疏漏，从select-based例子里抄过来忘了改
	//	perror("fd > FD_SETSIZE\n");
	//	return;
	//}

	printf("ACCEPT: fd = %u\n", fd);

	//unsigned int id;
	//_srv_context* parm = new _srv_context(context->srv_, fd);
	//HANDLE ht = (HANDLE)_beginthreadex( NULL, 0, clientWorkThread, parm, 0, &id );
	//CloseHandle(ht);

	struct bufferevent *bev = bufferevent_socket_new(context->srv_->m_base , fd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
	bufferevent_enable(bev, EV_READ|EV_PERSIST);

	context->srv_->m_ev_items_mutex_.lock();
	ev_item item(fd, context->srv_->m_base, bev);
	context->srv_->m_ev_items_.insert( std::make_pair(fd,  item ));
	context->srv_->m_ev_items_mutex_.unlock();

	if(context->srv_->m_new_fd_cb){
		context->srv_->m_new_fd_cb(fd);
	}
}

/*unsigned int __stdcall SockSrv::clientWorkThread(void * parm)
{
	_srv_context* arg = (_srv_context*)parm;
	struct event_base* base = event_base_new();
	arg->base_ = base;
	evthread_make_base_notifiable(base);
	struct bufferevent *bev = bufferevent_socket_new(base , arg->fd_, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
	bufferevent_enable(bev, EV_READ|EV_PERSIST);

	arg->srv_->m_ev_items_mutex_.lock();
	ev_item item(arg->fd_, base, bev);
	arg->srv_->m_ev_items_.insert( std::make_pair(arg->fd_,  item ));
	arg->srv_->m_ev_items_mutex_.unlock();

	if(arg->srv_->m_new_fd_cb){
		arg->srv_->m_new_fd_cb(arg->fd_);
	}
	event_base_dispatch(base);

	if(arg->srv_->m_remove_fd_cb){
		arg->srv_->m_remove_fd_cb(arg->fd_);
	}
	arg->srv_->m_ev_items_mutex_.lock();
	std::map<int, ev_item>::iterator it = arg->srv_->m_ev_items_.find(arg->fd_);
	if ( it != arg->srv_->m_ev_items_.end() )
	{
		arg->srv_->m_ev_items_.erase(it);
	}
	arg->srv_->m_ev_items_mutex_.unlock();
	event_base_free(base);
	delete arg;
	return 0;
}*/

void SockSrv::write_event_cb(evutil_socket_t writefd, short event, void *arg)
{
	send_context* context = (send_context*)arg;
	int remind = context->len_;
	unsigned char* buf = context->data_;
	int nSend = (std::min)(remind, _BUFFER_SIZE);
	while ( remind > 0 )
	{
		if( bufferevent_write(context->bev_, buf, nSend ) == 0 ){
			remind -= nSend;
			buf += nSend;
		}else{
			assert(false);
			break;
		}
		nSend = (std::min)(remind, _BUFFER_SIZE);
	}
	assert(remind == 0);
	event_free(context->event_);
	delete context;
}

void SockSrv::read_cb(struct bufferevent *bev, void *arg)
{
	_srv_context* context = (_srv_context*)arg;
	int n = 0;
	evutil_socket_t fd = bufferevent_getfd(bev);

	static unsigned char recvBuf[_BUFFER_SIZE+1];
	while (n = bufferevent_read(bev, recvBuf, _BUFFER_SIZE), n > 0)
	{
		//line[n] = '\0';
		//printf("fd=%u, read line: %s\n", fd, line);
		//char sz[] = "dfsade33";
		//bufferevent_write(bev, sz, strlen(sz));
		if ( context->srv_->m_recv_cb )
		{
			context->srv_->m_recv_cb(fd, recvBuf, n);
		}
	}
}

void SockSrv::write_cb(struct bufferevent *bev, void *arg)
{
	struct evbuffer * output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");
		bufferevent_free(bev);
	}
}


void SockSrv::error_cb(struct bufferevent *bev, short event, void *arg)
{
	_srv_context* context = (_srv_context*)arg;
	evutil_socket_t fd = bufferevent_getfd(bev);
	printf("fd = %u, ", fd);
	if (event & BEV_EVENT_TIMEOUT) {
		closesocket(fd);
		printf("Timed out\n"); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF) {
		closesocket(fd);
		printf("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR) {
		closesocket(fd);
		printf("some other error\n");
	}
	bufferevent_free(bev);

	if(context->srv_->m_remove_fd_cb){
		context->srv_->m_remove_fd_cb(fd);
	}
	context->srv_->m_ev_items_mutex_.lock();
	std::map<int, ev_item>::iterator it = context->srv_->m_ev_items_.find(fd);
	if ( it != context->srv_->m_ev_items_.end() )
	{
		context->srv_->m_ev_items_.erase(it);
	}
	context->srv_->m_ev_items_mutex_.unlock();
	//event_base_loopexit(context->base_, NULL );
}

void SockSrv::WatiEnd()
{
	if ( m_endEvent != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject(m_endEvent, INFINITE);
	}
}

void SockSrv::Send(int fd, const unsigned char* data, const int len)
{
	boost::mutex::scoped_lock lock_scope_base(m_base_mutex_);
	if ( !m_base )
	{
		return;
	}

	boost::mutex::scoped_lock lock_scope(m_ev_items_mutex_);
	struct event* ev = NULL;
	std::map<int, ev_item>::iterator it = m_ev_items_.find(fd);
	if ( it != m_ev_items_.end() )
	{
		send_context* context = new send_context(it->second.bev_, data, len);
		struct event* ev_write = event_new(it->second.base_,  -1, EV_WRITE, write_event_cb, context);
		context->event_ = ev_write;
		event_base_set(it->second.base_, ev_write);
		timeval tv = {0,0};
		event_add( ev_write, &tv);
	}
}

void SockSrv::stop_event_cb(evutil_socket_t writefd, short event, void *arg)
{
	stop_context* parm = (stop_context*)arg;
	event_free(parm->event_);
	event_base_loopexit(parm->base_, NULL);
	delete parm;
}

void SockSrv::Stop()
{
	boost::mutex::scoped_lock lock_scope_base(m_base_mutex_);
	stop_context * context = new stop_context;
	context->base_ = m_base;
	struct event* ev_stop = event_new(m_base,  -1, EV_WRITE, stop_event_cb, context);
	context->event_ = ev_stop;
	event_base_set(m_base, ev_stop);
	timeval tv = {0,0};
	event_add( ev_stop, &tv);
}

}