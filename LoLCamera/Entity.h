// --- File		: Entity.h

#ifndef Entity_H_INCLUDED
#define Entity_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include <windows.h>

#include "../MemProc/MemProc.h"
#include "../Vector/Vector2D.h"
#include "../MemPos/MemPos.h"

// ---------- Defines -------------


// ------ Class declaration -------
typedef
struct _Entity
{
	MemPos p;
	float hp, hp_max;

	MemProc *ctxt;
	DWORD entity_data; // <-- ptr to the structure

}	Entity;



// --------- Constructors ---------

Entity *
entity_new (MemProc *mp, DWORD addr);

BOOL
entity_init (Entity *e, MemProc *mp, DWORD addr);

// ----------- Methods ------------

inline BOOL
entity_is_dead (Entity *e);

inline BOOL
entity_is_alive (Entity *e);

int
entity_refresh (Entity *e);

void
entity_debug (Entity *e);




// --------- Destructors ----------

void
entity_free (Entity *entity);






#endif // Entity_INCLUDED
