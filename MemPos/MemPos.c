#include "MemPos.h"

MemPos *
mempos_new (MemProc *mp, DWORD addrX, DWORD addrY)
{
	MemPos *p;

	if ((p = malloc(sizeof(MemPos))) == NULL)
		return NULL;

	p->addrX = addrX + mp->base_addr;
	p->addrY = addrY + mp->base_addr;
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

	p->addrX = addrX + mp->base_addr;
	p->addrY = addrY + mp->base_addr;
	p->ctxt = mp;

	mempos_int_refresh(p);

	return p;
}

bool
mempos_init (MemPos *p, MemProc *mp, DWORD addrX, DWORD addrY)
{
	p->ctxt = mp;
	p->addrX = addrX + mp->base_addr;
	p->addrY = addrY + mp->base_addr;

	return mempos_refresh(p);
}

bool
mempos_refresh (MemPos *p)
{
	p->v.x = -1.1337;
	p->v.y = -1.1337;

	vector2D_set_pos (
		&p->v,
		read_memory_as_float(p->ctxt->proc, p->addrX),
		read_memory_as_float(p->ctxt->proc, p->addrY)
	);

	return (!(p->v.x == -1.1337 && p->v.y == -1.1337));
}

bool
mempos_int_refresh (MemPos *p)
{
	p->v.x = -1337;
	p->v.y = -1337;

	vector2D_set_pos (
		&p->v,
		read_memory_as_int(p->ctxt->proc, p->addrX),
		read_memory_as_int(p->ctxt->proc, p->addrY)
	);

	return (!(p->v.x == -1337 && p->v.y == -1337));
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
