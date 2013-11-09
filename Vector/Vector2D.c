#include "Vector2D.h"

// --- Constructors
Vector2D
vector2D_pos_new (float x, float y)
{
	Vector2D res = vector2D_pos(x, y);
	return res;
}

Vector2D
vector2D_new (void)
{
	return vector2D_pos_new (0.0, 0.0);
}

// --- Binary Operators
inline int
vector2D_equals (Vector2D *v1, Vector2D *v2)
{
	return (v1->x == v2->x && v1->y == v2->y);
}

inline void
vector2D_copy (Vector2D *dest, Vector2D *src)
{
	dest->x = src->x;
	dest->y = src->y;
}

inline Vector2D
vector2D_add (Vector2D *v1, Vector2D *v2)
{
	return vector2D_pos_new(v1->x + v2->x, v1->y + v2->y);
}

inline void
vector2D_sadd (Vector2D *v1, Vector2D *v2)
{
	v1->x = v1->x + v2->x;
	v1->y = v1->y + v2->y;
}

inline Vector2D
vector2D_sub (Vector2D *v1, Vector2D *v2)
{
	return vector2D_pos_new(v1->x - v2->x, v1->y - v2->y);
}

inline void
vector2D_ssub (Vector2D *v1, Vector2D *v2)
{
	v1->x = v1->x - v2->x;
	v1->y = v1->y - v2->y;
}

inline Vector2D
vector2D_mul (Vector2D *v1, Vector2D *v2)
{
	return vector2D_pos_new(v1->x * v2->x, v1->y * v2->y);
}

inline void
vector2D_smul (Vector2D *v1, Vector2D *v2)
{
	v1->x = v1->x * v2->x;
	v1->y = v1->y * v2->y;
}

inline Vector2D
vector2D_div (Vector2D *v1, Vector2D *v2)
{
	return vector2D_pos_new(v1->x / v2->x, v1->y / v2->y);
}

inline void
vector2D_sdiv (Vector2D *v1, Vector2D *v2)
{
	v1->x = v1->x / v2->x;
	v1->y = v1->y / v2->y;
}

inline int
vector2D_equal (Vector2D *v1, Vector2D *v2)
{
	return (v1->x == v2->x
		&&  v1->y == v2->y);
}

// --- Accessors
inline void
vector2D_set_pos (Vector2D *v, float x, float y)
{
	v->x = x;
	v->y = y;
}

inline void
vector2D_get_pos (Vector2D *v, float *x, float *y)
{
	*x = v->x;
	*y = v->y;
}


// --- Mathematics


inline Vector2D
vector2D_limit (Vector2D *v, float maxLen)
{
	Vector2D res = vector2D_zero();

	float vLen = vector2D_length(v);
	float cLen = (maxLen > vLen) ? vLen : maxLen;
	float angle = atan2(v->y, v->x);

	res.x = cos(angle) * cLen;
	res.y = sin(angle) * cLen;

	return res;
}

inline void
vector2D_slimit (Vector2D *v, float maxLen)
{
	float vLen = vector2D_length(v);
	float cLen = (maxLen > vLen) ? vLen : maxLen;
	float angle = atan2(v->y, v->x);

	v->x = cos(angle) * cLen;
	v->y = sin(angle) * cLen;
}

inline Vector2D
vector2D_normalize (Vector2D *v)
{
	Vector2D res = vector2D_zero();
	float magnitude = vector2D_length(v);

	if (magnitude != 0)
	{
		res.x = v->x / magnitude;
		res.y = v->y / magnitude;
	}

	return res;
}

inline void
vector2D_snormalize (Vector2D *v)
{
	float magnitude = vector2D_length(v);

	if (magnitude != 0)
	{
		v->x = v->x / magnitude;
		v->y = v->y / magnitude;
	}
}

inline Vector2D
vector2D_scalar (Vector2D *v, float k)
{
	Vector2D res =
	{
		.x = k * v->x,
		.y = k * v->y
	};

	return res;
}

void
vector2D_sscalar (Vector2D *v, float k)
{
	v->x = k * v->x;
	v->y = k * v->y;
}


int
vector2D_is_zero (Vector2D *v)
{
	return (v->x == 0 && v->y == 0);
}

void
vector2D_set_zero (Vector2D *v)
{
	vector2D_set_pos(v, 0.0, 0.0);
}

// --- Debug
void
vector2D_debug (Vector2D *v)
{
	printf("Vector Debug : %p\n"
		   "-----------------------\n"
		   "x = %f\n"
		   "y = %f\n\n",
		   (void *)v, v->x, v->y);
}

void
vector2D_sdebug (Vector2D *v, char *msg)
{
	printf("%s (%p)\n"
		   "-----------------------\n"
		   "x = %f\n"
		   "y = %f\n\n",
		   msg, (void *)v, v->x, v->y);
}

void
vector2D_mindebug (Vector2D *v)
{
	printf("{%f / %f}", v->x, v->y);
}
