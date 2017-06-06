#include "stdafx.h"
#include "cookie_impl.h"

CookiesManage CookiesManage::s_inst;

CefRefPtr<CefCookieManager> RequestContextHandlerPath::GetCookieManager()
{
	CefRefPtr<CefCookieManager> manager = CookiesManage::getInst().GetCookieManager(cookie_ctx_.c_str());
	if (manager.get())
	{
		return manager;
	}
	return NULL;
}

CefRefPtr<CefRequestContext> CookiesManage::getShareRequest()
{
	if (!shared_request_context_.get()) {
		shared_request_context_ =
			CefRequestContext::CreateContext(CefRequestContext::GetGlobalContext(),
			new ClientRequestContextHandler);
	}
	return shared_request_context_;
}