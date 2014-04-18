#include "Entity.h"

#define NOT_A_POINTER(addr) ((addr) < 0x10000)

#define EOFF_POSX			0x64
#define EOFF_POSY			(EOFF_POSX + 0x8)
#define EOFF_HP 			0x118
#define EOFF_HPM 			0x128
#define EOFF_MS 			0x720
#define EOFF_TEAM			0x1C
#define EOFF_PLAYER_NAME 	0x28
#define EOFF_CHAMP_STRUCT 	0x40
#define EOFF_CTXT			0x154
#define EOFF_HOVERED		0x231

// Offset in champion structure
#define EOFF_CHAMP_NAME		0x18

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

bool
entity_init (Entity *e, MemProc *mp, DWORD entity_addr)
{
	e->ctxt = mp;
	e->addr = entity_addr;
	e->entity_data = read_memory_as_int(mp->proc, entity_addr);
	e->isHovered = 0;
	e->isVisible = 0;

	if (!e->entity_data || NOT_A_POINTER(e->entity_data))
	{
		debug("Entity (%x) : data cannot be read = %x", entity_addr, e->entity_data);
		return false;
	}

	#ifdef DEBUG
		int colsize = 120;
		unsigned char buffer[colsize*16];
		read_from_memory(mp->proc, buffer, e->entity_data, sizeof(buffer));
		for (int i = 0; i < colsize; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				if (j == 0)
					debugbs("%.3X\t", j+(i*16));
				debugbs("%.2X ", buffer[j+(i*16)]);
			}

			debugbs("\t");
			for (int j = 0; j < 16; j++)
			{
				if (is_printable(buffer[j+(i*16)]))
					debugbs("%c", buffer[j+(i*16)]);
				else
					debugbs(".");
			}

			debugbs("\n");
		}
		debug("--------------------------------------------------------------------------------");
	#endif

	memset(e->player_name, 0, sizeof(e->player_name));
	memset(e->champ_name,  0, sizeof(e->champ_name));

	mempos_init(&e->p, mp, e->entity_data + EOFF_POSX, e->entity_data + EOFF_POSY);

	if (!entity_refresh(e))
		return false;

	read_from_memory(e->ctxt->proc, e->player_name, e->entity_data + EOFF_PLAYER_NAME, sizeof(e->player_name) - 1);
	DWORD champ_struct = read_memory_as_int(e->ctxt->proc, e->entity_data + EOFF_CHAMP_STRUCT);
	read_from_memory(e->ctxt->proc, e->champ_name, champ_struct + EOFF_CHAMP_NAME, sizeof(e->champ_name) - 1);

	if (!e->player_name)
		important("Player name has not been found.");
	if (!e->champ_name)
		important("Champ name has not been found.");

	entity_decode_object_name(e->ctxt, e->player_name);

	return true;
}

void
entity_decode_object_name (MemProc *mp, char *name)
{
	// "Object" name = extended object, read the pointer in the first 4 bytes
	char buffer[32];
	memcpy(buffer, &name[4], strlen(&name[4]));
	buffer[strlen("Object")] = '\0';

	if (str_equals(buffer, "Object"))
	{
		DWORD addr;
		memcpy(&addr, name, sizeof(DWORD));

		if (addr && !NOT_A_POINTER(addr))
			read_from_memory (mp->proc, name, addr, SIZEOF_PLAYERNAME - 1);
		else
			warning("Warning : Player <%s> was detected as an Object but cannot be decoded. (%x)", name, addr);
	}
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
	e->isHovered		= read_memory_as_byte (e->ctxt->proc, e->entity_data + EOFF_HOVERED);

	DWORD entityCtxt    = read_memory_as_int  (e->ctxt->proc, e->entity_data + EOFF_CTXT);
	e->isVisible        = read_memory_as_int  (e->ctxt->proc, entityCtxt     + EOFF_IS_VISIBLE);

	return (!(e->hp == 0.0 && e->hp_max == 0.0));
}



bool
entity_address_to_array (MemProc *mp, DWORD cur, DWORD end, Entity **champions)
{
	DWORD startCur = cur;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = champions[i];

		if (e == NULL)
			champions[i] = e = entity_new(mp, cur);
		else
			entity_init(e, mp, cur);

		if (e == NULL)
		{
			debug("  --> Ally %d not found", i);
			system("pause");
			return false;
		}
	}

	cur = startCur;
	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = champions[i];

		if (e != NULL)
		{
			debug("- E%d (0x%.8x) : champion=<%-16s> - player=<%-16s> \n"
					"\t p={%5.0f,%5.0f} hp={%4.0f/%4.0f} team=%6s ms=%3.0f",
				  i, e->entity_data, e->champ_name, e->player_name,
				  e->p.v.x, e->p.v.y, e->hp, e->hp_max, (e->team == ENTITY_TEAM_BLUE) ? "blue" : "purple", e->movement_speed
			);
		}
	}

	return true;
}


inline bool
entity_is_dead (Entity *e)
{
	return !entity_is_alive(e);
}

inline bool
entity_is_alive (Entity *e)
{
	if (e == NULL)
		return false;

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
		return false;

	return e->isVisible != 0;
}

void
entity_debug (Entity *e)
{
	debug("Entity DEBUG (0x%.8x) : x=%f / y=%f - hp=%f / hpm=%f - ms=%f - (%s) (%s) - %s - (%d)",
		e->entity_data, e->p.v.x, e->p.v.y, e->hp, e->hp_max, e->movement_speed, e->player_name, e->champ_name, (e->team == ENTITY_TEAM_BLUE) ? "blue" : "purple", e->isVisible);
}

void
entity_free (Entity *e)
{
	if (e != NULL)
	{
		free (e);
	}
}
