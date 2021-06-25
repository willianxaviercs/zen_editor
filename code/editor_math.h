
/* Zen Editor Math Library - Single Header File
*  June 25 - 08:11 am
 */

#ifndef EDITOR_MATH_H
#define EDITOR_MATH_H

#define KiloBytes(n) (         (n) * 1024LL)
#define MegaBytes(n) (KiloBytes(n) * 1024LL)
#define GigaBytes(n) (MegaBytes(n) * 1024LL)
#define TeraBytes(n) (GigaBytes(n) * 1024LL)

#define Max(A, B) (((A) > (B)) ? (A) : (B))
#define Min(A, B) (((A) < (B)) ? (A) : (B))

#include <math.h>

internal inline u32 editor_u32_clamp(u32 min, u32 x, u32 max);

internal inline u32 round_f32_to_u32(f32 n);

internal inline u32
editor_u32_clamp(u32 min, u32 x, u32 max)
{
    if (x < min) return min;
    else if (x > max) return max;
    return x;
}

internal inline u32
round_f32_to_u32(f32 n)
{
    u32 result = (u32)((n) + 0.5f);
    return result;
}


#endif /* EDITOR_MATH_H */
