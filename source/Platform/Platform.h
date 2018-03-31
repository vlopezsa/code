#pragma once

#include "Platform/Types.h"

class Platform
{
public:
	void StdOut(const char *str);
	void StdErr(const char *str);

	void Init();

	void Release();

	void Update();

	void *GetWindowHandle() { return m_Handle; }

private:
	Platform();
	~Platform();

	void InitWindow(uint32 width, uint32 height, const char *title);

	void * m_Handle;

public:
	static Platform& Instance()
	{
		static Platform platform;

		return platform;
	}

	Platform(Platform const&) = delete;
	void operator=(Platform const&) = delete;
};

