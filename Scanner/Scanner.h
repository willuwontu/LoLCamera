// --- Author	: Moreau Cyril - Spl3en
// --- File		: Scanner.h
// --- Date		: 2013-04-15-12.05.11
// --- Version	: 1.0

#ifndef Scanner_H_INCLUDED
#define Scanner_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include "../MemProc/MemProc.h"

// ---------- Defines -------------


// ------ Class declaration -------



// --------- Constructors ---------


// ----------- Methods ------------


BbQueue *memscan_search (MemProc *mp, DWORD *addr, unsigned char *pattern, unsigned char *search_mask, unsigned char *res_mask);
BbQueue *scan_search (unsigned char *pattern, unsigned char *mask);



// --------- Destructors ----------





#endif // Scanner_INCLUDED
