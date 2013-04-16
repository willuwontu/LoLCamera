#include "MemChunk.h"

MemChunk *
memchunk_new (HANDLE hProc, MEMORY_BASIC_INFORMATION *meminfo)
{
	MemChunk *mem;

	if ((mem = malloc(sizeof(MemChunk))) == NULL)
		return NULL;

	mem->proc = hProc;

	mem->addr = (DWORD) meminfo->BaseAddress;
	mem->size = meminfo->RegionSize;

	mem->buffer =  malloc (meminfo->RegionSize + 10);	// Unknown size allocation error, +10 fix it
	memset(mem->buffer, 0, meminfo->RegionSize + 10);	// FIXME
	/* Error #1: UNADDRESSABLE ACCESS: reading 0x03407032-0x03407033 1 byte(s)
	# 0 compare_pattern                     [C:/Users/Spl3en/Documents/GitHub/LoLCamera/Win32Tools/Win32Tools.c:327]
	# 1 find_pattern                        [C:/Users/Spl3en/Documents/GitHub/LoLCamera/Win32Tools/Win32Tools.c:341]
	# 2 memproc_search                      [C:/Users/Spl3en/Documents/GitHub/LoLCamera/MemProc/MemProc.c:206]
	# 3 memscan_search                      [C:/Users/Spl3en/Documents/GitHub/LoLCamera/Scanner/Scanner.c:32]
	# 4 camera_scan_champions               [C:/Users/Spl3en/Documents/GitHub/LoLCamera/LoLCamera/LoLCameraMem.c:176]
	# 5 camera_init                         [C:/Users/Spl3en/Documents/GitHub/LoLCamera/LoLCamera/LoLCamera.c:103]
	# 6 main                                [C:/Users/Spl3en/Documents/GitHub/LoLCamera/main.c:33]
	Note: @0:00:03.449 in thread 4492
	Note: instruction: mov    (%eax) -> %dl
	*/

	mem->matches = bb_queue_new();

	memchunk_read_from_memory(mem);

	return mem;
}

inline void
memchunk_read_from_memory (MemChunk *mem)
{
	read_from_memory(mem->proc, mem->buffer, mem->addr, mem->size);
}

void
memchunk_debug (MemChunk *mc)
{
	printf("Addr : 0x%.8x - 0x%.8x (%d bytes) \n", (int) mc->addr, (int) mc->addr + mc->size, mc->size);
}

void
memchunk_full_debug (MemChunk *mc)
{
	memchunk_debug(mc);

	for (unsigned int i = 0; i < mc->size; i++)
		printf("0x%.2x ", mc->buffer[i]);

	printf("\n");
}


void
memchunk_free (MemChunk *memchunk)
{
	if (memchunk != NULL)
	{
		free (memchunk);
	}
}
