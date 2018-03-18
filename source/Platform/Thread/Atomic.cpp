#pragma once

#include "Platform/Types.h"
#include "Platform/Thread/Atomic.h"

void AtomicStore(atomic_bool *obj, bool value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_int8 *obj, int8 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_int16 *obj, int16 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_int32 *obj, int32 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_int64 *obj, int64 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_uint8 *obj, uint8 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_uint16 *obj, uint16 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_uint32 *obj, uint32 value)
{
	std::atomic_store(obj, value);
}

void AtomicStore(atomic_uint64 *obj, uint64 value)
{
	std::atomic_store(obj, value);
}
