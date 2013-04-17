#include "Scanner.h"

BbQueue *memscan_search (MemProc *mp, char *desc, unsigned char *pattern, unsigned char *search_mask, unsigned char *res_mask)
{
	memproc_search(mp, pattern, search_mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(mp);
	char *str;
	DWORD ptr;
	int len;

	if (bb_queue_get_length(results) > 1)
	{
		infob("%s : (%d) occurences found : ", __FUNCTION__, bb_queue_get_length(results));
		foreach_bbqueue_item (results, MemBlock *block)
			printf("0x%.8x ", (int) block->addr);
		printf("\n");
	}

	if (bb_queue_get_length(results) == 0)
	{
		error("\%s\" : Nothing found", desc);
		str_debug_len(pattern, strlen(search_mask));
		printf("search_mask = %s\n", search_mask);
		return NULL;
	}

	MemBlock *block = bb_queue_pick_first(results);
	str = block->data;
	ptr = block->addr;

	bb_queue_free_all(results, memblock_free);

	if (res_mask == NULL)
		res_mask = search_mask;

	BbQueue *res = scan_search(str, res_mask);

	info("\"%s\" scanning : found at 0x%.8x :", desc, ptr);
	str = desc;

	foreach_bbqueue_item (res, Buffer *b)
	{
		len = 0;
		memcpy(&ptr, b->data, sizeof(DWORD));

		while (*str != '/' && *str != '\0')
		{
			str++;
			len++;
		}

		info("%.*s -> 0x%.8x", len, str-len, ptr);
		str++;
	}

	return res;
}

BbQueue *scan_search (unsigned char *pattern, unsigned char *mask)
{
	int len = strlen(mask);
	BbQueue *res = bb_queue_new();
	Ztring *z = ztring_new();

	for (int i = 0; i < len; i++)
	{
		if (mask[i] == '?')
			ztring_concat_letter(z, pattern[i]);

		else if (ztring_get_len(z) != 0)
		{
			bb_queue_add(res, z);
			z = ztring_new();
		}
	}

	if (ztring_get_len(z) != 0)
		bb_queue_add(res, z);

	BbQueue *res_buffer = bb_queue_new();

	while (bb_queue_get_length(res))
	{
		z = bb_queue_get_first(res);

		Buffer *b = buffer_new_ptr_noalloc(ztring_get_text(z), ztring_get_len(z));
		bb_queue_add (res_buffer, b);

		ztring_free(z);
	}

	return res_buffer;
}
