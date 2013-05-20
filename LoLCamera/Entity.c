#include "Entity.h"

#define EOFF_POSX			0x68
#define EOFF_POSY			(EOFF_POSX + 0x88)
#define EOFF_HP 			0x120
#define EOFF_HPM 			0x130
#define EOFF_PLAYER_NAME 	0x2C
#define EOFF_CHAMP_STRUCT 	0x44

// Offset in champion structure
#define EOFF_CHAMP_NAME		0x24



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

	e->hp		= read_memory_as_float(e->ctxt->proc, e->entity_data + EOFF_HP);
	e->hp_max	= read_memory_as_float(e->ctxt->proc, e->entity_data + EOFF_HPM);

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
	return e->hp != 0 && e->hp_max != 0;
}

void
entity_debug (Entity *e)
{
	info("Entity DEBUG (0x%.8x) : x=%f / y=%f - hp=%f / hpm=%f", e->entity_data, e->p.v.x, e->p.v.y, e->hp, e->hp_max);
}

void
entity_free (Entity *e)
{
	if (e != NULL)
	{
		free (e);
	}
}
