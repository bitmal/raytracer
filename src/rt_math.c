#include "rt_math.h"

#include <math.h>

extern v4
vec4_init(real32 r, real32 g, real32 b, real32 a)
{
	v4 v = {{r, g, b, a}};
	return v;
}

void
vec4_add(const v4 *lhs, const v4 *rhs, v4 *out)
{
	out->r = lhs->r + rhs->r;
	out->g = lhs->g + rhs->g;
	out->b = lhs->b + rhs->b;
	out->a = lhs->a + rhs->a;
}

void
vec4_add3(const v4 *lhs, const v4 *rhs, v4 *out)
{
	out->r = lhs->r + rhs->r;
	out->g = lhs->g + rhs->g;
	out->b = lhs->b + rhs->b;
}

void
vec4_subtract(const v4 *lhs, const v4 *rhs, v4 *out)
{
	out->r = lhs->r - rhs->r;
	out->g = lhs->g - rhs->g;
	out->b = lhs->b - rhs->b;
	out->a = lhs->a - rhs->a;
}

void
vec4_subtract3(const v4 *lhs, const v4 *rhs, v4 *out)
{
	out->r = lhs->r - rhs->r;
	out->g = lhs->g - rhs->g;
	out->b = lhs->b - rhs->b;
}

void
vec4_scalar(const v4 *v, real32 scalar, v4 *out)
{
	out->r = v->r*scalar;
	out->g = v->g*scalar;
	out->b = v->b*scalar;
	out->a = v->a*scalar;
}

void
vec4_scalar3(const v4 *v, real32 scalar, v4 *out)
{
	out->r = v->r*scalar;
	out->g = v->g*scalar;
	out->b = v->b*scalar;
}

void
vec4_normal(const v4 *v, v4 *out)
{
	real32 mag = vec4_magnitude3(v);

	out->x = v->x/mag;
	out->y = v->y/mag;
	out->z = v->z/mag;
}

void
vec4_direction(const v4 *lhs, const v4 *rhs, v4 *out)
{
	out->x = rhs->x - lhs->x;
	out->y = rhs->y - lhs->y;
	out->z = rhs->z - lhs->z;

	vec4_normal(out, out);
}

real32
vec4_magnitude(const v4 *v)
{
	return sqrtf((v->r*v->r) + (v->g*v->g) + (v->b*v->b) + (v->a*v->a));
}

real32
vec4_magnitude3(const v4 *v)
{
	return sqrtf((v->r*v->r) + (v->g*v->g) + (v->b*v->b));
}

real32
vec4_magnitude_squared(const v4 *v)
{
	return (v->r*v->r) + (v->g*v->g) + (v->b*v->b) + (v->a*v->a);
}

real32
vec4_magnitude3_squared(const v4 *v)
{
	return (v->r*v->r) + (v->g*v->g) + (v->b*v->b);
}

real32
vec4_dot(const v4 *lhs, const v4 *rhs)
{
	return lhs->r*rhs->r + lhs->g*rhs->g + lhs->b*rhs->b + lhs->a*rhs->a;
}

real32
vec4_dot3(const v4 *lhs, const v4 *rhs)
{
	return lhs->r*rhs->r + lhs->g*rhs->g + lhs->b*rhs->b;
}

void
vec4_cross3(const v4 *lhs, const v4 *rhs, v4 *out)
{
	out->x = lhs->y*rhs->z - lhs->z*rhs->y;
	out->y = lhs->z*rhs->x - lhs->x*rhs->z;
	out->z = lhs->x*rhs->y - lhs->y*rhs->x;
}

real32
vec4_distance3(const v4 *lhs, const v4 *rhs)
{
	real32 x = rhs->x - lhs->x;
	real32 y = rhs->y - lhs->y;
	real32 z = rhs->z - lhs->z;

	return sqrtf(x*x + y*y + z*z);
}
