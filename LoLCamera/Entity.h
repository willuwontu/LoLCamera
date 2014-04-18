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


#define SIZEOF_PLAYERNAME 32
#define SIZEOF_CHAMPNAME 32


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

	char player_name[SIZEOF_PLAYERNAME]; // max name length = 16
	char champ_name[SIZEOF_CHAMPNAME];
	int team;

	bool isVisible;
	bool isHovered;

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

bool
entity_address_to_array (MemProc *mp, DWORD entities_addr_start, DWORD entities_addr_end, Entity **champions);

void
entity_decode_object_name (MemProc *mp, char *name);


// --------- Destructors ----------

void
entity_free (Entity *entity);






#endif // Entity_INCLUDED
