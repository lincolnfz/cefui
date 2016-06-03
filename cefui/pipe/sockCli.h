#ifndef sockCli_h
#define sockCli_h
#pragma once
#include "sockBase.h"

namespace cyjh{

	typedef boost::function<void()> ConnectCb;
	typedef boost::function<void(int)> ErrorCb;
	typedef boost::function<void(const unsigned char*, const int)> CliRecvDataCb;
	class SockCli : public SockBase
	{
	public:
		enum ERR_CODE{
			ERROR_CODE_BEGIN,
			ERROR_CODE_EOF,
			ERROR_CODE_ERROR,
			ERROR_CODE_TIMEOUT,

			//////////////////////////////////////////////////////////////////////////
			//end
			ERROR_CODE_END,
		};
		struct connect_parm
		{
			char ip_[64];
			int port_;
			SockCli* cli_;
			connect_parm(SockCli* cli, const char* ip, int port)
			{
				strcpy_s(ip_, ip);
				port_ = port;
				cli_ = cli;
			}
		};

		struct _cli_send_context{
			unsigned char* data_;
			int len_;
			struct event* send_sig_;
			SockCli* cli_;
			_cli_send_context(SockCli* cli, const unsigned char* data, int len)
			{
				cli_ = cli;
				data_ = new unsigned char[len+1];
				data_[len] = '\0';
				memcpy_s( data_, len, data, len );
				len_ = len;
			}

			~_cli_send_context(){
				delete []data_;
			}
		};

		struct _cli_disconnect_context{
			struct event* ev_;
			SockCli* cli_;

			_cli_disconnect_context(SockCli* cli){
				cli_ = cli;
				ev_ = NULL;
			}
		};

	public:
		SockCli();
		virtual ~SockCli();
		void Connect(const char* ip, int port);
		bool Disconnect();
		virtual void Run() override{}
		bool SendData(const unsigned char* data, int len);

		template<typename T>
		void RegisterConnectFunction( void (T::*function)() , T* obj  )
		{
			m_connectCb = boost::bind( function , obj );
		}

		template<typename T>
		void RegisterErrorFunction( void (T::*function)(int) , T* obj  )
		{
			m_errorCb = boost::bind( function , obj , _1 );
		}

		template<typename T>
		void RegisterRecvDataFunction( void (T::*function)( const unsigned char*, const int) , T* obj  )
		{
			m_recvCb = boost::bind( function , obj , _1, _2);
		}

	protected:
		void Working(const char* ip, int port);
		static void read_cb(struct bufferevent *bev, void *arg);
		static void write_cb(struct bufferevent *bev, void *arg);
		static void write_event_cb(evutil_socket_t writefd, short event, void *arg);
		static void event_cb(struct bufferevent *bev, short event, void *arg);
		virtual void Cleanup() override;
		static void sig_disconnst_cb(evutil_socket_t sig, short which,void *arg);
		static unsigned int __stdcall workThread(void *);

	private:
		struct event* m_read_event;
		bool m_bConnect;
		struct bufferevent* m_bev;
		unsigned char m_sendBuf[_BUFFER_SIZE+1];
		unsigned char m_recvBuf[_BUFFER_SIZE+1];
		ConnectCb m_connectCb;
		ErrorCb m_errorCb;
		CliRecvDataCb m_recvCb;
	};
}

#endif