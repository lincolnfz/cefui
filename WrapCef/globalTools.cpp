#include "stdafx.h"
#include "globalTools.h"
#include <Windows.h>
#include "json/json.h"

std::wstring Utf82Unicode(const std::string& str)
{
	int iTextLen = 0;
	iTextLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if ( iTextLen <= 0 )
	{
		return std::wstring(L"");
	}
	WCHAR* wsz = new WCHAR[iTextLen + 1];
	memset((void*)wsz, 0, sizeof(WCHAR) * (iTextLen + 1));
	::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wsz, iTextLen);
	std::wstring strText(wsz);
	delete[]wsz;
	return strText;
}

std::string Unicode2Utf8(const std::wstring& str)
{
	int iTextLen = 0;
	iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
	if ( iTextLen <= 0 )
	{
		return std::string("");
	}

	char* sUtf8 = new char[iTextLen + 1];
	memset((void*)sUtf8, 0, sizeof(char) * (iTextLen + 1));
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, sUtf8, iTextLen, NULL, NULL);
	std::string strText(sUtf8);
	delete[]sUtf8;
	return strText;
}

bool TransValueType(CefRefPtr<CefV8Value> val, std::string& trans)
{
	bool ret = false;
	if (val.get() == nullptr)
	{
		return false;
	}
	Json::Value root;
	if (val->IsBool())
	{
		root["type"] = "bool";
		ret = true;
	}
	else if (val->IsInt())
	{
		root["type"] = "int";
		ret = true;
	}
	else if (val->IsDouble())
	{
		root["type"] = "double";
		ret = true;
	}
	else if (val->IsString())
	{
		root["type"] = "double";
		ret = true;
	}

	if (ret)
	{
		Json::FastWriter write;
		trans = write.write(root);
	}

	return ret;
}

bool TransValue2Wstr(CefRefPtr<CefV8Value> val, std::wstring& trans)
{
	bool ret = false;
	if (val.get() == nullptr)
	{
		return false;
	}

	if ( val->IsNull() )
	{
		//trans = L"null";
		ret = false;
	}
	else if ( val->IsUndefined() )
	{
		//trans = L"undefined";
		ret = false;
	}
	else if (val->IsBool())
	{
		trans = val->GetBoolValue() ? L"true" : L"false";
		ret = true;
	}
	else if (val->IsInt())
	{
		WCHAR buf[64] = {0};
		swprintf_s(buf, L"%d", val->GetIntValue());
		trans = buf;
		ret = true;
	}
	else if (val->IsDouble())
	{
		WCHAR buf[64] = {0};
		swprintf_s(buf, L"%f", val->GetDoubleValue() );
		trans = buf;
		ret = true;
	}
	else if (val->IsString())
	{
		trans = val->GetStringValue().ToWString();
		ret = true;
	}

	return ret;
}