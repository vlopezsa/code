#pragma once
#include "Platform/Types.h"
#include "Platform/Thread/Atomic.h"

class Log
{
public:
#define MAX_NUM_LOG_SYSTEM 10

	enum LOG_SYSTEM
	{
		LOG_DEFAULT = 0,
		LOG_RENDER,
		LOG_GRAPHICS,
		LOG_MAX = MAX_NUM_LOG_SYSTEM
	};

public:
	void Write(LOG_SYSTEM type, const char *str, ...);

	void enableSystem(LOG_SYSTEM type);

	void disableSystem(LOG_SYSTEM type);

private:
	bool isValidSystem(LOG_SYSTEM type);

	atomic_bool m_LogSystem[MAX_NUM_LOG_SYSTEM];

private:
	Log();
	~Log();


public:
	static Log& Instance()
	{
		static Log game;

		return game;
	}

	Log(Log const&) = delete;
	void operator=(Log const&) = delete;
};


#define LOG(X, ...)		{ Log::Instance().Write(Log::LOG_DEFAULT, "LOG: "    X, __VA_ARGS__); }
#define WARNING(X, ...) { Log::Instance().Write(Log::LOG_DEFAULT, "WARNING: "X, __VA_ARGS__); }
#define ERROR(X, ...)	{ Log::Instance().Write(Log::LOG_DEFAULT, "ERROR: "  X, __VA_ARGS__); }
