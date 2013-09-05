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
	float movement_speed;
	DWORD addr;

	MemProc *ctxt;
	DWORD entity_data; // <-- ptr to the structure

	char player_name[17]; // max name length = 16
	char champ_name[17];

}	Entity;



// --------- Constructors ---------

Entity *
entity_new (MemProc *mp, DWORD addr);

BOOL
entity_init (Entity *e, MemProc *mp, DWORD addr);

// ----------- Methods ------------

BOOL
entity_is_dead (Entity *e);

BOOL
entity_is_alive (Entity *e);

int
entity_refresh (Entity *e);

void
entity_debug (Entity *e);




// --------- Destructors ----------

void
entity_free (Entity *entity);






#endif // Entity_INCLUDED
