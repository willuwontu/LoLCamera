#include "LoLCamera.h"

static BbQueue * camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size);
static void      camera_search_signature  (unsigned char *pattern, DWORD *addr, char *mask, char *name);
static Patch *   camera_get_patch         (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);
static void      camera_get_patches       (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);

void camera_scan_patch ()
{
	Camera *this = camera_get_instance();

	// Search for camera positionning instructions
	info("------------------------------------------------------------------");
	info("Looking for patch addresses ...");

	this->border_screen = camera_get_patch (

		 this->mp, "Move the camera when the mouse is near the screen border",
		&this->border_screen_addr,

		 /* 00A37E2A  ║·  F30F1115 3C71DF03		 movss [dword ds:League_of_Legends.CameraX], xmm2	; float 450.0000  <-- CameraX
			00A37E32  ║·  F30F110D 4071DF03		 movss [dword ds:League_of_Legends.3DF7140], xmm1	; float 0.0
			00A37E3A  ║·  F30F1105 4471DF03		 movss [dword ds:League_of_Legends.CameraY], xmm0	; float 3897.000  <-- CameraY
		*/
		(unsigned char []) {
			0xF3,0x0F,0x11,0x15,0x3C,0x71,0xDF,0x03, 	// xxxx????
			0xF3,0x0F,0x11,0x0D,0x40,0x71,0xDF,0x03, 	// xxxx????
			0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03  	// xxxx????
		},  "xxxx????"
			"xxxx????"
			"xxxx????",

		(unsigned char []) {
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
		}, 	"xxxxxxxx"
			"xxxxxxxx"
			"xxxxxxxx"
	);

	this->respawn_reset = camera_get_patch (

		this->mp, "Center the camera on the champion when you respawn",
	   &this->respawn_reset_addr,

		/*	00A09334  ║·  F30F1105 3C71DF03   movss [dword ds:League_of_Legends.CameraX], xmm0      ; float 918.4202
			00A0933C  ║·  F30F1043 04         movss xmm0, [dword ds:ebx+4]
			00A09341  ║·  F30F1105 4071DF03   movss [dword ds:League_of_Legends.3DF7140], xmm0      ; float 0.0
			00A09349  ║·  F30F1043 08         movss xmm0, [dword ds:ebx+8]
			00A0934E  ║·  F30F1105 4471DF03   movss [dword ds:League_of_Legends.CameraY], xmm0      ; float 1154.925
		*/
		(unsigned char []) {
			0xF3,0x0F,0x11,0x05,0x3C,0x71,0xDF,0x03,	// xxxx????
			0xF3,0x0F,0x10,0x43,0x04,				 	// xxxxx
			0xF3,0x0F,0x11,0x05,0x40,0x71,0xDF,0x03,	// xxxx????
			0xF3,0x0F,0x10,0x43,0x08,					// xxxxx
			0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03		// xxxx????
		},	"xxxx????"
			"xxxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????",

		(unsigned char []) {
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
			0x90,0x90,0x90,0x90,0x90,					// xxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
			0x90,0x90,0x90,0x90,0x90,				 	// xxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
		},	"xxxxxxxx"
			"xxxxx"
			"xxxxxxxx"
			"xxxxx"
			"xxxxxxxx"
	);

	this->locked_camera = camera_get_patch (

		this->mp, "Center the camera on the champion when you are in locked camera mode",
	   &this->locked_camera_addr,

		/*	00A37AAC  ║·▼ 74 39                jz short League_of_Legends.00A37AE7
			00A37AAE  ║·  F30F1040 6C          movss xmm0, [dword ds:eax+6C]                                 ; float 0.0
			00A37AB3  ║·  F30F1105 3C71DF03    movss [dword ds:League_of_Legends.CameraX], xmm0              ; float 0.0, 0.0, 0.0, 0.0
			00A37ABB  ║·  F30F1040 70          movss xmm0, [dword ds:eax+70]                                 ; float 0.0, 0.0, 0.0, 0.0
			00A37AC0  ║·  F30F1105 4071DF03    movss [dword ds:League_of_Legends.3DF7140], xmm0              ; float 0.0, 0.0, 0.0, 0.0
			00A37AC8  ║·  F30F1040 74          movss xmm0, [dword ds:eax+74]                                 ; float 0.0, 0.0, 0.0, 0.0
			00A37ACD  ║·  F30F1105 4471DF03    movss [dword ds:League_of_Legends.CameraY], xmm0              ; float 0.0, 0.0, 0.0, 0.0
		*/
		(unsigned char []) {
			0x74,0x39,									// xx
			0xF3,0x0F,0x10,0x40,0x6C,					// xxxxx
			0xF3,0x0F,0x11,0x05,0x3C,0x71,0xDF,0x03,	// xxxx????
			0xF3,0x0F,0x10,0x40,0x70,					// xxxxx
			0xF3,0x0F,0x11,0x05,0x40,0x71,0xDF,0x03,	// xxxx????
			0xF3,0x0F,0x10,0x40,0x74,					// xxxxx
			0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03		// xxxx????
		},	"xx"
			"xxxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????",

		(unsigned char []) {
			0x90,0x90,								    // xx
			0x90,0x90,0x90,0x90,0x90,					// xxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
			0x90,0x90,0x90,0x90,0x90,					// xxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
			0x90,0x90,0x90,0x90,0x90,				 	// xxxxx
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, 	// xxxxxxxx
		},	"??"
			"?????"
			"xxxxxxxx"
			"xxxxx"
			"xxxxxxxx"
			"xxxxx"
			"xxxxxxxx"
	);

	camera_get_patches (this->F2345_pressed, 2,
		this->mp, "Center the camera on the ally X when FX is pressed",
		(DWORD *[2]) {
			&this->allies_cam_addr[0],
			&this->allies_cam_addr[1]
		},

		/*	00A370A7  ║·  80BB 2D030000 00    cmp [byte ds:ebx+32D], 0                 ; Case 2 of cascaded IF League_of_Legends.0A3705D
			00A370AE  ║·▼ 74 22               je short League_of_Legends.00A370D2
			00A370B0  ║·  D946 6C             fld [dword ds:esi+6C]                    ; pushf *(esi+6C) (esi=381988C0)		<--- start NOPing here
			00A370B3  ║·  D95B 14             fstp [dword ds:ebx+14]                   ; (cameraX = *(ebx+14)) = popf()
			00A370B6  ║·  D946 74             fld [dword ds:esi+74]
			00A370B9  ║·  D95B 1C             fstp [dword ds:ebx+1C]
		*/
		(unsigned char []) {
			0xD9, 0x46, 0x6C, // xxx
			0xD9, 0x5B, 0x14, // xxx
			0xD9, 0x46, 0x74, // xxx
			0xD9, 0x5B, 0x1C, // xxx
		},	"xxx"
			"xxx"
			"xxx"
			"xxx",

		(unsigned char []) {
			0x90,0x90,0x90, 	// xxx
			0x90,0x90,0x90, 	// xxx
			0x90,0x90,0x90, 	// xxx
			0x90,0x90,0x90, 	// xxx
		},	"xxx"
			"xxx"
			"xxx"
			"xxx"
	);
}

void camera_scan_campos ()
{
	Camera *this = camera_get_instance();
	char *description = "CameraX/CameraY";

	BbQueue *res = memscan_search(this->mp, description,
		(unsigned char []) {
		 /* 00A37E2A  ║·  F30F1115 <<3C71DF03>>	movss [dword ds:League_of_Legends.CameraX], xmm2	; float 450.0000  <-- CameraX
			00A37E32  ║·  F30F110D 4071DF03		movss [dword ds:League_of_Legends.3DF7140], xmm1	; float 0.0
			00A37E3A  ║·  F30F1105 <<4471DF03>>	movss [dword ds:League_of_Legends.CameraY], xmm0	; float 3897.000  <-- CameraY
		*/
			0xF3,0x0F,0x11,0x15,0x3C,0x71,0xDF,0x03,
			0xF3,0x0F,0x11,0x0D,0x40,0x71,0xDF,0x03,
			0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03
		},	"xxxx????"
			"xxxx????"
			"xxxx????",

			"xxxx????"
			"xxxxxxxx"
			"xxxx????"
	);

	if (!res)
	{
		warning("Cannot find %s address\nUsing the .ini value : 0x%.8x\n", description, this->entities_addr);
		return;
	}

	Buffer *cameraX = bb_queue_pick_first(res);
	Buffer *cameraY = bb_queue_pick_last(res);

	DWORD camx_addr_ptr, camy_addr_ptr;
	memcpy(&camx_addr_ptr, cameraX->data, sizeof(DWORD));
	memcpy(&camy_addr_ptr, cameraY->data, sizeof(DWORD));

	if (camx_addr_ptr != 0 && camy_addr_ptr != 0)
	{
		this->camx_addr = camx_addr_ptr - this->mp->base_addr;
		this->camy_addr = camy_addr_ptr - this->mp->base_addr;
	}
	else
		warning("Cannot find camera position");

	bb_queue_free_all(res, buffer_free);
}

void camera_scan_entities_arr ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "eArrEnd/eArrStart",
	/*
		00A36FD1    57              		push edi
		00A36FD2  ▼ 0F84 FA000000   		je League_Of_Legends.00A370D2
		00A36FD8    8B0D <<<C0F3D802>>>   	mov ecx, [dword ds:League_Of_Legends.2D8F3C0]  <-- eArrEnd
		00A36FDE    8B2D <<<BCF3D802>>>   	mov ebp, [dword ds:League_Of_Legends.2D8F3BC]  <-- eArrStart
		00A36FE4    3BE9            		cmp ebp, ecx
	*/
			"\x57"
			"\x0F\x84\xFA\x00\x00\x00"
			"\x8B\x0D\xC0\xF3\xD8\x02"
			"\x8B\x2D\xBC\xF3\xD8\x02"
			"\x3B\xE9",

			"x"
			"xxxxxx"
			"xx????"
			"xx????"
			"xx",

			NULL // <-- request the same mask than search_mask
	);

	if (!res)
	{
		warning("Cannot find entities array address\nUsing the .ini value : 0x%.8x\n", this->entities_addr);
		return;
	}

	Buffer *eArrEnd   = bb_queue_pick_first(res),
		   *eArrStart = bb_queue_pick_last(res);

	memcpy(&this->entities_addr, eArrStart->data, eArrStart->size);
	memcpy(&this->entities_addr_end, eArrEnd->data, eArrEnd->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->entities_addr)
	{
		warning("Cannot scan entities");
		return;
	}
}

void camera_scan_variables ()
{
	info("------------------------------------------------------------------");
	info("Searching for variables address ...");
	camera_scan_campos();
	camera_scan_entities_arr();

	info("------------------------------------------------------------------");
}

void camera_scan_champions ()
{
	Camera *this = camera_get_instance();

	DWORD entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	DWORD entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);

	for (int i = 0; entity_ptr != entity_ptr_end; entity_ptr+=4, i++)
	{
		if (this->champions[i] != NULL)
			entity_free(this->champions[i]);

		Entity *e = this->champions[i] = entity_new(this->mp, entity_ptr);

		if (e == NULL && i != 0)
			info("  --> Ally %d not found", i);
		else
		{
			info("  --> Ally %d found (pos: x=%.0f y=%.0f hp=%.0f hpmax=%.0f)",
				i, e->v.x, e->v.y, e->hp, e->hp_max);
		}
	}


	info("------------------------------------------------------------------");
}

BOOL camera_scan_mouse_screen ()
{
	Camera *this = camera_get_instance();

	if (!this->mouse_screen_ptr)
	{
		warning("Cannot get mouse screen coordinates");
		return FALSE;
	}

	this->mouse_screen_addr = read_memory_as_int(this->mp->proc, this->mouse_screen_ptr);

	if (this->mouse_screen != NULL)
	{
		this->mouse_screen->addrX = this->mouse_screen_addr + 0x4C;
		this->mouse_screen->addrY = this->mouse_screen_addr + 0x50;
	}

	return (this->mouse_screen_addr != 0);
}

BOOL camera_scan_shop_is_opened ()
{
	Camera *this = camera_get_instance();

	// Shop is open is the address of the pointer to the "isShopOpened"
	this->shop_is_opened_addr = read_memory_as_int(this->mp->proc, this->shop_is_opened_ptr);

	if (!this->shop_is_opened_addr)
		return FALSE;

	// isShopOpen = edi+7c
	this->shop_is_opened_addr = this->shop_is_opened_addr + 0x7c;

	return TRUE;
}

BOOL camera_refresh_shop_is_opened ()
{
	Camera *this = camera_get_instance();

	unsigned char buffer[1];
	read_from_memory(this->mp->proc, buffer, this->shop_is_opened_addr, 1);
	this->shop_opened = (int) buffer[0];

	return TRUE;
}


// ------------ Scanners ------------

static Patch *camera_get_patch (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask)
{
	// Get the address of the signature
	camera_search_signature (sig, addr, sig_mask, description);

	// Create a new patch
	return patch_new (description, mp, *addr, sig, patch, patch_mask);
}

static void camera_get_patches (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask)
{
	// Get the address of the signature
	BbQueue *occs = camera_search_signatures (sig, sig_mask, description, addrs, size);
	int loop = 0;

	foreach_bbqueue_item_raw (occs, DWORD addr)
	{
		char *newdesc = str_dup_printf("%s (%d)", description, loop);
		patches[loop++] = patch_new (newdesc, mp, addr, sig, patch, patch_mask);
		free(newdesc);

		if (loop > size)
			break;
	}
}

static void camera_search_signature (unsigned char *pattern, DWORD *addr, char *mask, char *name)
{
	Camera *this = camera_get_instance();
	infob("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0)
	{
		printf("\n");
		warning("\"%s\" not found (already patched ?)\nUsing the current .ini value : 0x%.8x", name, *addr);
		return;
	}

	if (bb_queue_get_length(results) > 1)
	{
		printf("\n");
		warning("Multiple occurences of \"%s\" found (%d found) :", name, bb_queue_get_length(results));

		foreach_bbqueue_item (results, memblock) {
			printf(" -> 0x%.8x\n", (int) memblock->addr);
		}
	}

	memblock = bb_queue_pick_first(results);
	*addr = memblock->addr;
	printf(" -> 0x%.8x\n", (int) memblock->addr);

	bb_queue_free_all(results, memblock_free);
}

static BbQueue *camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size)
{
	Camera *this = camera_get_instance();
	infob("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *addresses = bb_queue_new();
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0)
	{
		printf("\n");
		warning("\"%s\" not found (already patched ?)\nUsing the current .ini value :", name);
		for (int i = 0; i < size; i++)
		{
			bb_queue_add_raw(addresses, *(addr[i]));
			printf("  --> [%d] - 0x%.8x\n", i, (int) *(addr[i]));
		}

		return addresses;
	}

	if (bb_queue_get_length(results) != size)
	{
		printf("\n");
		warning("Occurences excepted was %d, %d found.", size, bb_queue_get_length(results));

		if (bb_queue_get_length(results) < size)
		{
			for (int i = bb_queue_get_length(results); i < size; i++)
			{
				bb_queue_add_raw(addresses, *(addr[i]));
			}
		}
	}

	int loop = 0;

	printf(" ->");

	foreach_bbqueue_item (results, memblock) {
		bb_queue_add_raw(addresses, memblock->addr);
		*(addr[loop++]) = memblock->addr;
		printf(" 0x%.8x -", (int) memblock->addr);
	}

	printf("\n");

	bb_queue_free_all(results, memblock_free);

	return addresses;
}
