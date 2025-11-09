#ifndef __types_h__
#define __types_h__

#include <stdint.h>
#include <stddef.h>

// Unsigned integer types
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

// Signed integer types
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

// Character types
typedef char      i_char;
typedef unsigned char u_char;

// Floating point types
typedef float     f32;
typedef double    f64;

// Boolean type
typedef u8        b8;
typedef u32       b32;

// Size types
typedef size_t    usize;
typedef ptrdiff_t isize;

// Pointer types
typedef void*     ptr;
typedef const void* const voidptr;

// Boolean constants
#define True  1
#define False 0

// NULL pointer
#ifndef NULL
#define NULL ((void*)0)
#endif

// Macros for enum and struct declarations
#define enum_type(name, type) typedef type name; enum name
#define struct_name(name) typedef struct name name; struct name

// Utility macros
#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max) (MIN(MAX(x, min), max))

// Kilobytes, Megabytes, Gigabytes
#define KB(x) ((x) * 1024ULL)
#define MB(x) (KB(x) * 1024ULL)
#define GB(x) (MB(x) * 1024ULL)

// Bit manipulation
#define BIT(x) (1 << (x))
#define SET_BIT(value, bit) ((value) |= BIT(bit))
#define CLEAR_BIT(value, bit) ((value) &= ~BIT(bit))
#define TOGGLE_BIT(value, bit) ((value) ^= BIT(bit))
#define CHECK_BIT(value, bit) (((value) & BIT(bit)) != 0)

// Success/failure codes
#define SUCCESS 1
#define FAILURE 0

// Common limits
#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define I8_MIN  (-128)
#define I8_MAX  127
#define I16_MIN (-32768)
#define I16_MAX 32767
#define I32_MIN (-2147483648)
#define I32_MAX 2147483647
#define I64_MIN (-9223372036854775808LL)
#define I64_MAX 9223372036854775807LL

#define F32_MIN 1.17549435e-38f
#define F32_MAX 3.40282347e+38f
#define F64_MIN 2.2250738585072014e-308
#define F64_MAX 1.7976931348623157e+308

#endif /* __types_h__ */