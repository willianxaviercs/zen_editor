#ifndef EDITOR_TYPES_H
#define EDITOR_TYPES_H

#include <stdint.h>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef s32  b32;

union v4
{
    struct
    {
        float x, y, z, w;
    };
    struct
    {
        float r, g, b, a;
    };
    float e[4];
};

internal inline v4 
create_v4(float x, float y, float z, float w)
{
    v4 result = {x, y, z, w};
    return result;
}


// convertS an hex AARRGGBB U32 to an RGBA v4 
internal v4
convert_uhex_to_v4(u32 hex)
{
    v4 result;
    
    u8 hexa = (u8)(hex >> 24);
    u8 hexr = (u8)(hex >> 16);
    u8 hexg = (u8)(hex >> 8);
    u8 hexb = (u8)(hex);
    
    result.r = (f32)hexr / 255.0f;
    result.g = (f32)hexg / 255.0f;
    result.b = (f32)hexb / 255.0f;
    result.a = (f32)hexa / 255.0f;
    
    return result;
}

#endif /* EDITOR_TYPES_H */