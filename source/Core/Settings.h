#pragma once

#include "Platform/Types.h"

#include <string.h>

class Settings
{
public:
	void Load() {
	//	const char *test = "Graphic Test";
	//	memcpy(m_Title, test, strlen(test));
	}

	uint32 getFrameWidth()  { return m_Width; }
	uint32 getFrameHeight() { return m_Height; }

	const char *getTitleName() { return m_Title; }

private:
	Settings() {}
	~Settings() {}

	uint32 m_Width  = 1024;
	uint32 m_Height = 768;

	const char *m_Title = "Graphic Test";

public:
	static Settings& Instance()
	{
		static Settings settings;

		return settings;
	}

	Settings(Settings const&) = delete;
	void operator=(Settings const&) = delete;
};
