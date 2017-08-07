#include "stdafx.h"
#include "HttpTrans.h"
#include <Winsock2.h>
#include <curl/curl.h>
#include <process.h>
#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <Shlwapi.h>
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "client_app.h"
#include "WebViewFactory.h"
#include "json/json.h"
#include "ShareHelper.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wldap32.lib")

#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

typedef boost::function<void(const int&, FILE*, const int&, const int&)> RepCB;
struct SendParm
{
	std::string  sUrl;
	std::string  sProxy;
	std::string  sData;
	//std::string  spRsp;
	std::string  header;
	unsigned int uiTimeout;
	bool post;
	RepCB rcb;
	unsigned int id;
	struct curl_slist *chunk;
	FILE* fp;
};

size_t curl_writer(void *buffer, size_t size, size_t count, void * stream)
{
	//std::string * pStream = static_cast<std::string *>(stream);
	//(*pStream).append((char *)buffer, size * count);
	FILE* fp = static_cast<FILE*>(stream);
	if ( fp )
	{
		fwrite(buffer, size, count, fp);
	}
	return size * count;
};

/**
* 生成一个easy curl对象，进行一些简单的设置操作
*/
CURL * curl_easy_handler(const std::string & sUrl,
	const std::string & sProxy,
	const std::string & sData,
	FILE* fp,
	const unsigned int& uiTimeout,
	const bool& post,
	const std::string & header,
	struct curl_slist ** chunk)
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
	fseek(fp, 0, SEEK_SET);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	if (!StrCmpNIA(sUrl.c_str(), "https", 5))
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
	if ( post )
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		if (!sData.empty())
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sData.c_str());
		}
	}	

	if ( !header.empty() )
	{
		//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		/* Remove a header curl would otherwise add by itself */
		//chunk = curl_slist_append(chunk, "Accept:");
		//*chunk = curl_slist_append(*chunk, "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Safari/537.36");
		//*chunk = curl_slist_append(*chunk, "Accept-Encoding:gzip, deflate");
		//*chunk = curl_slist_append(*chunk, header.c_str());
		/* set our custom set of headers */
		Json::Value root;
		Json::Reader read;
		if (read.parse(header.c_str(), root) && root.isArray()){
			int len = root.size();
			for (int i = 0; i < len; ++i)
			{
				std::string val = root[i].asString();
				*chunk = curl_slist_append(*chunk, val.c_str());
			}
		}
	}
	if (post)
	{
		char szbuf[256];
		sprintf_s(szbuf, "Content-length: %d", sData.size());
		*chunk = curl_slist_append(*chunk, szbuf);
	}
	if (*chunk)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *chunk);
	}
	return curl;
}

/**
* 使用select函数监听multi curl文件描述符的状态
* 监听成功返回0，监听失败返回-1
*/
int curl_multi_select(CURLM * curl_m)
{
	int ret = 0;

	struct timeval timeout_tv;
	fd_set  fd_read;
	fd_set  fd_write;
	fd_set  fd_except;
	int     max_fd = -1;

	// 注意这里一定要清空fdset,curl_multi_fdset不会执行fdset的清空操作  //  
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_except);

	// 设置select超时时间  //  
	timeout_tv.tv_sec = 1;
	timeout_tv.tv_usec = 0;

	// 获取multi curl需要监听的文件描述符集合 fd_set //  
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
	* 执行监听，当文件描述符状态发生改变的时候返回
	* 返回0，程序调用curl_multi_perform通知curl执行相应操作
	* 返回-1，表示select错误
	* 注意：即使select超时也需要返回0，具体可以去官网看文档说明
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
	* 调用curl_multi_perform函数执行curl请求
	* url_multi_perform返回CURLM_CALL_MULTI_PERFORM时，表示需要继续调用该函数直到返回值不是CURLM_CALL_MULTI_PERFORM为止
	* running_handles变量返回正在处理的easy curl数量，running_handles为0表示当前没有正在执行的curl请求
	*/
	int running_handles;
	while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles));

	/**
	* 为了避免循环调用curl_multi_perform产生的cpu持续占用的问题，采用select来监听文件描述符
	*/
	while (running_handles)
	{
		if (-1 == curl_multi_select(curl_m))
		{
			break;
		}
		else {
			// select监听到事件，调用curl_multi_perform通知curl执行相应的操作 //  
			while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles));
		}
	}

	// 输出执行结果 //  
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

int easy_curl_done(CURL* curl_e, int& reDirectCount)
{
	int iRetcode = -1;
	reDirectCount = 0;
	CURLcode code = curl_easy_perform(curl_e);
	if ( code == CURLE_OK )
	{
		curl_easy_getinfo(curl_e, CURLINFO_HTTP_CODE, &iRetcode);
		curl_easy_getinfo(curl_e, CURLINFO_REDIRECT_COUNT, &reDirectCount);
	}
	else{
		iRetcode = code;
	}
	curl_easy_cleanup(curl_e);
	return iRetcode;
}

unsigned int hashID( const int& pageid, const char* id )
{
	char szHash[256];
	sprintf_s(szHash, "%s-%d", id, pageid);
	boost::hash<std::string> string_hash;
	std::string hash(szHash);
	return string_hash(hash);
}

HttpTrans HttpTrans::s_inst;

unsigned int __stdcall sendDataThread(LPVOID parm)
{
	SendParm* send = (SendParm*)parm;
	WCHAR szTempDirectory[MAX_PATH], szTempName[MAX_PATH], szTempPath[512];
	GetTempPathW(MAX_PATH, szTempDirectory);
	GetTempFileName(szTempDirectory, TEXT("rsp_"), 0, szTempPath);
	//swprintf_s(szTempPath, L"%%s\\%s.tmp", szTempDirectory, szTempName);
	int code = -1;
	send->fp = nullptr;
	int errCode = _wfopen_s(&send->fp, szTempPath, L"wb+");
	int reDirectCount = 0;
	if ( send->fp != nullptr )
	{
		CURL * curl_e = curl_easy_handler(send->sUrl, send->sProxy, send->sData,
			send->fp, send->uiTimeout,
			send->post, send->header, &send->chunk);
		code = easy_curl_done(curl_e, reDirectCount);
		if (send->chunk)
		{
			curl_slist_free_all(send->chunk);
		}
	}	
	send->rcb(code, send->fp, send->id, reDirectCount);
	if ( send->fp )
	{
		fclose(send->fp);
		DeleteFileW(szTempPath);
	}
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

bool HttpTrans::sendData(const int& pageid, const char* id, const char* url, const char* proxy, const char* data,
	const bool& post, const char* header, const unsigned int timeout)
{
	//std::unique_lock<std::mutex> lock(lock_);
	SendParm* parm = new SendParm;
	parm->id = hashID(pageid, id);
	parm->sUrl = url;
	parm->sProxy = proxy ? proxy : "";
	parm->sData = data ? data : "";
	parm->post = post;
	parm->header = header ? header : "";
	parm->uiTimeout = timeout;
	parm->chunk = nullptr;
	parm->rcb = boost::bind(&HttpTrans::recvData, this, _1, _2, _3, _4);

	std::shared_ptr<send_context_> sp(new send_context_);
	sp->pageid_ = pageid;
	sp->req_id_ = id;
	std::pair<std::map<unsigned int, std::shared_ptr<send_context_>>::iterator, bool> ret;
	ret = TransMap_.insert(std::make_pair(parm->id, sp));
	if ( ret.second )
	{
		HANDLE ht = (HANDLE)_beginthreadex(nullptr, 0, sendDataThread, parm, 0, 0);
		CloseHandle(ht);
	}
	else{
		delete parm;
	}
	return ret.second;
}

void HttpTrans::abort(const int& pageid, const char* id)
{
	unsigned int hash = hashID(pageid, id);
	std::map<unsigned int, std::shared_ptr<send_context_>>::iterator it = TransMap_.find(hash);
	if (it != TransMap_.end())
	{
		TransMap_.erase(it);
	}
}

void ackData(std::shared_ptr<resp_context_> parm)
{
	std::map<unsigned int, std::shared_ptr<send_context_>>::iterator it = HttpTrans::getInstance().TransMap_.find(parm->id_);
	if (it != HttpTrans::getInstance().TransMap_.end())
	{
		//send
		CefRefPtr<CefBrowser>browser = WebViewFactory::getInstance().GetBrowser(it->second->pageid_);
		if ( browser.get() )
		{
			cyjh::Instruct inst;
			inst.setName("ackSendData");
			inst.getList().AppendVal(it->second->req_id_);
			inst.getList().AppendVal(parm->errcode_);
			inst.getList().AppendVal(parm->head_);
			inst.getList().AppendVal(parm->body_);
			CefRefPtr<cyjh::UIThreadCombin> ipc = ClientApp::getGlobalApp()->getUIThreadCombin();
			ipc->AsyncRequest(browser, inst);
		}
		HttpTrans::getInstance().TransMap_.erase(it);
	}
}

void HttpTrans::recvData(const int& code, FILE* fp, const int& id, const int& reDirectCount)
{
	//std::unique_lock<std::mutex> lock(lock_);
	std::wstring rsp;
	if ( fp )
	{
		fseek(fp, 0L, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if ( length > 0 )
		{
			char* szBuf = new char[length+1];
			memset(szBuf, 0, length + 1);
			if (fread_s(szBuf, length + 1, 1, length, fp) > 0){
				rsp = cyjh::UTF8ToUnicode(szBuf);
				delete[]szBuf;
			}
		}
	}
	std::shared_ptr<resp_context_> parm(new resp_context_);
	parm->id_ = id;
	parm->errcode_ = code;
	parm->head_ = " ";
	parm->body_ = " ";

	int i = 0;
	int pos = 0;
	while (i < reDirectCount)
	{
		pos = rsp.find(L"\r\n\r\n");
		if ( pos > 0 )
		{
			rsp.erase(0, pos + 4);
		}
		++i;
	}

	pos = rsp.find(L"\r\n\r\n");
	if ( pos > 0 )
	{
		std::wstring whead = rsp.substr(0, pos);
		int size = rsp.size() - pos - 4;
		if ( size > 0 )
		{
			std::wstring wbody = rsp.substr(pos + 4, size);
			parm->body_ = cyjh::UnicodeToUTF8(wbody);
		}
		parm->head_ = cyjh::UnicodeToUTF8(whead);
	}
	CefPostTask(TID_UI, base::Bind(&ackData, parm));
}