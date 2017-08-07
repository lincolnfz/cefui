#ifndef sharehelper_h
#define sharehelper_h
#pragma once

#include <string>

namespace cyjh{

	DWORD WaitWithMessageLoop(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds);

	DWORD WaitForMultiEvent(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds);

#define PICK_MEMBER_FUN_NAME(x) x_funName(x).c_str()
	std::string x_funName(char* name);

	std::string UnicodeToUTF8(const std::wstring& str);

	std::wstring UTF8ToUnicode(const std::string& str);

	std::wstring UTF8ToUnicode(const char* str);

	std::string UrlEncode(const std::wstring& str);
}
#endif