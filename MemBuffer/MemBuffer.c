#include "MemBuffer.h"

MemBuffer *
membuffer_new (DWORD addr, unsigned char *code, int size)
{
	MemBuffer *membuffer;

	if ((membuffer = malloc(sizeof(MemBuffer))) == NULL)
		return NULL;

	membuffer_init (membuffer, addr,
		buffer_new_ptr_noalloc(code, size)
	);

	return membuffer;
}

MemBuffer *
membuffer_new_from_buffer (DWORD addr, Buffer *buf)
{
	MemBuffer *membuffer;

	if ((membuffer = malloc(sizeof(MemBuffer))) == NULL)
		return NULL;

	membuffer_init (membuffer, addr, buf);

	return membuffer;
}

void
membuffer_init (MemBuffer *mb, DWORD addr, Buffer *buf)
{
	mb->addr = addr;
	mb->buffer = buf;
}

void
membuffer_free (MemBuffer *membuffer)
{
	if (membuffer != NULL)
	{
		buffer_free(membuffer->buffer);
		free (membuffer);
	}
}
