#include "stdafx.h"
#include "ResponseUI.h"
#include "IPC.h"
#include "WebViewFactory.h"


ResponseUI::ResponseUI()
{
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_RegisterBrowser);
	REGISTER_RESPONSE_FUNCTION(ResponseUI, rsp_getPrivateProfileString);
}


ResponseUI::~ResponseUI()
{
}

bool ResponseUI::rsp_RegisterBrowser(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> reqParm, std::shared_ptr<cyjh::Instruct> )
{
	std::wstring szSrvPipe = reqParm->getList().GetWStrVal(0);
	std::wstring szCliPipe = reqParm->getList().GetWStrVal(1);
	int ipcID = cyjh::IPC_Manager::getInstance().MatchIpc(szSrvPipe.c_str(), szCliPipe.c_str());
	CefRefPtr<WebItem> item = WebViewFactory::getInstance().GetBrowserItem(browser->GetIdentifier());
	if ( item.get() )
	{
		item->m_ipcID = ipcID;
	}
	else{
		assert(false);
	}
	return true;
}

#include "client_app.h"
bool ResponseUI::rsp_getPrivateProfileString(const CefRefPtr<CefBrowser> browser, const std::shared_ptr<cyjh::Instruct> reqParm, std::shared_ptr<cyjh::Instruct> out)
{
	{
		//≤‚ ‘¥˙¬Î
		cyjh::Instruct parm;
		//parm.setName(__FUNCTION__);
		parm.setName("invokedJSMethod");
		std::wstring s(L"458");
		parm.getList().AppendVal(s);
		CefRefPtr<cyjh::UIThreadCombin> ui = ClientApp::getGlobalApp()->getUIThreadCombin();
		std::shared_ptr<cyjh::Instruct> outVal;
		ui->Request(browser, parm, outVal);
		int i = 0;
	}
	out->getList().AppendVal(std::wstring(L"a45486"));
	return true;
}