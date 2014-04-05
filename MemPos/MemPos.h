// --- File		: MemPos.h

#ifndef MemPos_H_INCLUDED
#define MemPos_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include "../Win32Tools/Win32Tools.h"
#include "../MemProc/MemProc.h"
#include "../Vector/Vector2D.h"

// ---------- Defines -------------


// ------ Struct declaration -------
typedef
struct _MemPos
{
	DWORD addrX, addrY;	// Memory
	Vector2D v;			// Position

	MemProc *ctxt;		// Process context

}	MemPos;



// --------- Constructors ---------

MemPos *
mempos_new (MemProc *memproc, DWORD addrX, DWORD addrY);

MemPos *
mempos_int_new (MemProc *mp, DWORD addrX, DWORD addrY);


// ----------- Functions ------------

bool
mempos_refresh (MemPos *p);

void
mempos_init (MemPos *p, MemProc *mp, DWORD addrX, DWORD addrY);

bool
mempos_int_refresh (MemPos *p);

// --- Setters

void
mempos_set (MemPos *p, float newX, float newY);

// --- Getters

void
mempos_get (MemPos *p, float *x, float *y);



// --------- Destructors ----------

void
mempos_free (MemPos *mempos);






#endif // MemPos_INCLUDED
