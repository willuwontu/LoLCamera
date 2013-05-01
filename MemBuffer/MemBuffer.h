// --- File		: MemBuffer.h

#ifndef MemBuffer_H_INCLUDED
#define MemBuffer_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>

// ---------- Defines -------------
#include "../Ztring/Ztring.h"
#include "../Win32Tools/Win32Tools.h"

// ------ Class declaration -------
typedef
struct _MemBuffer
{
	DWORD addr;
	Buffer *buffer;

}	MemBuffer;



// --------- Constructors ---------

MemBuffer *
membuffer_new (DWORD addr, unsigned char *code, int size);

void
membuffer_init (MemBuffer *mb, DWORD addr, Buffer *buf);

// ----------- Methods ------------




// --------- Destructors ----------

void
membuffer_free (MemBuffer *membuffer);






#endif // MemBuffer_INCLUDED
