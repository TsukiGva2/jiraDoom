#ifndef TSUKI_RAY_2D_STUFF
#define TSUKI_RAY_2D_STUFF

#include <raylib.h>
#include <raymath.h>

#ifndef PI
    #define PI 3.14159265358979323846f
#endif

#define screen_log(x, y, h, fmt, ...) DrawText(\
		TextFormat(fmt, __VA_ARGS__), x, y, h, PURPLE);

typedef Texture2D tex;
typedef Vector2   vec;
typedef float     scalar;
typedef Rectangle rect;
#define VEC(x,y) (vec){x,y}
#define RECT(x,y,w,h) (rect){x,y,w,h}

static inline vec vec_add(vec v1, vec v2)
{
	return Vector2Add(v1, v2);
}
static inline vec vec_sub(vec v1, vec v2)
{
	return Vector2Subtract(v1, v2);
}
static inline vec vec_div(vec v1, vec v2)
{
	return Vector2Divide(v1, v2);
}
static inline vec vec_mul(vec v1, vec v2)
{
	return Vector2Multiply(v1, v2);
}
static inline vec vec_scale(vec v, scalar c)
{
	return Vector2Scale(v, c);
}
static inline vec vec_norm(vec v)
{
	return Vector2Normalize(v);
}
static inline vec vec_rot(vec v, scalar angle)
{
	return Vector2Rotate(v, angle);
}
static inline scalar vec_dist(vec v1, vec v2)
{
	return Vector2Distance(v1, v2);
}

#endif

