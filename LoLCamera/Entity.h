// --- File		: Entity.h

#ifndef Entity_H_INCLUDED
#define Entity_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include <windows.h>

#include "../MemProc/MemProc.h"
#include "../Vector/Vector2D.h"

// ---------- Defines -------------


// ------ Class declaration -------
typedef
struct _Entity
{
	Vector2D v;
	float hp, hp_max;

	DWORD addr;
	MemProc *ctxt;

}	Entity;



// --------- Constructors ---------

Entity *
entity_new (MemProc *mp, DWORD addr);


// ----------- Methods ------------


int
entity_refresh (Entity *e);

void
entity_debug (Entity *e);




// --------- Destructors ----------

void
entity_free (Entity *entity);






#endif // Entity_INCLUDED
