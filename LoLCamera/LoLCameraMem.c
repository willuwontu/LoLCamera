#include "LoLCamera.h"

static BbQueue * camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size);
static void      camera_search_signature  (unsigned char *pattern, DWORD *addr, char *mask, char *name);
static Patch *   camera_get_patch         (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);
static void      camera_get_patches       (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);

BOOL camera_scan_patch ()
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

	return TRUE;
}

BOOL camera_scan_campos ()
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
		warning("Cannot find %s address\nUsing the .ini value : 0x%.8x", description, this->entities_addr);
		return FALSE;
	}

	Buffer *cameraX = bb_queue_pick_first(res);
	Buffer *cameraY = bb_queue_pick_last(res);

	DWORD camx_addr_ptr, camy_addr_ptr;
	memcpy(&camx_addr_ptr, cameraX->data, sizeof(DWORD));
	memcpy(&camy_addr_ptr, cameraY->data, sizeof(DWORD));
	bb_queue_free_all(res, buffer_free);

	if (!camx_addr_ptr || !camy_addr_ptr)
	{
		warning("Cannot find camera position");
		return FALSE;
	}

	this->camx_addr = camx_addr_ptr - this->mp->base_addr;
	this->camy_addr = camy_addr_ptr - this->mp->base_addr;

	return TRUE;
}

BOOL camera_scan_loading ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "loadingState",
	/*
		00A45799  ║► └F30F100D 40D88101            movss xmm1, [dword ds:League_of_Legends.181D840]                  ; float 0.05694580
		00A457A1  ║·  F30F1015 B4DB6E01            movss xmm2, [dword ds:League_of_Legends.16EDBB4]                  ; float 10.00000
		00A457A9  ║·  0F28C1                       movaps xmm0, xmm1
		00A457AC  ║·  F30F5905 B4A0B001            mulss xmm0, [dword ds:League_of_Legends.1B0A0B4]                  ; float 0.004300000
		00A457B4  ║·  F30F5805 349FD301            addss xmm0, [dword ds:League_of_Legends.1D39F34]                  ; float 3.239630
		00A457BC  ║·  0F2FC2                       comiss xmm0, xmm2
		00A457BF  ║·  F30F590D <<4CA0B001>>        mulss xmm1, [dword ds:League_of_Legends.1B0A04C]                  ; float 0.002000000
		00A457C7  ║·  F30F580D 389FD301            addss xmm1, [dword ds:League_of_Legends.1D39F38]                  ; float 1.506778
	*/
		(unsigned char[]) {
			0xF3,0x0F,0x10,0x0D,0x40,0xD8,0x81,0x01,
			0xF3,0x0F,0x10,0x15,0xB4,0xDB,0x6E,0x01,
			0x0F,0x28,0xC1,
			0xF3,0x0F,0x59,0x05,0xB4,0xA0,0xB0,0x01,
			0xF3,0x0F,0x58,0x05,0x34,0x9F,0xD3,0x01,
			0x0F,0x2F,0xC2,
			0xF3,0x0F,0x59,0x0D,0x4C,0xA0,0xB0,0x01,
			0xF3,0x0F,0x58,0x0D,0x38,0x9F,0xD3,0x01
		},

		"xxxx????"
		"xxxx????"
		"xxx"
		"xxxx????"
		"xxxx????"
		"xxx"
		"xxxx????"
		"xxxx????",

		"xxxxxxxx"
		"xxxxxxxx"
		"xxx"
		"xxxxxxxx"
		"xxxxxxxx"
		"xxx"
		"xxxx????"
		"xxxxxxxx"
	);

	if (!res)
	{
		warning("Cannot find loading state address\nUsing the .ini value : 0x%.8x", this->loading_state_addr);
		return FALSE;
	}

	Buffer *loading_state_addr = bb_queue_pick_first(res);
	memcpy(&this->loading_state_addr, loading_state_addr->data, loading_state_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->loading_state_addr)
	{
		warning("Cannot scan loading state");
		return FALSE;
	}

	return TRUE;
}

BOOL camera_scan_game_struct ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "gameStruct",
	/*
		00A0B08F    ║·  A1 2871DF03                  mov eax, [dword ds:League_of_Legends.GameStruct]                  ; GameStruct
		00A0B094    ║·  83F8 01                      cmp eax, 1                                                        ; Cascaded IF (cases 1..3, 4 exits)
		00A0B097    ║·  53                           push ebx
		00A0B098    ║·  B3 01                        mov bl, 1
		00A0B09A    ║·  885C24 0F                    mov [byte ss:arg.3+3], bl
		00A0B09E    ║·▼ 75 2E                        jne short League_of_Legends.00A0B0CE
	*/
		(unsigned char[]) {
			0xA1,0x28,0x71,0xDF,0x03,
			0x83,0xF8,0x01,
			0x53,
			0xB3,0x01,
			0x88,0x5C,0x24,0x0F,
			0x75,0x2E
		},

		"x????"
		"xxx"
		"x"
		"xx"
		"xxxx"
		"xx",

		NULL // same
	);

	if (!res)
	{
		warning("Cannot find game struct address\nUsing the .ini value : 0x%.8x", this->game_struct_addr);
		return FALSE;
	}

	Buffer *game_struct_addr = bb_queue_pick_first(res);
	memcpy(&this->game_struct_addr, game_struct_addr->data, game_struct_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->game_struct_addr)
	{
		warning("Cannot scan game struct");
		return FALSE;
	}

	// 00A38400    ║·  D99E F0010000                fstp [dword ds:esi+1F0]
	this->champx_addr = this->game_struct_addr + 0x1F0 - this->mp->base_addr;
	// 00A38412    ║·  D99E F8010000                fstp [dword ds:esi+1F8]
	this->champy_addr = this->game_struct_addr + 0x1F8 - this->mp->base_addr;

	// 00A383D5    ║·  F30F1186 FC010000            movss [dword ds:esi+1FC], xmm0
	this->mousex_addr = this->game_struct_addr + 0x1FC - this->mp->base_addr;
	// 00A383F5    ║·  F30F1186 04020000            movss [dword ds:esi+204], xmm0
	this->mousey_addr = this->game_struct_addr + 0x204 - this->mp->base_addr;

	// 00A3B34C    ║·  F30F1183 D0020000            movss [dword ds:ebx+2D0], xmm0
	this->destx_addr  = this->game_struct_addr + 0x2D0 - this->mp->base_addr;
	// 00A3B36C    ║·  F30F1183 D8020000            movss [dword ds:ebx+2D8], xmm0
	this->desty_addr  = this->game_struct_addr + 0x2D8 - this->mp->base_addr;

	return TRUE;
}

BOOL camera_scan_variables ()
{
	BOOL res = TRUE;

	info("------------------------------------------------------------------");
	info("Searching for static variables address ...");

	/*
		entities_addr = 0x2d8f3bc			OK	camera_scan_entities_arr
		entities_addr_end = 0x2d8f3c0		OK	camera_scan_entities_arr
		camera_posx_addr = 0x039f713c		OK	camera_scan_campos
		camera_posy_addr = 0x039f7144		OK	camera_scan_campos
		champion_posx_addr = 0x039f7318		OK	camera_scan_game_struct
		champion_posy_addr = 0x039f7320		OK	camera_scan_game_struct
		mouse_posx_addr = 0x039f7324		OK	camera_scan_game_struct
		mouse_posy_addr = 0x039f732c		OK	camera_scan_game_struct
		dest_posx_addr = 0x039f73F8			OK	camera_scan_game_struct
		dest_posy_addr = 0x039f7400			OK	camera_scan_game_struct
		mouse_screen_ptr = 0x039f39a4		OK	camera_scan_mouse_screen
	*/

	BOOL (*scan_funcs[])(void) = {
		camera_scan_campos,
		camera_scan_loading,
		camera_scan_mouse_screen,
		camera_scan_game_struct,
		camera_scan_shop_is_opened
	};

	for (int i = 0; i < (sizeof(scan_funcs) / sizeof(BOOL (*)())); i++)
	{
		if (!scan_funcs[i]())
			res = FALSE;
	}


	info("------------------------------------------------------------------");
	info("Reading the content of pointers...");
	camera_scan_champions();

	info("------------------------------------------------------------------");

	return res;
}

BOOL camera_scan_champions ()
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
		warning("Cannot find entities array address\nUsing the .ini value : 0x%.8x", this->entities_addr);
		return FALSE;
	}

	Buffer *eArrEnd   = bb_queue_pick_first(res),
		   *eArrStart = bb_queue_pick_last(res);

	memcpy(&this->entities_addr, eArrStart->data, eArrStart->size);
	memcpy(&this->entities_addr_end, eArrEnd->data, eArrEnd->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->entities_addr)
	{
		warning("Cannot scan entities");
		return FALSE;
	}

	this->entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	this->entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);

	if (!this->entity_ptr || !this->entity_ptr_end)
	{
		warning("Cannot read entity array boundaries");
		return FALSE;
	}

	for (int i = 0; this->entity_ptr != this->entity_ptr_end && i < 10; this->entity_ptr += 4, i++)
	{
		Entity *e = this->champions[i];

		if (e == NULL)
			this->champions[i] = e = entity_new(this->mp, this->entity_ptr);
		else
			entity_init(e, this->mp, this->entity_ptr);

		if (e == NULL) // 0 = self
			info("  --> Ally %d not found", i);
		else
			info("  --> Entity %d found (pos: x=%.0f y=%.0f hp=%.0f hpmax=%.0f - 0x%.8x)", i, e->p.v.x, e->p.v.y, e->hp, e->hp_max, this->entity_ptr);
	}

	info("------------------------------------------------------------------");

	return TRUE;
}

BOOL camera_refresh_champions ()
{
	Camera *this = camera_get_instance();

	DWORD entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	DWORD entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);

	this->team_size = (entity_ptr_end - entity_ptr) / 4;

	for (int i = 0; entity_ptr != entity_ptr_end && i < 10; entity_ptr+=4, i++)
	{
		if (!entity_refresh(this->champions[i]))
		{
			if (this->champions[i])
			{
				warning("Entity 0x%.8x cannot be refreshed", this->champions[i]->entity_data);
				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOL camera_scan_mouse_screen ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "mouseScreenPtr",
		/*
		00AD00B5    ║·  8B0D <<A439DF03>>            mov ecx, [dword ds:League_of_Legends.3DF39A4]
		00AD00BB    ║·  8B01                         mov eax, [dword ds:ecx]
		00AD00BD    ║·  8B40 5C                      mov eax, [dword ds:eax+5C]
		00AD00C0    ║·  8D5424 08                    lea edx, [local.64]
		00AD00C4    ║·  52                           push edx
		00AD00C5    ║·  8D5424 10                    lea edx, [local.63]
		00AD00C9    ║·  52                           push edx
		*/
		(unsigned char[]) {
			0x8B,0x0D,0xA4,0x39,0xDF,0x03,
			0x8B,0x01,
			0x8B,0x40,0x5C,
			0x8D,0x54,0x24,0x08,
			0x52,
			0x8D,0x54,0x24,0x10,
			0x52
		},
		"xx????"
		"xx"
		"xxx"
		"xxxx"
		"x"
		"xxxx"
		"x",
		NULL // same
	);

	if (!res)
	{
		warning("Cannot find mouse_screen_ptr address\nUsing the .ini value : 0x%.8x", this->mouse_screen_ptr);
		return FALSE;
	}

	Buffer *mouse_screen_ptr = bb_queue_pick_first(res);
	memcpy(&this->mouse_screen_ptr, mouse_screen_ptr->data, mouse_screen_ptr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->mouse_screen_ptr)
	{
		warning("Cannot scan mouse_screen_ptr");
		return FALSE;
	}

	DWORD mouse_screen_addr = read_memory_as_int(this->mp->proc, this->mouse_screen_ptr);

	if (mouse_screen_addr != 0)
	{
		this->mouse_screen_addr = mouse_screen_addr;

		if (this->mouse_screen != NULL)
		{
			this->mouse_screen->addrX = this->mouse_screen_addr + 0x4C;
			this->mouse_screen->addrY = this->mouse_screen_addr + 0x50;
		}
	}

	return (mouse_screen_addr != 0);
}

BOOL camera_scan_shop_is_opened ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "shopIsOpened",
	/*	00A382C6    ║► └8B0D <<90A0D301>>            mov ecx, [dword ds:League_of_Legends.1D3A090]
		00A382CC    ║·  33C0                         xor eax, eax
		00A382CE    ║·  3BC8                         cmp ecx, eax
		00A382D0    ║·▼ 74 1F                        je short League_of_Legends.00A382F1 */
		(unsigned char[]) {
			0x8B,0x0D,0x90,0xA0,0xD3,0x01,
			0x33,0xC0,
			0x3B,0xC8,
			0x74,0x1F
		},
		"xx????"
		"xx"
		"xx"
		"xx",
		NULL // same
	);

	if (!res)
	{
		warning("Cannot find shop_is_opened_ptr address\nUsing the .ini value : 0x%.8x", this->shop_is_opened_ptr);
		return FALSE;
	}

	Buffer *shop_is_opened_ptr = bb_queue_pick_first(res);
	memcpy(&this->shop_is_opened_ptr, shop_is_opened_ptr->data, shop_is_opened_ptr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->shop_is_opened_ptr)
	{
		warning("Cannot scan shop_is_opened_ptr");
		return FALSE;
	}

	// Shop is open is the address of the pointer to the "isShopOpened"
	DWORD shop_is_opened_addr = read_memory_as_int(this->mp->proc, this->shop_is_opened_ptr);

	if (!shop_is_opened_addr)
		return FALSE;

	// isShopOpen = edi+7c
	this->shop_is_opened_addr = shop_is_opened_addr + 0x7c;

	return TRUE;
}

BOOL camera_refresh_shop_is_opened ()
{
	Camera *this = camera_get_instance();

	unsigned char buffer[1] = {0xFF};
	read_from_memory(this->mp->proc, buffer, this->shop_is_opened_addr, 1);
	this->shop_opened = (int) buffer[0];

	return (buffer[0] != 0xFF);
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
