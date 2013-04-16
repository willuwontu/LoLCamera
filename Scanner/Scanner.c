#include "Scanner.h"

/*
*	Example:

	//	00A36FD1    57              push edi
	//	00A36FD2  ▼ 0F84 FA000000   je League_Of_Legends.00A370D2
	//	00A36FD8    8B0D C0F3D802   mov ecx, [dword ds:League_Of_Legends.2D8F3C0]  <-- EntitiesArrayEnd
	//	00A36FDE    8B2D BCF3D802   mov ebp, [dword ds:League_Of_Legends.2D8F3BC]  <-- EntitiesArrayStart
	//	00A36FE4    3BE9            cmp ebp, ecx

	memscan_search (
		this->mp,
		(unsigned char []) {
			0x57,
			0x0F,0x84,0xFA,0x00,0x00,0x00,
			0x8B,0x0D,0xC0,0xF3,0xD8,0x02,
			0x8B,0x2D,0xBC,0xF3,0xD8,0x02,
			0x3B,0xE9
		},	"x"
			"xxxxxx"
			"xx????"
			"xx????"
			"xx",
			NULL
	);
*
**/

BbQueue *memscan_search (MemProc *mp, DWORD *addr, unsigned char *pattern, unsigned char *search_mask, unsigned char *res_mask)
{
	memproc_search(mp, pattern, search_mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(mp);
	char *str;

	if (bb_queue_get_length(results) > 1)
	{
		infob("%s : (%d) occurences found : ", __FUNCTION__, bb_queue_get_length(results));
		foreach_bbqueue_item (results, MemBlock *block)
			printf("0x%.8x ", (int) block->addr);
		printf("\n");
	}

	if (bb_queue_get_length(results) == 0)
	{
		error("(%x) Nothing found", *addr);
		str_debug_len(pattern, strlen(search_mask));
		printf("search_mask = %s\n", search_mask);
		return NULL;
	}

	MemBlock *block = bb_queue_pick_first(results);
	str = block->data;
	*addr = block->addr;

	bb_queue_free_all(results, memblock_free);

	if (res_mask == NULL)
		res_mask = search_mask;

	return scan_search(str, res_mask);
}

BbQueue *scan_search (unsigned char *pattern, unsigned char *mask)
{
	int len = strlen(mask);
	BbQueue *res = bb_queue_new();
	Ztring *z = ztring_new();

	for (int i = 0; i < len; i++)
	{
		if (mask[i] != '?')
			ztring_concat_letter(z, pattern[i]);

		else if (ztring_get_len(z) != 0)
		{
			bb_queue_add(res, z);
			z = ztring_new();
		}
	}

	if (ztring_get_len(z) != 0)
		bb_queue_add(res, z);

	return res;
}
