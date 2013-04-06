#include "MemPos.h"

MemPos *
mempos_new (MemProc *mp, DWORD addrX, DWORD addrY)
{
	MemPos *p;

	if ((p = malloc(sizeof(MemPos))) == NULL)
		return NULL;

	p->addrX = addrX + mp->base_addr;
	p->addrY = addrY + mp->base_addr;

	vector2D_set_pos (
		&p->v,
		read_memory_as_float(mp->proc, p->addrX),
		read_memory_as_float(mp->proc, p->addrY)
	);

	p->ctxt = mp;

	return p;
}

int
mempos_refresh (MemPos *p)
{
	vector2D_set_pos (
		&p->v,
		read_memory_as_float(p->ctxt->proc, p->addrX),
		read_memory_as_float(p->ctxt->proc, p->addrY)
	);

	return (!(p->v.x == 0.0 && p->v.y == 0.0));
}

void
mempos_set (MemPos *p, float newX, float newY)
{
	vector2D_set_pos (&p->v, newX, newY);

	write_memory_as_float(p->ctxt->proc, p->addrX, newX);
	write_memory_as_float(p->ctxt->proc, p->addrY, newY);
}

void
mempos_free (MemPos *mempos)
{
	if (mempos != NULL)
	{
		free (mempos);
	}
}
