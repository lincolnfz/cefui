#ifndef _webkitecho_h_
#define _webkitecho_h_
#pragma once

#include "WrapCef.h"

class WebkitEcho
{
public:
	WebkitEcho();
	virtual ~WebkitEcho();

	static void SetFunMap(wrapQweb::EchoMap*);
	static const wrapQweb::EchoMap* getFunMap(){
		return s_fnMap;
	}

private:
	static wrapQweb::EchoMap* s_fnMap;
};

#endif
