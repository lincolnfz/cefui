#include "stdafx.h"
#include "WebkitEcho.h"

wrapQweb::EchoMap* WebkitEcho::s_fnMap = NULL;

WebkitEcho::WebkitEcho()
{
	
}


WebkitEcho::~WebkitEcho()
{
}

void WebkitEcho::SetFunMap(wrapQweb::EchoMap* map)
{
	s_fnMap = map;
}
