#include "Entity.h"

#define EOFF_POSX			0x64
#define EOFF_POSY			(EOFF_POSX + 0x8)
#define EOFF_HP 			0x118
#define EOFF_HPM 			0x128
#define EOFF_MS 			0x720
#define EOFF_TEAM			0x1C
#define EOFF_PLAYER_NAME 	0x28
#define EOFF_CHAMP_STRUCT 	0x40
#define EOFF_CTXT			0x154

// Offset in champion structure
#define EOFF_CHAMP_NAME		0x1C

// Offset in champion context
#define EOFF_IS_VISIBLE		0x24


Entity *
entity_new (MemProc *mp, DWORD addr)
{
	Entity *e;

	if ((e = calloc(sizeof(Entity), 1)) == NULL)
		return NULL;

	if (!entity_init(e, mp, addr))
	{
		entity_free(e);
		return NULL;
	}

	return e;
}

BOOL
entity_init (Entity *e, MemProc *mp, DWORD addr)
{
	e->ctxt = mp;
	e->addr = addr;
	e->entity_data = read_memory_as_int(mp->proc, addr);

	memset(e->player_name, 0, sizeof(e->player_name));
	memset(e->champ_name,  0, sizeof(e->champ_name));

	mempos_init(&e->p, mp, e->entity_data + EOFF_POSX - mp->base_addr, e->entity_data + EOFF_POSY - mp->base_addr);

	if (!entity_refresh(e))
		return FALSE;

	read_from_memory(e->ctxt->proc, e->player_name, e->entity_data + EOFF_PLAYER_NAME, sizeof(e->player_name) - 1);
	DWORD champ_struct = read_memory_as_int(e->ctxt->proc, e->entity_data + EOFF_CHAMP_STRUCT);
	read_from_memory(e->ctxt->proc, e->champ_name, champ_struct + EOFF_CHAMP_NAME, sizeof(e->champ_name) - 1);

	return TRUE;
}

int
entity_refresh (Entity *e)
{
	if (!e)
		return 0;

	if (!e->entity_data)
		return 0;

	if (!mempos_refresh(&e->p))
		return 0;

	e->hp				= read_memory_as_float(e->ctxt->proc, e->entity_data + EOFF_HP);
	e->hp_max			= read_memory_as_float(e->ctxt->proc, e->entity_data + EOFF_HPM);
	e->movement_speed	= read_memory_as_float(e->ctxt->proc, e->entity_data + EOFF_MS);
	e->team				= read_memory_as_int  (e->ctxt->proc, e->entity_data + EOFF_TEAM);

	DWORD entityCtxt    = read_memory_as_int  (e->ctxt->proc, e->entity_data + EOFF_CTXT);
	e->isVisible        = read_memory_as_int  (e->ctxt->proc, entityCtxt     + EOFF_IS_VISIBLE);

	return (!(e->hp == 0.0 && e->hp_max == 0.0));
}

inline BOOL
entity_is_dead (Entity *e)
{
	return !entity_is_alive(e);
}

inline BOOL
entity_is_alive (Entity *e)
{
	if (e == NULL)
		return FALSE;

	return e->hp != 0 && e->hp_max != 0;
}

int
entity_ally_with (Entity *e1, Entity *e2)
{
    if (!e1 || !e2)
        return 0;

    return e1->team == e2->team;
}

int
entity_is_visible (Entity *e)
{
	if (e == NULL)
		return FALSE;

	return e->isVisible != 0;
}

void
entity_debug (Entity *e)
{
	debug("Entity DEBUG (0x%.8x) : x=%f / y=%f - hp=%f / hpm=%f - (%s) (%s) - %s - (%d)",
		e->entity_data, e->p.v.x, e->p.v.y, e->hp, e->hp_max, e->player_name, e->champ_name, (e->team == ENTITY_TEAM_BLUE) ? "blue" : "purple", e->isVisible);
}

void
entity_free (Entity *e)
{
	if (e != NULL)
	{
		free (e);
	}
}
