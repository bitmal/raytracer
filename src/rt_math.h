#ifndef __RT_MATH_
#define __RT_MATH_

#include "stdinc.h"

typedef union vec4
{
	struct
	{
		real32 r;
		real32 g;
		real32 b;
		real32 a;
	};

	struct
	{
		real32 x;
		real32 y;
		real32 z;
		real32 w;
	};

	real32 _[4];
} v4;

extern v4
vec4_init(real32 r, real32 g, real32 b, real32 a);

extern void
vec4_add(v4 *lhs, v4 *rhs, v4 *out);

extern void
vec4_add3(v4 *lhs, v4 *rhs, v4 *out);

extern void
vec4_subtract(v4 *lhs, v4 *rhs, v4 *out);

extern void
vec4_subtract3(v4 *lhs, v4 *rhs, v4 *out);

extern void
vec4_scalar(v4 *v, real32 scalar, v4 *out);

extern void
vec4_scalar3(v4 *v, real32 scalar, v4 *out);

extern real32
vec4_magnitude(v4 *v);

extern real32
vec4_magnitude3(v4 *v);

extern real32
vec4_magnitude_squared(v4 *v);

extern real32
vec4_magnitude3_squared(v4 *v);

extern real32
vec4_dot(v4 *lhs, v4 *rhs);

extern real32
vec4_dot3(v4 *lhs, v4 *rhs);

extern void
vec4_cross3(v4 *lhs, v4 *rhs, v4 *out);

typedef union mat44
{
	struct
	{
		real32 _0[4];
		real32 _1[4];
		real32 _2[4];
		real32 _3[4];
	};

	real32 _[16];
} m44;

#endif
