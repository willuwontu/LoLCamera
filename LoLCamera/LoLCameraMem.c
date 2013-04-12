#include "LoLCamera.h"

static BbQueue *camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size);
static void camera_search_signature (unsigned char *pattern, DWORD *addr, char *mask, char *name);
static Patch *camera_get_patch (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);
static void camera_get_patches (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);

void camera_scan_patch ()
{
	Camera *this = camera_get_instance();

	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	info("Scanning process...");
	memproc_dump(this->mp, text_section, text_section + text_size);

	// Search for camera positionning instructions
	info("\n------------------------------------------------------------------\nLooking for addresses ...");

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

	camera_get_patches (this->F2345_pressed, 2,

		this->mp, "Center the camera on the ally X when FX is pressed",
		(DWORD *[2]) {
			&this->allies_cam_addr[0],
			&this->allies_cam_addr[1]
		},

		/*	00A370A7  ║·  80BB 2D030000 00    cmp [byte ds:ebx+32D], 0                 ; Case 2 of cascaded IF League_of_Legends.0A3705D
			00A370AE  ║·▼ 74 22               je short League_of_Legends.00A370D2
			00A370B0  ║·  D946 6C             fld [dword ds:esi+6C]                    ; push *(esi+6C) (esi=381988C0)		<--- start NOPing here
			00A370B3  ║·  D95B 14             [1] fstp [dword ds:ebx+14]               ; (cameraX / *(ebx+14)) = pop()
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

	info("\n------------------------------------------------------------------\n");
}

void camera_scan_champions ()
{
	Camera *this = camera_get_instance();

	/*
		00A36FD1    57              push edi
		00A36FD2  ▼ 0F84 FA000000   je League_Of_Legends.00A370D2
		00A36FD8    8B0D C0F3D802   mov ecx, [dword ds:League_Of_Legends.2D8F3C0]  <-- EntitiesArrayEnd
		00A36FDE    8B2D BCF3D802   mov ebp, [dword ds:League_Of_Legends.2D8F3BC]  <-- EntitiesArrayStart
		00A36FE4    3BE9            cmp ebp, ecx
	*/
	// Todo : sigscanner for entities_array
	if (!this->entities_array_addr)
	{
		warning("Cannot scan entities");
		return;
	}

	DWORD entity_ptr = read_memory_as_int(this->mp->proc, this->entities_array_addr);

	for (int i = 0; i < 5; i++)
	{
		this->champions[i] = entity_new(this->mp, entity_ptr);
		if (this->champions[i] == NULL && i != 0)
			info("  --> Ally %d not found", i);

		entity_ptr += 4;
	}
}

int camera_shop_is_opened ()
{
	Camera *this = camera_get_instance();

	// Shop is open is the address of the pointer to the "isShopOpened"
	DWORD addr = read_memory_as_int(this->mp->proc, this->shop_is_opened_addr);

	if (!addr)
		return 0;

	// isShopOpen = edi+7c
	addr = addr + 0x7c;

	unsigned char buffer[1];
	read_from_memory(this->mp->proc, buffer, addr, 1);
	return (int) buffer[0];
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
	DWORD addr;
	int loop = 0;

	foreach_bbqueue_item_raw (occs, addr)
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
	info("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0) {
		warning("\"%s\" not found (already patched ?)\nUsing the current .ini value : 0x%.8x", name, *addr);
		return;
	}

	if (bb_queue_get_length(results) > 1)
	{
		warning("Multiple occurences of %s found (%d found) :", name, bb_queue_get_length(results));

		foreach_bbqueue_item (results, memblock) {
			printf(" -> 0x%.8x\n", (int) memblock->addr);
		}
	}

	memblock = bb_queue_pick_first(results);
	*addr = memblock->addr;

	bb_queue_free_all(results, memblock_free);
}

static BbQueue *camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size)
{
	Camera *this = camera_get_instance();
	info("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *addresses = bb_queue_new();
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0) {

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

	foreach_bbqueue_item (results, memblock) {
		bb_queue_add_raw(addresses, memblock->addr);
		*(addr[loop++]) = memblock->addr;
	}

	bb_queue_free_all(results, memblock_free);

	return addresses;
}
