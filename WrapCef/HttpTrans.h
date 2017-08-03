#pragma once

class HttpTrans
{
public:
	virtual ~HttpTrans();
	static HttpTrans& getInstance(){
		return s_inst;
	}

	void sendData(const int& id, const char* url, const char* proxy, const char* data,
		const bool& post, const char* cookie, const char* header, const unsigned int timeout = 20000);

protected:
	HttpTrans();
	static HttpTrans s_inst;

	void recvData(const int&, const std::string&, const int&);
};

