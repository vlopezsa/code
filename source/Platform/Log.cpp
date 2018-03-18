#include "Log.h"
#include "Platform.h"

#include <stdio.h>
#include <stdarg.h>

Log::Log()
{
	for (uint32 i = 0; i < MAX_NUM_LOG_SYSTEM; i++)
	{
		m_LogSystem[i] = false;
	}

	m_LogSystem[LOG_DEFAULT] = true;
	m_LogSystem[LOG_RENDER] = true;
}

Log::~Log()
{

}

bool Log::isValidSystem(LOG_SYSTEM type)
{
	if ((int32)type < 0 || (int32)type >= MAX_NUM_LOG_SYSTEM)
	{
		ERROR("Invalid LOG_SYSTEM type: %d", (int32)type);
		return false;
	}

	return true;
}

void Log::Write(LOG_SYSTEM type, const char *str, ...)
{
	if (!isValidSystem(type))
		return;

	if (m_LogSystem[type] == false)
		return;

	char buffer[1024];
	va_list args;
	va_start(args, str);
	vsprintf_s(buffer, str, args);
	
	Platform::Instance().StdOut(buffer);

	va_end(args);
}

void Log::enableSystem(LOG_SYSTEM type)
{
	if (!isValidSystem(type))
		return;

	AtomicStore(&m_LogSystem[type], true);
}

void Log::disableSystem(LOG_SYSTEM type)
{
	if (!isValidSystem(type))
		return;

	AtomicStore(&m_LogSystem[type], false);
}