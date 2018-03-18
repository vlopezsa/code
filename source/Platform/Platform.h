#pragma once

#include "Platform/Types.h"

class Platform
{
public:
	void StdOut(const char *str);
	void StdErr(const char *str);

private:
	Platform();
	~Platform();

public:
	static Platform& Instance()
	{
		static Platform platform;

		return platform;
	}

	Platform(Platform const&) = delete;
	void operator=(Platform const&) = delete;
};

