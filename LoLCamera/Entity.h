// --- File		: Entity.h

#ifndef Entity_H_INCLUDED
#define Entity_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include <windows.h>
#include "../Utils/Utils.h"

#include "../MemProc/MemProc.h"
#include "../Vector/Vector2D.h"
#include "../MemPos/MemPos.h"

// ---------- Defines -------------
#define ENTITY_TEAM_BLUE	0x64
#define ENTITY_TEAM_PURPLE	0xC8


// ------ Struct declaration -------
typedef
struct _Entity
{
	MemPos p;
	float hp, hp_max;
	float movement_speed;
	DWORD addr;

	MemProc *ctxt;
	DWORD entity_data; // <-- ptr to the structure

	char player_name[32]; // max name length = 32
	char champ_name[32];
	int team;

	bool isVisible;

}	Entity;



// --------- Constructors ---------

Entity *
entity_new (MemProc *mp, DWORD addr);

bool
entity_init (Entity *e, MemProc *mp, DWORD addr);

// ----------- Functions ------------

bool
entity_is_dead (Entity *e);

bool
entity_is_alive (Entity *e);

int
entity_is_visible (Entity *e);

int
entity_refresh (Entity *e);

void
entity_debug (Entity *e);

int
entity_ally_with (Entity *e1, Entity *e2);



// --------- Destructors ----------

void
entity_free (Entity *entity);






#endif // Entity_INCLUDED
