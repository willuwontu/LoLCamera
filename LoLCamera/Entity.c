#include "Entity.h"

#define EOFF_POSX	0x6C
#define EOFF_POSY	0x74
#define EOFF_HP 	0x124
#define EOFF_HPM 	0x134

Entity *
entity_new (MemProc *mp, DWORD addr)
{
	Entity *e;

	if ((e = malloc(sizeof(Entity))) == NULL)
		return NULL;

	e->addr = addr;
	e->ctxt = mp;

	if (!entity_refresh(e))
	{
		entity_free(e);
		return NULL;
	}

	return e;
}

int
entity_refresh (Entity *e)
{
	if (!e)
		return 1;

	DWORD entity_data = read_memory_as_int(e->ctxt->proc, e->addr);

	if (!entity_data)
		return 0;

	vector2D_set_pos (
		&e->v,
		read_memory_as_float(e->ctxt->proc, entity_data + EOFF_POSX),
		read_memory_as_float(e->ctxt->proc, entity_data + EOFF_POSY)
	);

	e->hp		= read_memory_as_float(e->ctxt->proc, entity_data + EOFF_HP);
	e->hp_max	= read_memory_as_float(e->ctxt->proc, entity_data + EOFF_HPM);

	return (!(e->v.x == 0.0 && e->v.y == 0.0));
}

inline BOOL
entity_is_dead (Entity *e)
{
	return e->hp == 0 && e->hp_max != 0;
}

inline BOOL
entity_is_alive (Entity *e)
{
	return e->hp != 0 && e->hp_max != 0;
}

void
entity_debug (Entity *e)
{
	info("Entity DEBUG (0x%.8x) : x=%f / y=%f - hp=%f / hpm=%f", (int) e->addr, e->v.x, e->v.y, e->hp, e->hp_max);
}

void
entity_free (Entity *e)
{
	if (e != NULL)
	{
		free (e);
	}
}
