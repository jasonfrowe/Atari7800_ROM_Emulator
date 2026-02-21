#ifndef POKEY_TYPEDEFS_H
#define POKEY_TYPEDEFS_H

#include <stdint.h>

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  sint8;
typedef int16_t sint16;
typedef int32_t sint32;
typedef int64_t sint64;

// Altirra-specific macros/defines
#define VDRESTRICT
#define VDASSERT(x)
#define VDUNUSED(x) (void)(x)

#endif // POKEY_TYPEDEFS_H
