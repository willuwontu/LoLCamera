// --- Author	: Moreau Cyril - Spl3en
// --- File		: Vector2D.h
// --- Date		: 2011-11-19-05.29.13
// --- Version	: 1.0

/*
*	Vector2D is a bidimensionnal vector implementation
*/

#ifndef Vector2D_H_INCLUDED
#define Vector2D_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ---------- Defines -------------
/*  Initilisation at declaration only  */
#define vector2D_zero()					\
{										\
	.x = 0.0,							\
	.y = 0.0							\
}										\

#define vector2D_cp(v)					\
{										\
	.x = v.x,							\
	.y = v.y							\
}										\

#define vector2D_pos(posx, posy)		\
{										\
	.x = (posx),						\
	.y = (posy)							\
}										\

/* 			   Mathematics 				*/
#define vector2D_length_square(pV) 		\
(										\
		pow((pV)->x, 2) 				\
	  + pow((pV)->y, 2)					\
)										\

#define vector2D_length(pV) 			\
(										\
	sqrt (								\
		pow((pV)->x, 2) 				\
	  + pow((pV)->y, 2)					\
	)									\
)										\

#define vector2D_distance_between(pV1, pV2) \
(											\
	abs(									\
		vector2D_length((pV1))				\
	  - vector2D_length((pV2))				\
	)										\
)											\

#define vector2D_distance(pV1, pV2)             \
    sqrt(                                       \
        ((float)pow((pV1)->x - (pV2)->x, 2)		\
      + ((float)pow((pV1)->y - (pV2)->y, 2)))	\
    )                                           \



// ------ Class declaration -------
typedef struct Vector2D Vector2D;

struct Vector2D
{
	// public:
	float x, y;
};


// --------- Constructors ---------

Vector2D
vector2D_new (void);

Vector2D
vector2D_pos_new (float x, float y);


// ----------- Methods ------------

Vector2D
vector2D_normalize 	(Vector2D *v);

void
vector2D_snormalize (Vector2D *v);

Vector2D
vector2D_scalar 	(Vector2D *v, float k);

void
vector2D_sscalar 	(Vector2D *v, float k);

Vector2D
vector2D_limit 		(Vector2D *v, float limit);

void
vector2D_slimit 	(Vector2D *v, float maxLen);

int
vector2D_is_zero 	(Vector2D *v);

void
vector2D_set_zero 	(Vector2D *v);

// --- Operators
void
vector2D_copy 		(Vector2D *dest, Vector2D *src);

int
vector2D_equals		(Vector2D *v1, Vector2D *v2);

Vector2D
vector2D_add 		(Vector2D *v1, Vector2D *v2);

void
vector2D_sadd 		(Vector2D *v1, Vector2D *v2);

Vector2D
vector2D_sub 		(Vector2D *v1, Vector2D *v2);

void
vector2D_ssub 		(Vector2D *v1, Vector2D *v2);

Vector2D
vector2D_mul 		(Vector2D *v1, Vector2D *v2);

void
vector2D_smul 		(Vector2D *v1, Vector2D *v2);

Vector2D
vector2D_div 		(Vector2D *v1, Vector2D *v2);

void
vector2D_sdiv 		(Vector2D *v1, Vector2D *v2);

int
vector2D_equal 		(Vector2D *v1, Vector2D *v2);

// --- Accessors

void
vector2D_get_pos 	(Vector2D *v, float *x, float *y);

void
vector2D_set_pos 	(Vector2D *v, float x, float y);

// --- Debug

void
vector2D_sdebug 	(Vector2D *v, char *msg);

void
vector2D_debug 		(Vector2D *v);

void
vector2D_mindebug 	(Vector2D *v);



#endif // Vector2D_INCLUDED
