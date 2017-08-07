#pragma once
#include <mutex>
#include <map>
#include <memory>
#include <include\cef_base.h>

struct send_context_ 
{
	int pageid_;
	std::string req_id_;
};

struct resp_context_
{
	int errcode_;
	unsigned int id_;
	std::string head_;
	std::string body_;
};

class HttpTrans// : public CefBase
{
public:
	friend void ackData(std::shared_ptr<resp_context_> parm);
	virtual ~HttpTrans();
	static HttpTrans& getInstance(){
		return s_inst;
	}

	bool sendData(const int& pageid, const char* id, const char* url, const char* proxy, const char* data,
		const bool& post, const char* header, const unsigned int timeout = 20000);

	void abort(const int& pageid, const char* id);

protected:
	HttpTrans();
	static HttpTrans s_inst;

	void recvData(const int&, FILE*, const int&, const int& reDirectCount);

	

	//std::mutex lock_;
	std::map<unsigned int, std::shared_ptr<send_context_>> TransMap_;

	//IMPLEMENT_REFCOUNTING(HttpTrans);
};

