#ifndef sharehelper_h
#define sharehelper_h
#pragma once

namespace cyjh{

	DWORD WaitWithMessageLoop(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds);

	DWORD WaitForMultiEvent(HANDLE* hEvent, DWORD nCount, DWORD dwMilliseconds);

}
#endif