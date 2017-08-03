#include "stdafx.h"
#include "HttpTrans.h"
#include <Winsock2.h>
#include <curl/curl.h>
#include <process.h>
#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <Shlwapi.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wldap32.lib")

#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

size_t curl_writer(void *buffer, size_t size, size_t count, void * stream)
{
	std::string * pStream = static_cast<std::string *>(stream);
	(*pStream).append((char *)buffer, size * count);

	return size * count;
};

/**
* ����һ��easy curl���󣬽���һЩ�򵥵����ò���
*/
CURL * curl_easy_handler(const std::string & sUrl,
	const std::string & sProxy,
	const std::string & sData,
	std::string & sRsp,
	const unsigned int& uiTimeout,
	const bool& post,
	const std::string & cookie,
	const std::string & header)
{

	CURL * curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	if (uiTimeout > 0)
	{
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, uiTimeout);
	}
	if (!sProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, sProxy.c_str());
	}

	// write function //  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sRsp);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	if (!StrCmpNIA(sUrl.c_str(), "https", 5))
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
	if ( post )
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
	}
	if ( !sData.empty() )
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sData.c_str());
	}
	if ( !cookie.empty() )
	{
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());
	}

	if ( !header.empty() )
	{		
		//curl_easy_setopt(curl, CURLOPT_HEADERDATA, );
	}

	return curl;
}

/**
* ʹ��select��������multi curl�ļ���������״̬
* �����ɹ�����0������ʧ�ܷ���-1
*/
int curl_multi_select(CURLM * curl_m)
{
	int ret = 0;

	struct timeval timeout_tv;
	fd_set  fd_read;
	fd_set  fd_write;
	fd_set  fd_except;
	int     max_fd = -1;

	// ע������һ��Ҫ���fdset,curl_multi_fdset����ִ��fdset����ղ���  //  
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_except);

	// ����select��ʱʱ��  //  
	timeout_tv.tv_sec = 1;
	timeout_tv.tv_usec = 0;

	// ��ȡmulti curl��Ҫ�������ļ����������� fd_set //  
	curl_multi_fdset(curl_m, &fd_read, &fd_write, &fd_except, &max_fd);

	/**
	* When max_fd returns with -1,
	* you need to wait a while and then proceed and call curl_multi_perform anyway.
	* How long to wait? I would suggest 100 milliseconds at least,
	* but you may want to test it out in your own particular conditions to find a suitable value.
	*/
	if (-1 == max_fd)
	{
		return -1;
	}

	/**
	* ִ�м��������ļ�������״̬�����ı��ʱ�򷵻�
	* ����0���������curl_multi_perform֪ͨcurlִ����Ӧ����
	* ����-1����ʾselect����
	* ע�⣺��ʹselect��ʱҲ��Ҫ����0���������ȥ�������ĵ�˵��
	*/
	int ret_code = ::select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
	switch (ret_code)
	{
	case -1:
		/* select error */
		ret = -1;
		break;
	case 0:
		/* select timeout */
	default:
		/* one or more of curl's file descriptors say there's data to read or write*/
		ret = 0;
		break;
	}

	return ret;
}

CURLcode curl_multi_done(CURL* curl_e)
{
	CURLM * curl_m = curl_multi_init();
	curl_multi_add_handle(curl_m, curl_e);

	/*
	* ����curl_multi_perform����ִ��curl����
	* url_multi_perform����CURLM_CALL_MULTI_PERFORMʱ����ʾ��Ҫ�������øú���ֱ������ֵ����CURLM_CALL_MULTI_PERFORMΪֹ
	* running_handles�����������ڴ����easy curl������running_handlesΪ0��ʾ��ǰû������ִ�е�curl����
	*/
	int running_handles;
	while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles));

	/**
	* Ϊ�˱���ѭ������curl_multi_perform������cpu����ռ�õ����⣬����select�������ļ�������
	*/
	while (running_handles)
	{
		if (-1 == curl_multi_select(curl_m))
		{
			break;
		}
		else {
			// select�������¼�������curl_multi_perform֪ͨcurlִ����Ӧ�Ĳ��� //  
			while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles));
		}
	}

	// ���ִ�н�� //  
	int         msgs_left;
	CURLMsg *   msg;
	CURLcode code = CURL_LAST;
	while ((msg = curl_multi_info_read(curl_m, &msgs_left)))
	{
		if ( CURLMSG_DONE == msg->msg )
		{
			code = msg->data.result;
		}
	}
	curl_multi_remove_handle(curl_m, curl_e);

	curl_easy_cleanup(curl_e);

	curl_multi_cleanup(curl_m);

	return code;
}

typedef boost::function<void(const int&, const std::string&, const int&)> RepCB;

HttpTrans HttpTrans::s_inst;

struct SendParm
{
	std::string  sUrl;
	std::string  sProxy;
	std::string  sData;
	std::string  spRsp;
	std::string  cookie;
	std::string  header;
	unsigned int uiTimeout;
	bool post;
	RepCB rcb;
	int id;
};

unsigned int __stdcall sendDataThread(LPVOID parm)
{
	SendParm* send = (SendParm*)parm;
	CURL * curl_e = curl_easy_handler(send->sUrl, send->sProxy, send->sData,
			send->spRsp, send->uiTimeout,
			send->post, send->cookie, send->header);
	CURLcode code = curl_multi_done(curl_e);
	send->rcb(code, send->spRsp, send->id);
	delete send;
	return 0;
}

HttpTrans::HttpTrans()
{
	curl_global_init(CURL_GLOBAL_ALL);
}


HttpTrans::~HttpTrans()
{
	curl_global_cleanup();
}

void HttpTrans::sendData(const int& id, const char* url, const char* proxy, const char* data,
	const bool& post, const char* cookie, const char* header, const unsigned int timeout)
{
	SendParm* parm = new SendParm;
	parm->id = id;
	parm->sUrl = url;
	parm->sProxy = proxy ? proxy : "";
	parm->sData = data ? data : "";
	parm->post = post;
	parm->cookie = cookie ? cookie : "";
	parm->header = header ? header : "";
	parm->uiTimeout = timeout;
	parm->rcb = boost::bind(&HttpTrans::recvData, this, _1, _2, _3);
	HANDLE ht = (HANDLE)_beginthreadex(nullptr, 0, sendDataThread, parm, 0, 0);
	CloseHandle(ht);
}

void HttpTrans::recvData(const int& code, const std::string& rsp, const int& id)
{
	int pos = rsp.find("\r\n\r\n");
	std::string head, body;
	if ( pos > 0 )
	{
		head = rsp.substr(0, pos);
		int size = rsp.size() - pos - 4;
		if ( size > 0 )
		{
			body = rsp.substr(pos + 4, size);
		}
	}
}