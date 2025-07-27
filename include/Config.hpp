#pragma once

typedef uint8_t  u8;
typedef int8_t   c8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint64_t u64;

#if defined(_DEBUG)
#include <assert.h>
//#define DEBUG_BREAK_IF(_CONDITION_) assert(!(_CONDITION_));
#define DEBUG_BREAK_IF(condition) if (condition) { fprintf(stderr, "Debug break: %s at %s:%d\n", #condition, __FILE__, __LINE__); std::exit(EXIT_FAILURE); }
#else
#define DEBUG_BREAK_IF(_CONDITION_)
#endif

void LogError( const char *msg, ... );
void LogInfo( const char *msg, ... );
void LogWarning( const char *msg, ... );
