#ifndef sharehelper_h
#define sharehelper_h
#pragma once

#include <string>

namespace cyjh{

	DWORD WaitWithMessageLoop(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds);

	DWORD WaitForMultiEvent(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds);

#define PICK_MEMBER_FUN_NAME(x) x_funName(x).c_str()
	std::string x_funName(char* name);

}
#endif