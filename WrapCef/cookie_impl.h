#ifndef _cookie_impl_h
#define _cookie_impl_h

#include "cefclient.h"


extern std::wstring g_strGlobalCachePath;

class ClientRequestContextHandler : public CefRequestContextHandler {
public:
	ClientRequestContextHandler() {}

	/*bool OnBeforePluginLoad(const CefString& mime_type,
		const CefString& plugin_url,
		const CefString& top_origin_url,
		CefRefPtr<CefWebPluginInfo> plugin_info,
		PluginPolicy* plugin_policy) OVERRIDE{
		// Always allow the PDF plugin to load.
		if (*plugin_policy != PLUGIN_POLICY_ALLOW &&
		mime_type == "application/pdf") {
			*plugin_policy = PLUGIN_POLICY_ALLOW;
			return true;
		}

		return false;
	}*/

private:
	IMPLEMENT_REFCOUNTING(ClientRequestContextHandler);
};

class RequestContextHandlerPath : public CefRequestContextHandler {
public:
	explicit RequestContextHandlerPath(const WCHAR* cookie_path)
	{
		cookie_ctx_ = cookie_path;
	}

	virtual CefRefPtr<CefCookieManager> GetCookieManager() OVERRIDE;
	
private:
	IMPLEMENT_REFCOUNTING(RequestContextHandlerPath);
	std::wstring cookie_ctx_;
};

typedef std::map<size_t, CefRefPtr<CefCookieManager>> COOKIES_MAN_MAP;

class CookiesManage
{
public:
	static CookiesManage& getInst(){
		return s_inst;
	}
	virtual ~CookiesManage(){}
	CefRefPtr<CefCookieManager> GetCookieManager(const WCHAR* cookie_ctx){
		std::wstring path(cookie_ctx);
		boost::hash<std::wstring> string_hash;
		size_t hash = string_hash(path);
		COOKIES_MAN_MAP::iterator it = map_.find(hash);
		CefRefPtr<CefCookieManager> item;
		if (it != map_.end())
		{
			item = it->second;
		}
		else{
			item = CefCookieManager::CreateManager(CefString(path), false, NULL);
			map_.insert(std::make_pair(hash, item));
		}

		COOKIES_MAN_MAP::iterator it2 = map_.find(hash);
		CefRefPtr<CefCookieManager> val;
		if (it2 != map_.end())
		{
			val = it2->second;
		}
		return val;
	}

	CefRefPtr<CefRequestContext> getShareRequest();
protected:
	CookiesManage(){}
private:
	static CookiesManage s_inst;
	COOKIES_MAN_MAP map_;
	CefRefPtr<CefRequestContext> shared_request_context_;
};


#endif
