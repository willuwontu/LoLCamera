#include "MemPos.h"

MemPos *
mempos_new (MemProc *mp, DWORD addrX, DWORD addrY)
{
	MemPos *p;

	if ((p = malloc(sizeof(MemPos))) == NULL)
		return NULL;

	p->addrX = addrX;
	p->addrY = addrY;
	p->ctxt = mp;

	if (!mempos_refresh(p))
	{
		mempos_free(p);
		return NULL;
	}

	return p;
}

MemPos *
mempos_int_new (MemProc *mp, DWORD addrX, DWORD addrY)
{
	MemPos *p;

	if ((p = malloc(sizeof(MemPos))) == NULL)
		return NULL;

	p->addrX = addrX;
	p->addrY = addrY;
	p->ctxt = mp;

	mempos_int_refresh(p);

	return p;
}

bool
mempos_init (MemPos *p, MemProc *mp, DWORD addrX, DWORD addrY)
{
	p->ctxt = mp;
	p->addrX = addrX;
	p->addrY = addrY;

	return mempos_refresh(p);
}

bool
mempos_refresh (MemPos *p)
{
	float x = read_memory_as_float(p->ctxt->proc, p->addrX);
	float y = read_memory_as_float(p->ctxt->proc, p->addrY);

	if (x == -1337.1337 && y == -1337.1337)
		return false;

	vector2D_set_pos (&p->v, x, y);

	return true;
}

bool
mempos_int_refresh (MemPos *p)
{
	int x = read_memory_as_int(p->ctxt->proc, p->addrX);
	int y = read_memory_as_int(p->ctxt->proc, p->addrY);

	if (x == -13371337 && y == -13371337)
		return false;

	vector2D_set_pos (&p->v, x, y);

	return true;
}

void
mempos_set (MemPos *p, float newX, float newY)
{
	vector2D_set_pos (&p->v, newX, newY);

	write_memory_as_float(p->ctxt->proc, p->addrX, newX);
	write_memory_as_float(p->ctxt->proc, p->addrY, newY);
}

void
mempos_get (MemPos *p, float *x, float *y)
{
	vector2D_get_pos(&p->v, x, y);
}

void
mempos_free (MemPos *mempos)
{
	if (mempos != NULL)
	{
		free (mempos);
	}
}
