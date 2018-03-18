#include "Platform.h"

#include <iostream>

#include <windows.h>

Platform::Platform()
{
}


Platform::~Platform()
{
}

void Platform::StdOut(const char *str)
{
	OutputDebugStringA(str);
	OutputDebugStringA("\n");
}

void Platform::StdErr(const char *str)
{
	OutputDebugStringA(str);
	OutputDebugStringA("\n");
}