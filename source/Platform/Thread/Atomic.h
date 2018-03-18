#pragma once

#include "Platform/Types.h"

#include <atomic>

#ifndef atomic_int8
typedef std::atomic_schar atomic_int8;
#endif

#ifndef atomic_int16
typedef std::atomic_short atomic_int16;
#endif

#ifndef atomic_int32
typedef std::atomic_long atomic_int32;
#endif

#ifndef atomic_int64
typedef std::atomic_llong atomic_int64;
#endif

#ifndef atomic_uint8
typedef std::atomic_uchar atomic_uint8;
#endif

#ifndef atomic_uint16
typedef std::atomic_ushort atomic_uint16;
#endif

#ifndef atomic_uint32
typedef std::atomic_ulong atomic_uint32;
#endif

#ifndef atomic_uint64
typedef std::atomic_ullong atomic_uint64;
#endif

#ifndef atomic_bool
typedef std::atomic<bool> atomic_bool;
#endif


void AtomicStore(atomic_bool *obj, bool value);
void AtomicStore(atomic_int8 *obj, int8 value);
void AtomicStore(atomic_int16 *obj, int16 value);
void AtomicStore(atomic_int32 *obj, int32 value);
void AtomicStore(atomic_int64 *obj, int64 value);
void AtomicStore(atomic_uint8 *obj, int8 value);
void AtomicStore(atomic_uint16 *obj, int16 value);
void AtomicStore(atomic_uint32 *obj, int32 value);
void AtomicStore(atomic_uint64 *obj, int64 value);