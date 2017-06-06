#ifndef _globalTools_h
#define _globalTools_h
#include <string>
#include "include/cef_app.h"

std::wstring Utf82Unicode(const std::string& str);

std::string Unicode2Utf8(const std::wstring& str);

bool TransValueType(CefRefPtr<CefV8Value> val, std::string& trans);

bool TransValue2Wstr(CefRefPtr<CefV8Value> val, std::wstring& trans);

bool getAppDataFolder(std::wstring& directory);

bool getAppDataFolder(std::string& directory);

#endif //_globalTools_h