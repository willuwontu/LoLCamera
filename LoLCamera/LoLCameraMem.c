#include "LoLCamera.h"

#define NOT_A_POINTER(addr) ((addr) < 0x10000)

#define SELF_NAME_OFFSET 0x28
#define WIN_IS_OPENED_OFFSET   0x308

static bool      camera_search_signature  (unsigned char *pattern, DWORD *addr, unsigned char **code_ptr, char *mask, char *name);
static Patch *   camera_get_patch         (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);
bool camera_scan_win_is_opened_offset (void);

bool camera_scan_patch (void)
{
	Camera *this = camera_get_instance();

	// Search for camera positionning instructions
	info("------------------------------------------------------------------");
	info("Looking for patch addresses ...");

	this->camera_movement = camera_get_patch (
		this->mp, "Modify camera position (memory)",
		&this->camera_movement_addr,

		(unsigned char[]) {
		/*
			016A0991  ║·  F30F580D 6FCF3402       addss xmm1, [dword ds:League_of_Legends.234CF6F]                                            ; float 1293.950
			016A0999  ║·  F30F581D 73CF3402       addss xmm3, [dword ds:League_of_Legends.234CF73]                                            ; float 0.0
			016A09A1  ║·  F30F5825 77CF3402       addss xmm4, [dword ds:League_of_Legends.234CF77]                                            ; float 733.0612
			016A09A9  ║·  F30F110D 6FCF3402       movss [dword ds:League_of_Legends.234CF6F], xmm1                                            ; float 1293.950
			016A09B1  ║·  F30F111D 73CF3402       movss [dword ds:League_of_Legends.234CF73], xmm3                                            ; float 0.0
			016A09B9  ║·  F30F1125 77CF3402       movss [dword ds:League_of_Legends.234CF77], xmm4                                            ; float 733.0612
		*/
			0xF3,0x0F,0x58,0x0D,0x6F,0xCF,0x34,0x02,
			0xF3,0x0F,0x58,0x1D,0x73,0xCF,0x34,0x02,
			0xF3,0x0F,0x58,0x25,0x77,0xCF,0x34,0x02,
			0xF3,0x0F,0x11,0x0D,0x6F,0xCF,0x34,0x02,
			0xF3,0x0F,0x11,0x1D,0x73,0xCF,0x34,0x02,
			0xF3,0x0F,0x11,0x25,0x77,0xCF,0x34,0x02
		},

			"xxxx????"
			"xxxx????"
			"xxxx????"
			"xxxx????"
			"xxxx????"
			"xxxx????",

		(unsigned char[]) {
			0xF3,0x0F,0x58,0x0D,0x6F,0xCF,0x34,0x02,
			0xF3,0x0F,0x58,0x1D,0x73,0xCF,0x34,0x02,
			0xF3,0x0F,0x58,0x25,0x77,0xCF,0x34,0x02,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
		},
			"????????"
			"????????"
			"????????"
			"xxxxxxxx"
			"xxxxxxxx"
			"xxxxxxxx"
	);


	camera_get_patch (
		this->mp, "Move the camera (value) on border",
		&this->border_screen_addr,

		(unsigned char[]) {
		/*
			007A05EC        ·  F30F111D 2B99BA02       movss [dword ds:League_of_Legends.2BA992B], xmm3       ; float 0.0, 0.0, 0.0, 0.0
			007A05F4        ·  F30F110D 2799BA02       movss [dword ds:League_of_Legends.2BA9927], xmm1       ; float 0.0, 0.0, 0.0, 0.0
			007A05FC        ·  F30F1125 2F99BA02       movss [dword ds:League_of_Legends.2BA992F], xmm4       ; float 0.0, 0.0, 0.0, 0.0
			007A0604        ·  83F9 02                 cmp ecx, 2
			007A0607        ·▼ 74 17                   je short League_of_Legends.007A0620
			007A0609        ·  83F9 04                 cmp ecx, 4
			007A060C        ·▼ 74 12                   je short League_of_Legends.007A0620
		*/
			0xF3,0x0F,0x11,0x1D,0x2B,0x99,0xBA,0x02,
			0xF3,0x0F,0x11,0x0D,0x27,0x99,0xBA,0x02,
			0xF3,0x0F,0x11,0x25,0x2F,0x99,0xBA,0x02,
			0x83,0xF9,0x02,
			0x74,0x17,
			0x83,0xF9,0x04,
			0x74,0x12
		},
			"xxx?????"
			"xxx?????"
			"xxx?????"
			"xxx"
			"xx"
			"xxx"
			"xx",

		(unsigned char[]) {
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x83,0xF9,0x02,
			0x74,0x17,
			0x83,0xF9,0x04,
			0x74,0x12
		},
			"xxxxxxxx"
			"xxxxxxxx"
			"xxxxxxxx"
			"???"
			"??"
			"???"
			"??"
	);

	camera_get_patch (
		this->mp, "Lock the camera (value) on camera locked",
		&this->locked_camera_addr,

		(unsigned char[]) {
		/*
				0107FE54  ║· │803D CC9B4803 00        cmp [byte ds:League_of_Legends.3489BCC], 0
				0107FE5B  ║·▼│0F84 7D020000           je League_of_Legends.010800DE
				0107FE61  ║► │F30F1047 64             movss xmm0, [dword ds:edi+64]                        ; float 0.0004092904
				0107FE66  ║· │F30F1105 27994803       movss [dword ds:League_of_Legends.3489927], xmm0     ; float 0.0, 0.0, 0.0, 3036.157
				0107FE6E  ║· │F30F1047 68             movss xmm0, [dword ds:edi+68]                        ; float 0.0003991595
				0107FE73  ║· │F30F1105 2B994803       movss [dword ds:League_of_Legends.348992B], xmm0     ; float 0.0, 0.0, 0.0, 3036.157
				0107FE7B  ║· │F30F1047 6C             movss xmm0, [dword ds:edi+6C]                        ; float 0.0004054068
				0107FE80  ║· │F30F1105 2F994803       movss [dword ds:League_of_Legends.348992F], xmm0     ; float 0.0, 0.0, 0.0, 3036.157
				01698515  ║· │8B4D F4                 mov ecx, [dword ss:ebp-0C]
		*/
			0x80,0x3D,0xCC,0x9B,0x48,0x03,0x00,
			0x0F,0x84,0x7D,0x02,0x00,0x00,
			0xF3,0x0F,0x10,0x47,0x64,
			0xF3,0x0F,0x11,0x05,0x27,0x99,0x48,0x03,
			0xF3,0x0F,0x10,0x47,0x68,
			0xF3,0x0F,0x11,0x05,0x2B,0x99,0x48,0x03,
			0xF3,0x0F,0x10,0x47,0x6C,
			0xF3,0x0F,0x11,0x05,0x2F,0x99,0x48,0x03,
			0x8B,0x4D,0xF4,
		},
			"xx????x"
			"xx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xxxx????"
			"xx?",

		(unsigned char[]) {
			0x80,0x3D,0xCC,0x9B,0x48,0x03,0x00,
			0x0F,0x84,0x7D,0x02,0x00,0x00,
			0xF3,0x0F,0x10,0x47,0x64,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, // < nop
			0xF3,0x0F,0x10,0x47,0x68,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, // < nop
			0xF3,0x0F,0x10,0x47,0x6C,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90, // < nop
			0x8B,0x4D,0xF4
		},
			"???????"
			"??????"
			"?????"
			"xxxxxxxx"
			"?????"
			"xxxxxxxx"
			"?????"
			"xxxxxxxx"
			"???"
	);

	this->patchlist = patch_list_get();

	return true;
}

bool camera_scan_camval (void)
{
	Camera *this = camera_get_instance();
	unsigned char *description = "CameraX_Value/CameraY_Value";

	BbQueue *res = memscan_search(this->mp, description,
		(unsigned char []) {
		/*
			Find Camera_value :

			1:
				68E6D24A    ║·   [1] lea edi, [ecx+5254]
				[...]
				68E6D254    ║·   movs [dword es:edi], [dword ds:esi]

			2:
				68E90A89    ║·   [2] mov ecx, [dword ss:local.1] <-----------------------
				68E90A8C    ║·   push [dword ss:arg5]                                       ; ║Arg4 => [Arg5]
				68E90A8F    ║·   push [dword ss:arg4]                                       ; ║Arg3 => [Arg4]
				68E90A92    ║·   push [dword ss:arg3]                                       ; ║Arg2 => [Arg3]
				68E90A95    ║·   push [dword ss:arg2]                                       ; ║Arg1 => [Arg2]
				68E90A98    ║·   call fmodex.68E6D0FF                                       ; └fmodex.68E6D0FF <- FMOD::System::set3DListenerAttributesArg1,Arg2,Arg3,Arg4,Arg5,Arg6)

			2:
				69731A36    ║·  [3] push [dword ds:esi+14]                                 ; ║Arg1
				69731A39    ║·  call <jmp.&fmodex.FMOD::System::set3DListenerAttributes>   ; └fmodex.FMOD::System::set3DListenerAttributes


			01519D65       ║► └F30F1005 57BBD303       movss xmm0, [dword ds:League_of_Legends.3D3BB57]            ; float 739.0000 <--- cameraX
			01519D6D       ║·  8B0D 38CB4702           mov ecx, [dword ds:League_of_Legends.247CB38]
			01519D73       ║·  F30F1145 D8             movss [dword ss:local.10], xmm0
			01519D78       ║·  F30F1005 5BBBD303       movss xmm0, [dword ds:League_of_Legends.3D3BB5B]            ; float 0.0
			01519D80       ║·  F30F1145 DC             movss [dword ss:local.9], xmm0
			01519D85       ║·  F30F1005 5FBBD303       [9] movss xmm0, [dword ds:League_of_Legends.3D3BB5F]        ; float 300.0000 <--- cameraY
			01519D8D       ║·  F30F1145 E0             movss [dword ss:local.8], xmm0
			01519D92       ║·  8B01                    mov eax, [dword ds:ecx]
			01519D94       ║·  8B80 88000000           mov eax, [dword ds:eax+88]
			01519D9A       ║·  FFD0                    call eax
		*/
			0xF3,0x0F,0x10,0x05,0x57,0xBB,0xD3,0x03,
			0x8B,0x0D,0x38,0xCB,0x47,0x02,
			0xF3,0x0F,0x11,0x45,0xD8,
			0xF3,0x0F,0x10,0x05,0x5B,0xBB,0xD3,0x03,
			0xF3,0x0F,0x11,0x45,0xDC,
			0xF3,0x0F,0x10,0x05,0x5F,0xBB,0xD3,0x03,
			0xF3,0x0F,0x11,0x45,0xE0,
			0x8B,0x01,
			0x8B,0x80,0x88,0x00,0x00,0x00,
			0xFF,0xD0
		},

			"xxxx????"
			"xx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xx"
			"xx?xxx"
			"xx",

			"xxxx????"
			"xxxxxx"
			"xxxxx"
			"xxxxxxxx"
			"xxxxx"
			"xxxx????"
			"xxxxx"
			"xx"
			"xxxxxx"
			"xx"

	);

	if (!res)
	{
		warning("Cannot find %s address", description);
		return false;
	}

	DWORD camx_addr_ptr, camy_addr_ptr;
	Buffer *cameraX = bb_queue_pick_first(res);
	Buffer *cameraY = bb_queue_pick_last(res);
	memcpy(&camx_addr_ptr, cameraX->data, sizeof(DWORD));
	memcpy(&camy_addr_ptr, cameraY->data, sizeof(DWORD));

	bb_queue_free_all(res, buffer_free);

	if (!camx_addr_ptr || !camy_addr_ptr)
	{
		warning("Cannot find camera position");
		return false;
	}

	this->camx_val = camx_addr_ptr - this->mp->base_addr;
	this->camy_val = camy_addr_ptr - this->mp->base_addr;

	return true;
}

bool camera_posmemory_cond (MemProc *mp, BbQueue *results)
{
	Buffer *b[3];
	DWORD ptr[3];

	b[0] = bb_queue_pick_nth(results, 1);
	b[1] = bb_queue_pick_nth(results, 2);
	b[2] = bb_queue_pick_nth(results, 3);

	memcpy(&ptr[0], b[0]->data, sizeof(DWORD));
	memcpy(&ptr[1], b[1]->data, sizeof(DWORD));
	memcpy(&ptr[2], b[2]->data, sizeof(DWORD));

	if ((ptr[0] + 4 != ptr[1])
	||  (ptr[0] + 8 != ptr[2]))
		return false;

	float x = read_memory_as_float(mp->proc, ptr[0]);
	float y = read_memory_as_float(mp->proc, ptr[2]);

	if (x == 0.0 || y == 0.0)
		return false;

	return true;
}

bool camera_scan_campos (void)
{
	Camera *this = camera_get_instance();
	unsigned char *description = "CameraX_Memory/CameraY_Memory";

	BbQueue *res = memscan_search_cond (this->mp, description,
		(unsigned char []) {
		/*
			015325C4  ║·  F30F1006                movss xmm0, [dword ds:esi]
			015325C8  ║·  F30F1105 6FCF4602       movss [dword ds:League_of_Legends.246CF6F], xmm0  ; float 300.0000
			015325D0  ║·  F30F1046 04             movss xmm0, [dword ds:esi+4]
			015325D5  ║·  F30F1105 73CF4602       movss [dword ds:League_of_Legends.246CF73], xmm0  ; float 0.0
			015325DD  ║·  F30F1046 08             movss xmm0, [dword ds:esi+8]
			015325E2  ║·  F30F1105 77CF4602       movss [dword ds:League_of_Legends.246CF77], xmm0  ; float 475.0000
		*/
			0xF3,0x0F,0x10,0x06,
			0xF3,0x0F,0x11,0x05,0x6F,0xCF,0x46,0x02,
			0xF3,0x0F,0x10,0x46,0x04,
			0xF3,0x0F,0x11,0x05,0x73,0xCF,0x46,0x02,
			0xF3,0x0F,0x10,0x46,0x08,
			0xF3,0x0F,0x11,0x05,0x77,0xCF,0x46,0x02
		},

			"xxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????",

			"xxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????"
			"xxxxx"
			"xxxx????",

			camera_posmemory_cond);

	if (!res)
	{
		warning("Cannot find %s address", description);
		return false;
	}

	DWORD camx_addr_ptr, camy_addr_ptr;
	Buffer *cameraX = bb_queue_pick_first(res);
	memcpy(&camx_addr_ptr, cameraX->data, sizeof(DWORD));
	camy_addr_ptr = camx_addr_ptr + 8;

	bb_queue_free_all(res, buffer_free);

	if (!camx_addr_ptr || !camy_addr_ptr)
	{
		warning("Cannot find camera position");
		return false;
	}

	this->camx_addr  = camx_addr_ptr  - this->mp->base_addr;
	this->camy_addr  = camy_addr_ptr  - this->mp->base_addr;

	return true;
}

bool camera_scan_hover_interface (void)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "HoverInterface",
	/*
		0107EF8D  ║·  C705 D89A4803 FFFF7FFF           mov [dword ds:League_of_Legends.3489AD8], FF7FFFFF               ; ║
		0107EF97  ║·  C705 DC9A4803 00000000           mov [dword ds:League_of_Legends.3489ADC], 0                      ; ║
		0107EFA1  ║·  C705 E09A4803 FFFF7FFF           mov [dword ds:League_of_Legends.3489AE0], FF7FFFFF               ; ║
		0107EFAB  ║·  C705 E49A4803 00000000           mov [dword ds:League_of_Legends.3489AE4], 0                      ; ║
		0107EFB5  ║·  C605 <E89A4803> 00                 mov [byte ds:League_of_Legends.3489AE8], 0                     ; ║
		0107EFBC  ║·  C705 EC9A4803 00000000           mov [dword ds:League_of_Legends.3489AEC], 0                      ; ║
		0107EFC6  ║·  C705 F09A4803 FFFFFFFF           mov [dword ds:League_of_Legends.3489AF0], -1                     ; ║
		0107EFD0  ║·  C705 F49A4803 FFFFFFFF           mov [dword ds:League_of_Legends.3489AF4], -1                     ; ║
	*/
		(unsigned char[]) {
			0xC7,0x05,0xD8,0x9A,0x48,0x03,0xFF,0xFF,0x7F,0xFF,
			0xC7,0x05,0xDC,0x9A,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0xE0,0x9A,0x48,0x03,0xFF,0xFF,0x7F,0xFF,
			0xC7,0x05,0xE4,0x9A,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC6,0x05,0xE8,0x9A,0x48,0x03,0x00,
			0xC7,0x05,0xEC,0x9A,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0xF0,0x9A,0x48,0x03,0xFF,0xFF,0xFF,0xFF,
			0xC7,0x05,0xF4,0x9A,0x48,0x03,0xFF,0xFF,0xFF,0xFF
		},
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????x"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx",

			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xx????x"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
	);

	if (!res)
	{
		warning("Cannot find HoverInterface address");
		return false;
	}

	Buffer *interface_hovered_addr = bb_queue_pick_first(res);
	memcpy(&this->interface_hovered_addr, interface_hovered_addr->data, interface_hovered_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->interface_hovered_addr)
	{
		warning("Cannot scan HoverInterface");
		return false;
	}

	camera_refresh_hover_interface();

	return true;
}

bool camera_scan_game_info (void)
{
	Camera *this = camera_get_instance();

	/*
	BbQueue *res = memscan_search (this->mp, "gameState",
		00477293  ║·  833D FC13DA02 00   cmp [dword ds:League_of_Legends.2DA13FC], 0
		0047729A  ║·▼ 75 4A              jne short League_of_Legends.004772E6
		0047729C  ║·  803D CB13DA02 00   cmp [byte ds:League_of_Legends.2DA13CB], 0
		004772A3  ║·▼ 74 0E              je short League_of_Legends.004772B3
		004772A5  ║·  C605 CB13DA02 00   mov [byte ds:League_of_Legends.2DA13CB], 0
		004772AC  ║·  C605 080FDA02 01   mov [byte ds:League_of_Legends.2DA0F08], 1
		004772B3      C745 F0 F00CDA02   mov [dword ss:ebp-10], offset League_of_Legends.02DA0CF0
		004772BA  ║·  B9 F00CDA02        mov ecx, offset League_of_Legends.02DA0CF0
		(unsigned char[]) {
			0x83,0x3D,0xFC,0x13,0xDA,0x02,0x00,
			0x75,0x4A,
			0x80,0x3D,0xCB,0x13,0xDA,0x02,0x00,
			0x74,0x0E,
			0xC6,0x05,0xCB,0x13,0xDA,0x02,0x00,
			0xC6,0x05,0x08,0x0F,0xDA,0x02,0x01,
			0xC7,0x45,0xF0,0xF0,0x0C,0xDA,0x02,
			0xB9,0xF0,0x0C,0xDA,0x02
		},

			"xx????x"
			"x?"
			"xx????x"
			"x?"
			"xx????x"
			"xx????x"
			"xx?????"
			"x????",

			"xxxxxxx"
			"xx"
			"xxxxxxx"
			"xx"
			"xxxxxxx"
			"xxxxxxx"
			"xxx????"
			"x????"
	);

	if (!res)
	{
		warning("Cannot find game state address");
		return false;
	}

	int start = 1;
	bool looping = true;
	Buffer *game_info_addr = NULL;
	while (looping)
	{
		DWORD addr[2];
		game_info_addr = bb_queue_pick_nth(res, start);
		Buffer *game_info_addr_same = bb_queue_pick_nth(res, start+1);

		memcpy(&addr[0], game_info_addr->data, game_info_addr->size);
		memcpy(&addr[1], game_info_addr_same->data, game_info_addr_same->size);

		if ((addr[0] == addr[1]) && (addr[0] != 0))
			looping = false;

		else if (++start > (bb_queue_get_length(res) + 1))
		{
			fatal_error("gameState not found.");
			looping = false;
		}
	}

	memcpy(&this->game_info_addr, game_info_addr->data, game_info_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->game_info_addr)
	{
		warning("Cannot scan game state");
		return false;
	}

	read_from_memory(this->mp->proc, this->self_name, this->game_info_addr + SELF_NAME_OFFSET, sizeof(this->self_name) - 1);

	if (str_is_empty(this->self_name))
	{
		warning("Cannot find self name");
		return false;
	}
	else
		debug("Self name (%p) : <%s>", this->game_info_addr + SELF_NAME_OFFSET, this->self_name);

	return true;
	*/

	BbQueue *res = memscan_search (this->mp, "gameInfo",
	/*
		0046FAD2  ║·  64A3 00000000           mov [dword fs:0], eax
		0046FAD8  ║·  833D ACEFC702 00        cmp [dword ds:League_of_Legends.2C7EFAC], 0
		0046FADF  ║·▼ 0F84 DA000000           je League_of_Legends.0046FBBF
		0046FAE5  ║·  A1 9CA0C802             mov eax, [dword ds:League_of_Legends.2C8A09C]
		0046FAEA  ║·  A8 01                   test al, 01
	*/
		(unsigned char[]) {
			0x64,0xA3,0x00,0x00,0x00,0x00,
			0x83,0x3D,0xAC,0xEF,0xC7,0x02,0x00,
			0x0F,0x84,0xDA,0x00,0x00,0x00,
			0xA1,0x9C,0xA0,0xC8,0x02,
			0xA8,0x01
		},

			"xxxxxx"
			"xx????x"
			"xx??xx"
			"x????"
			"xx",

			"xxxxxx"
			"xx????x"
			"xxxxxx"
			"xxxxx"
			"xx"
	);

	if (!res)
	{
		warning("Cannot find game info address");
		return false;
	}

	DWORD addr;
	Buffer *buffer = bb_queue_pick_first(res);
	memcpy(&addr, buffer->data, buffer->size);
	bb_queue_free_all(res, buffer_free);

	if (!addr)
	{
		warning("Cannot scan game info");
		return false;
	}

	this->game_info_addr = read_memory_as_int (this->mp->proc, addr);
	read_from_memory (this->mp->proc, this->self_name, this->game_info_addr + SELF_NAME_OFFSET, sizeof(this->self_name) - 1);

	if (str_is_empty(this->self_name))
	{
		warning("Cannot find self name");
		return false;
	}
	else
		debug("Self name (%p) : <%s>", this->game_info_addr + SELF_NAME_OFFSET, this->self_name);

	return true;
}

bool camera_scan_cursor_champ (void)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "Cursor/Champ",
	/*
		00F59C35  ║·  C745 D8 74BC5E03        mov [dword ss:local.10], offset League_of_Legends.035EBC74
		00F59C3C  ║·  C745 DC 64855E03        mov [dword ss:local.9], offset League_of_Legends.035E8564
		00F59C43  ║·  C745 E0 68BC5E03        mov [dword ss:local.8], offset League_of_Legends.035EBC68
		00F59C4A  ║·  895D E4                 mov [dword ss:local.7], ebx
		00F59C4D  ║·  8955 F0                 mov [dword ss:local.4], edx
		00F59C50  ║·  8945 F4                 mov [dword ss:local.3], eax
		00F59C53  ║·  C645 F8 01              mov [byte ss:local.2], 1
	*/

		(unsigned char[]) {
			0xC7,0x45,0xD8,0x74,0xBC,0x5E,0x03,
			0xC7,0x45,0xDC,0x64,0x85,0x5E,0x03,
			0xC7,0x45,0xE0,0x68,0xBC,0x5E,0x03,
			0x89,0x5D,0xE4,
			0x89,0x55,0xF0,
			0x89,0x45,0xF4,
			0xC6,0x45,0xF8,0x01
		},
			"xxx????"
			"xxx????"
			"xxx????"
			"xxx"
			"xxx"
			"xxx"
			"xxxx",

			"xxx????"
			"xxxxxxx"
			"xxx????"
			"xxx"
			"xxx"
			"xxx"
			"xxxx"
	);

	if (!res)
	{
		warning("Cannot find mouse or champion structure");
		return false;
	}

	Buffer *cursorX = bb_queue_pick_first(res);
	Buffer *champX  = bb_queue_pick_last(res);

	memcpy(&this->mousex_addr, cursorX->data, cursorX->size);
	memcpy(&this->champx_addr, champX->data,  champX->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->mousex_addr || !this->champx_addr)
	{
		warning("Cannot scan mouse or champion position");
		return false;
	}

	this->champx_addr -= this->mp->base_addr;
	this->champy_addr  = this->champx_addr + 8;

	this->mousex_addr -= this->mp->base_addr;
	this->mousey_addr  = this->mousex_addr + 8;

	return true;
}

bool camera_scan_dest (void)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search(this->mp, "DestPos",
		/*
			0107EFE5  ║·  C605 FC9A4803 01                 mov [byte ds:League_of_Legends.3489AFC], 1
			0107EFEC  ║·  C705 009B4803 00000000           mov [dword ds:League_of_Legends.3489B00], 0
			0107EFF6  ║·  C705 289B4803 000000FF           mov [dword ds:League_of_Legends.3489B28], FF000000
			0107F000  ║·  C705 249B4803 00000000           mov [dword ds:League_of_Legends.3489B24], 0
			0107F00A  ║·  C705 2C9B4803 FFFFFFFF           mov [dword ds:League_of_Legends.3489B2C], -1
			0107F014  ║·  C705 309B4803 00000000           mov [dword ds:League_of_Legends.<<<3489B30>>>], 0
		*/
		(unsigned char []) {
			0xC6,0x05,0xFC,0x9A,0x48,0x03,0x01,
			0xC7,0x05,0x00,0x9B,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0x28,0x9B,0x48,0x03,0x00,0x00,0x00,0xFF,
			0xC7,0x05,0x24,0x9B,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0x2C,0x9B,0x48,0x03,0xFF,0xFF,0xFF,0xFF,
			0xC7,0x05,0x30,0x9B,0x48,0x03,0x00,0x00,0x00,0x00
		},

			"xx????x"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx",

			"xxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xx????xxxx"
	);

	if (!res)
	{
		warning("Cannot find Dest offsets\n");
		return false;
	}

	Buffer *dest = bb_queue_pick_first(res);

	memcpy(&this->destx_addr, dest->data, dest->size);

	this->destx_addr -= this->mp->base_addr;
	this->desty_addr = this->destx_addr + 8;

	bb_queue_free_all(res, buffer_free);

	return true;
}

bool camera_scan_variables (void)
{
	bool res = true;


	info("------------------------------------------------------------------");
	info("Searching for static variables address ...");

	bool (*scan_funcs[])(void) = {
		camera_scan_campos,
		camera_scan_camval,
		camera_scan_dest,
		camera_scan_cursor_champ,
		camera_scan_game_info,
		camera_scan_win_is_opened,
		camera_scan_minimap_size,
		camera_scan_ping_or_skill_waiting,
		camera_scan_hover_interface
	};

	for (int i = 0; i < sizeof_array(scan_funcs); i++)
	{
		if (!scan_funcs[i]())
			res = false;
	}

	info("------------------------------------------------------------------");
	info("Reading the content of pointers...");
	camera_scan_champions(true);
	camera_refresh_self();

	return res;
}

bool camera_refresh_self (void)
{
	Camera *this = camera_get_instance();

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
	    if (strcmp(this->self_name, this->champions[i]->player_name) == 0)
		{
			this->self = this->champions[i];
			return (this->self != NULL);
		}
	}

	return false;
}

bool camera_scan_win_is_opened (void)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "winIsOpened",
	/*
		 ║·  FF15 DC50BC00           ║call [dword ds:<&IMM32.ImmAssociateContext>]      ; └IMM32.ImmAssociateContext
		 ║·  8B0D E0D34602           ║mov ecx, [dword ds:League_of_Legends.246D3E0]
		 ║·  8B01                    ║mov eax, [dword ds:ecx]
		 ║·  FF50 04                 ║call [dword ds:eax+4]
	*/
		(unsigned char[]) {
			0xFF,0x15,0xDC,0x50,0xBC,0x00,
			0x8B,0x0D,0xE0,0xD3,0x46,0x02,
			0x8B,0x01,
			0xFF,0x50,0x04
		},
			"xx????"
			"xx????"
			"xx"
			"xxx",

			"xxxxxx"
			"xx????"
			"xx"
			"xxx"
	);

	if (!res)
	{
		warning("Cannot find win_is_opened_ptr address");
		return false;
	}

	Buffer *win_is_opened_ptr = bb_queue_pick_first(res);
	memcpy(&this->win_is_opened_ptr, win_is_opened_ptr->data, win_is_opened_ptr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->win_is_opened_ptr)
	{
		warning("Cannot scan win_is_opened_ptr");
		return false;
	}

	DWORD win_is_opened_addr;
	win_is_opened_addr  = read_memory_as_int(this->mp->proc, this->win_is_opened_ptr);
	win_is_opened_addr += 0x54;

	if (!win_is_opened_addr)
		return false;

	this->win_is_opened_addr = win_is_opened_addr + WIN_IS_OPENED_OFFSET;

	return true;
}

bool camera_win_offset_cond (MemProc *mp, BbQueue *results)
{
	(void) mp;

	Buffer *b[2];
	DWORD off[2];

	b[0] = bb_queue_pick_first(results);
	b[1] = bb_queue_pick_last(results);

	memcpy(&off[0], b[0]->data, sizeof(DWORD));
	memcpy(&off[1], b[1]->data, sizeof(DWORD));

	// offset 1 = offset 2
	if (off[0] != off[1])
		return false;

	return true;
}

bool camera_minimap_size_cond (MemProc *mp, BbQueue *results)
{
	Buffer *b;
	DWORD addr;

	b = bb_queue_pick_first(results);
	memcpy(&addr, b->data, sizeof(DWORD));

	if (NOT_A_POINTER(addr))
		return false;

	DWORD ptr[2];
	DWORD offset[2] = {0xC4, 0xDC};

	ptr[0] = read_memory_as_int(mp->proc, addr);

	if (NOT_A_POINTER(ptr[0]))
		return false;

	ptr[1] = read_memory_as_int(mp->proc, ptr[0] + offset[0]);

	if (NOT_A_POINTER(ptr[1]))
		return false;

	addr = ptr[1] + offset[1];

	int left = read_memory_as_int(mp->proc, addr);
	if (left == 0 || left > 10000)
		return false;

	int top = read_memory_as_int(mp->proc, addr + 4);
	if (top == 0 || top > 10000)
		return false;

	int right = read_memory_as_int(mp->proc, addr + 4);
	if (right == 0 || right > 10000)
		return false;

	int bot = read_memory_as_int(mp->proc, addr + 4);
	if (bot == 0 || bot > 10000)
		return false;

	return true;
}

bool camera_scan_minimap_size (void)
{
	Camera *this = camera_get_instance();
	unsigned char *description = "minimapSize";

	BbQueue *res = memscan_search_cond (this->mp, description,
	/*
        0168CC87  ║·  E8 44751000   call League_of_Legends.017941D0                      ; └League_of_Legends.017941D0
        0168CC8C  ║►  E8 EF6DFEFF   call League_of_Legends.01673A80                      ; [League_of_Legends.01673A80
        0168CC91  ║·  8B0D 3CD04102 [3] mov ecx, [dword ds:League_of_Legends.241D03C]
        0168CC97  ║·  85C9          test ecx, ecx
        0168CC99  ║·▼ 74 05         jz short League_of_Legends.0168CCA0
        0168CC9B  ║·  E8 C093F5FF   call League_of_Legends.015E6060                      ; [League_of_Legends.015E6060
        0168CCA0  ║►  E8 7BB3E4FF   call League_of_Legends.014D8020                      ; [League_of_Legends.014D8020
	*/
		(unsigned char []) {
		    0xE8,0x44,0x75,0x10,0x00,
		    0xE8,0xEF,0x6D,0xFE,0xFF,
		    0x8B,0x0D,0x3C,0xD0,0x41,0x02,
		    0x85,0xC9,
            0x74,0x05,
            0xE8,0xC0,0x93,0xF5,0xFF,
            0xE8,0x7B,0xB3,0xE4,0xFF
		},
		"x????"
        "x????"
        "xx????"
        "xx"
        "x?"
        "x????"
        "x????",

        "xxxxx"
        "xxxxx"
        "xx????"
        "xx"
        "xx"
        "xxxxx"
        "xxxxx",

        camera_minimap_size_cond
	);

	if (!res)
	{
		warning("Cannot find %s address", description);
		return false;
	}

	Buffer *minimapSize = bb_queue_pick_first(res);

	memcpy(&this->mmsize_addr, minimapSize->data, minimapSize->size);

    // Cleaning
    bb_queue_free_all(res, buffer_free);

	// TODO : Get offsets properly
	DWORD ptr[2];
	DWORD offset[2] = {0xC4, 0xDC};

    // Get the final value
	ptr[0] = read_memory_as_int(this->mp->proc, this->mmsize_addr);
	ptr[1] = read_memory_as_int(this->mp->proc, ptr[0] + offset[0]);
	this->mmsize_addr = ptr[1] + offset[1];

	if (NOT_A_POINTER(this->mmsize_addr)) {
		warning("Cannot scan %s array", description);
	}

    // Get window position
    RECT wrect;
    GetWindowRect(this->mp->hwnd, &wrect);

    // Get minimap information
	this->minimap.xLeft  = wrect.left          + read_memory_as_int(this->mp->proc, this->mmsize_addr);
	this->minimap.yTop   = wrect.top           + read_memory_as_int(this->mp->proc, this->mmsize_addr + 4);
	this->minimap.xRight = this->minimap.xLeft + read_memory_as_int(this->mp->proc, this->mmsize_addr + 8);
	this->minimap.yBot   = this->minimap.yTop  + read_memory_as_int(this->mp->proc, this->mmsize_addr + 12);

	debug("Minimap Size = LT = [%d:%d] / RB = [%d:%d]",
		this->minimap.xLeft, this->minimap.yTop,
		this->minimap.xRight, this->minimap.yBot);

	return true;
}

bool camera_scan_ping_or_skill_waiting (void)
{
	Camera *this = camera_get_instance();
	unsigned char * description = "pingWaiting";

	BbQueue *res = memscan_search (this->mp, description,
    /*
        0145F1E9   ·  E8 F20D1F00   call League_of_Legends.0164FFE0
        0145F1EE   ·  A1 20CC2D02   mov eax, [dword ds:League_of_Legends.22DCC20]
        0145F1F3   ·  A3 08CC2D02   mov [dword ds:League_of_Legends.22DCC08], eax
    */

		(unsigned char []) {
            0xE8,0xF2,0x0D,0x1F,0x00,
            0xA1,0x20,0xCC,0x2D,0x02,
            0xA3,0x08,0xCC,0x2D,0x02
		},

        "x????"
        "x????"
        "x????",

        "xxxxx"
        "xxxxx"
        "x????"
    );

	if (!res)
	{
		warning("Cannot find %s", description);
		return false;
	}

	Buffer *pingWaiting = bb_queue_pick_first(res);
	memcpy(&this->ping_state_addr, pingWaiting->data, pingWaiting->size);

	if (!this->ping_state_addr)
	{
		warning("Cannot scan %s", description);
		return false;
	}

	return true;
}

bool camera_scan_victory (void)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "VictoryState",
	/*
		00AB2B67   ·  56                                   push esi
		00AB2B68   ·▼ 75 0F                                jne short League_of_Legends.00AB2B79
		00AB2B6A   ·  833D 20A1CA01 00                     cmp [dword ds:League_of_Legends.1CAA120], 0
		00AB2B71   ·▼ 75 06                                jne short League_of_Legends.00AB2B79
		00AB2B73   ·  5E                                   pop esi
		00AB2B74   ·▲ E9 9755F6FF                          jmp League_of_Legends.00A18110
		00AB2B79   ►  A1 <<08A1CA01>>                      mov eax, [dword ds:League_of_Legends.1CAA108]
		00AB2B7E   ·  83F8 02                              cmp eax, 2
		00AB2B81   ·▼ 74 04                                je short League_of_Legends.00AB2B87
		00AB2B83   ·  85C0                                 test eax, eax
	*/
		(unsigned char []) {
			0x56,
			0x75,0x0F,
			0x83,0x3D,0x20,0xA1,0xCA,0x01,0x00,
			0x75,0x06,
			0x5E,
			0xE9,0x97,0x55,0xF6,0xFF,
			0xA1,0x08,0xA1,0xCA,0x01,
			0x83,0xF8,0x02,
			0x74,0x04,
			0x85,0xC0
		},

		"x"
		"xx"
		"xx????x"
		"xx"
		"x"
		"x????"
		"x????"
		"xxx"
		"xx"
		"xx",

		"x"
		"xx"
		"xxxxxxx"
		"xx"
		"x"
		"xxxxx"
		"x????"
		"xxx"
		"xx"
		"xx"
	);

	if (!res)
	{
		warning("Cannot find VictoryState address");
		return false;
	}

	Buffer *victory_state_addr = bb_queue_pick_first(res);
	memcpy(&this->victory_state_addr, victory_state_addr->data, victory_state_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->victory_state_addr)
	{
		warning("Cannot scan victory_state_addr");
		return false;
	}

	int victory_state = read_memory_as_int(this->mp->proc, this->victory_state_addr);

	if (victory_state != 0)
	{
		this->victory_state = victory_state;
	}

	return (victory_state != 0);
}

bool camera_cond_champions (MemProc *mp, BbQueue *results)
{
	Buffer *b[2];
	DWORD ptr[2];

	b[0] = bb_queue_pick_first(results);
	b[1] = bb_queue_pick_last(results);

	memcpy(&ptr[0], b[0]->data, sizeof(DWORD));
	memcpy(&ptr[1], b[1]->data, sizeof(DWORD));

	// Address end = address start + 4
	if (!(ptr[0] + 4 == ptr[1]))
	{
		debug("camera_cond_champions : Condition error : 1");
		return false;
	}

	if (NOT_A_POINTER(ptr[0]))
	{
		debug("camera_cond_champions : Condition error : 2");
		return false;
	}

	ptr[0] = read_memory_as_int(mp->proc, ptr[0]);
	ptr[1] = read_memory_as_int(mp->proc, ptr[1]);

	if (NOT_A_POINTER(ptr[0]) || NOT_A_POINTER(ptr[1]))
	{
		debug("camera_cond_champions : Condition error : 3");
	}

	if ((ptr[1] - ptr[0]) / 4 > 12) // Support 6v6
	{
		debug("camera_cond_champions : Condition error : 4");
		return false;
	}

	if (ptr[0] == (DWORD) -1)
	{
		debug("camera_cond_champions : Condition error : 5");
		return false;
	}

	Entity *dummy = entity_new(mp, ptr[0]);

	if (dummy == NULL)
	{
		debug("camera_cond_champions : Condition error : 6");
		return false;
	}

	if (str_pos(dummy->player_name, "Turret_") != -1)
	{
		debug("camera_cond_champions : Condition error : 7");
		return false;
	}

	if (str_pos(dummy->player_name, "Barracks_") != -1)
	{
		debug("camera_cond_champions : Condition error : 8");
		return false;
	}

	if (dummy->movement_speed <= 0) // Buildings ms = 0
	{
		debug("camera_cond_champions : Condition error : 9");
		return false;
	}

	entity_free(dummy);

	return true;
}

bool camera_scan_champions (bool display_error)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search_cond (this->mp, "entityArrayStart/entityArrayEnd",
	/*
		013EE469   ║► ┌8B35 9C9CD203      ╓mov esi, [dword ds:League_of_Legends.3D29C9C]
		013EE46F   ║· │8B3D A09CD203      ║mov edi, [dword ds:League_of_Legends.3D29CA0]
	*/
		(unsigned char[]) {
			0x8B,0x35,0x9C,0x9C,0xD2,0x03,
			0x8B,0x3D,0xA0,0x9C,0xD2,0x03
		},

		"xx????"
		"xx????",

		"xx????"
		"xx????",

		camera_cond_champions
	);

	if (!res)
	{
		if (display_error)
			warning("Cannot find entities array address");
		return false;
	}

	Buffer *eArrStart = bb_queue_pick_first(res),
		   *eArrEnd   = bb_queue_pick_last(res);

	memcpy(&this->entities_addr, eArrStart->data, eArrStart->size);
	memcpy(&this->entities_addr_end, eArrEnd->data, eArrEnd->size);
	if (!this->entities_addr || !this->entities_addr_end) {
		warning("Cannot find entities 1");
		return false;
	}

	this->entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	this->entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);
	if (!this->entity_ptr || !this->entity_ptr_end) {
		warning("Cannot find entities 2");
		return false;
	}

	if (entity_address_to_array(this->mp, this->entity_ptr, this->entity_ptr_end, this->champions))
		camera_refresh_self();

	return true;
}


/** Refreshers **/
bool camera_refresh_victory (void)
{
	Camera *this = camera_get_instance();

	this->victory_state = read_memory_as_int(this->mp->proc, this->victory_state_addr);

	return true;
}

bool camera_refresh_mouse_screen (void)
{
	Camera *this = camera_get_instance();
    return GetCursorPos(&this->mouse_screen);
}

bool camera_refresh_ping_state (void)
{
    Camera *this = camera_get_instance();
    this->ping_state = read_memory_as_int(this->mp->proc, this->ping_state_addr);
    return true;
}

bool camera_refresh_champions (void)
{
	Camera *this = camera_get_instance();

	DWORD entity_ptr     = this->entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	DWORD entity_ptr_end = this->entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);

	this->playersCount = (entity_ptr_end - entity_ptr) / 4;

	for (int i = 0; entity_ptr != entity_ptr_end && i < 10; entity_ptr+=4, i++)
	{
		if (!entity_refresh(this->champions[i]))
		{
			if (this->champions[i])
			{
				warning("Entity 0x%.8x cannot be refreshed", this->champions[i]->entity_data);
				return false;
			}
		}
	}

	return true;
}

bool camera_refresh_entity_hovered (void)
{
	Camera *this = camera_get_instance();

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = this->champions[i];

		if (e->isHovered)
		{
			if (strcmp(e->player_name, this->self_name) != 0)
			{
				if (vector2D_distance(&this->cam->v, &e->p.v) < 3000.0)
				{
					this->entity_hovered = e;
					return true;
				}
			}
		}
	}

	this->entity_hovered = NULL;

	return true;
}

bool camera_refresh_entities_nearby (void)
{
	Camera *this = camera_get_instance();
	float in_screen = this->distance_entity_nearby;

	float distance;
	int index = 0;
	int index_allies = 0;
	int index_ennemies = 0;

	memset(this->nearby, 0, sizeof(this->nearby));
	memset(this->nearby_allies, 0, sizeof(this->nearby_allies));
	memset(this->nearby_ennemies, 0, sizeof(this->nearby_ennemies));

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	this->nb_allies_nearby = 0;
	this->nb_ennemies_nearby = 0;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = this->champions[i];
		distance = vector2D_distance(&e->p.v, &this->cam->v);

		if (
			(distance < in_screen)
		&&  (entity_is_visible(e))
		&&  (e != this->self)
		)
		{
			this->nearby[index++] = e;

			if (e->team == this->self->team)
			{
				this->nearby_allies[index_allies++] = e;
				this->nb_allies_nearby = index_allies;
			}
			else
			{
				this->nearby_ennemies[index_ennemies++] = e;
				this->nb_ennemies_nearby = index_ennemies;
			}
		}
	}

	return true;
}

bool camera_refresh_win_is_opened (void)
{
	Camera *this = camera_get_instance();

	this->interface_opened = read_memory_as_int(this->mp->proc, this->win_is_opened_addr);
	// Shop opened : 4
	// Chat opened : 2
	// Nothing     : 1

	return this->interface_opened != 0;
}

bool camera_refresh_hover_interface (void)
{
	Camera *this = camera_get_instance();
	this->interface_hovered = read_memory_as_int(this->mp->proc, this->interface_hovered_addr);

	return (this->interface_hovered == 1 || this->interface_hovered == 0);
}

void camera_save_patch (long int id, DWORD *addr, unsigned char *code, int len)
{
	char patch_path[100];

	sprintf(patch_path, "./patches/patch_%ld", id);
	FILE *patch_file = fopen(patch_path, "wb+");

	// 1st try : the folder maybe doesn't exist
	if (!patch_file)
	{
		// Maybe patches folder doesn't exit
		_mkdir("patches");
		patch_file = fopen(patch_path, "w+");
	}

	// 2nd try
	if (!patch_file)
	{
		warning("Cannot save patches. Please exit with X ONLY or it may crashes the client.");
		return;
	}

	fwrite(addr, sizeof(DWORD), 1, patch_file);
	fwrite(&len, sizeof(int), 1, patch_file);
	fwrite(code, sizeof(char), len, patch_file);

	fclose(patch_file);
}

unsigned char *camera_read_patch (long int id, int *len, DWORD *addr)
{
	char patch_path[100];
	sprintf(patch_path, "./patches/patch_%ld", id);
	FILE *patch_file = fopen(patch_path, "rb");

	// 1st try : the folder maybe doesn't exist
	if (!patch_file)
	{
		return NULL;
	}

	fread(addr, sizeof(DWORD), 1, patch_file);
	fread(len, sizeof(int), 1, patch_file);

	char *code = malloc(*len);
	fread(code, sizeof(char), *len, patch_file);

	return code;
}

// ------------ Scanners ------------
static unsigned long int patch_id = 0;

static Patch *camera_get_patch (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask)
{
	unsigned char *code = NULL;
	int len;

	// Get the address of the signature
	if (!camera_search_signature (sig, addr, &code, sig_mask, description))
	{
		// Error : Try to restore the former saved patches
		code = camera_read_patch(patch_id, &len, addr);

		if ((code != NULL)
		&&  (write_to_memory(mp->proc, code, *addr, len)))
		{
				// Phew ... it worked
				info("Patch %d restored", patch_id);
		}

		else
		{
			important("Cannot load patch %d... Please restart the League of Legends client. Sorry :(", patch_id);
			return NULL;
		}
	}

	// Save the patch
	else
		camera_save_patch(patch_id, addr, code, strlen(patch_mask));

	patch_id++;

	// Create a new patch
	return patch_new (description, mp, *addr, code, sig, patch, patch_mask);
}

static bool camera_search_signature (unsigned char *pattern, DWORD *addr, unsigned char **code_ptr, char *mask, char *name)
{
	Camera *this = camera_get_instance();
	debug("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0)
	{
		warning("\"%s\" not found (already patched ?)", name);
		return false;
	}

	if (bb_queue_get_length(results) > 1)
	{
		printf("\n");
		warning("Multiple occurences of \"%s\" found (%d found) :", name, bb_queue_get_length(results));

		foreach_bbqueue_item (results, memblock) {
			debugb(" -> 0x%.8x\n", (int) memblock->addr);
		}
	}

	memblock = bb_queue_pick_first(results);
	*addr = memblock->addr;
	*code_ptr = malloc(memblock->size);
	memcpy(*code_ptr, memblock->data, memblock->size);

	debug(" -> 0x%.8x", (int) memblock->addr);

	bb_queue_free_all(results, memblock_free);

	return true;
}

void camera_export_to_cheatengine (void)
{
	#ifdef DEBUG
		Camera *this = camera_get_instance();

		if (!this)
			return;

		char *out = str_dup_printf (
		"<?xml version=\"1.0\"?>\n"
		"<CheatTable CheatEngineTableVersion=\"12\">\n"
		"  <CheatEntries>\n"
		"    <CheatEntry>\n"
		"      <ID>0</ID>\n"
		"      <Description>\"CameraX_Memory\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>60</ID>\n"
		"      <Description>\"CameraY_Memory\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>666</ID>\n"
		"      <Description>\"CameraX_Value\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>667</ID>\n"
		"      <Description>\"CameraY_Value\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>3</ID>\n"
		"      <Description>\"ChampX\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>9</ID>\n"
		"      <Description>\"ChampY\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>5</ID>\n"
		"      <Description>\"MouseX\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>6</ID>\n"
		"      <Description>\"MouseY\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>7</ID>\n"
		"      <Description>\"DestX\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>10</ID>\n"
		"      <Description>\"DestY\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>19</ID>\n"
		"      <Description>\"EntityPointed\"</Description>\n"
		"      <ShowAsHex>1</ShowAsHex>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Array of byte</VariableType>\n"
		"      <ByteLength>10</ByteLength>\n"
		"      <Address>%.8x</Address>\n"
		"      <Offsets>\n"
		"        <Offset>0</Offset>\n"
		"      </Offsets>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>21</ID>\n"
		"      <Description>\"WindowOpened\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Byte</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"      <Offsets>\n"
		"        <Offset>2C</Offset>\n"
		"      </Offsets>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>34</ID>\n"
		"      <Description>\"GameEntitiesArray\"</Description>\n"
		"      <ShowAsHex>1</ShowAsHex>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Array of byte</VariableType>\n"
		"      <ByteLength>24</ByteLength>\n"
		"      <Address>%.8x</Address>\n"
		"      <CheatEntries>\n"
		"        <CheatEntry>\n"
		"          <ID>35</ID>\n"
		"          <Description>\"EntityF1\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>0</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>290</ID>\n"
		"              <Description>\"isHovered\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Byte</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>231</Offset>\n"
		"                <Offset>0</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>29</ID>\n"
		"              <Description>\"HP\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>118</Offset>\n"
		"                <Offset>0</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>46</ID>\n"
		"              <Description>\"HPmax\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>128</Offset>\n"
		"                <Offset>0</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>45</ID>\n"
		"              <Description>\"posX\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>64</Offset>\n"
		"                <Offset>0</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>54</ID>\n"
		"              <Description>\"posY\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>6c</Offset>\n"
		"                <Offset>0</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>1154</ID>\n"
		"              <Description>\"isVisible\"</Description>\n"
		"              <ShowAsHex>1</ShowAsHex>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>4 Bytes</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>24</Offset>\n"
		"                <Offset>154</Offset>\n"
		"                <Offset>0</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"          </CheatEntries>\n"
		"        </CheatEntry>\n"
		"        <CheatEntry>\n"
		"          <ID>36</ID>\n"
		"          <Description>\"EntityF2\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>4</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>300</ID>\n"
		"              <Description>\"isHovered\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Byte</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>231</Offset>\n"
		"                <Offset>4</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>30</ID>\n"
		"              <Description>\"HP\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>118</Offset>\n"
		"                <Offset>4</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>40</ID>\n"
		"              <Description>\"HPmax\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>128</Offset>\n"
		"                <Offset>4</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>48</ID>\n"
		"              <Description>\"posY\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>6c</Offset>\n"
		"                <Offset>4</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>47</ID>\n"
		"              <Description>\"posX\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>64</Offset>\n"
		"                <Offset>4</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>1155</ID>\n"
		"              <Description>\"isVisible\"</Description>\n"
		"              <ShowAsHex>1</ShowAsHex>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>4 Bytes</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>24</Offset>\n"
		"                <Offset>154</Offset>\n"
		"                <Offset>4</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"          </CheatEntries>\n"
		"        </CheatEntry>\n"
		"        <CheatEntry>\n"
		"          <ID>37</ID>\n"
		"          <Description>\"EntityF3\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>8</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>490</ID>\n"
		"              <Description>\"isHovered\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Byte</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>231</Offset>\n"
		"                <Offset>8</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>49</ID>\n"
		"              <Description>\"HP\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>118</Offset>\n"
		"                <Offset>8</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>50</ID>\n"
		"              <Description>\"HPmax\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>128</Offset>\n"
		"                <Offset>8</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>31</ID>\n"
		"              <Description>\"posX\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>64</Offset>\n"
		"                <Offset>8</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>41</ID>\n"
		"              <Description>\"posY\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>6c</Offset>\n"
		"                <Offset>8</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>1156</ID>\n"
		"              <Description>\"isVisible\"</Description>\n"
		"              <ShowAsHex>1</ShowAsHex>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>4 Bytes</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>24</Offset>\n"
		"                <Offset>154</Offset>\n"
		"                <Offset>8</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"          </CheatEntries>\n"
		"        </CheatEntry>\n"
		"        <CheatEntry>\n"
		"          <ID>38</ID>\n"
		"          <Description>\"EntityF4\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>C</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>290</ID>\n"
		"              <Description>\"isHovered\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Byte</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>231</Offset>\n"
		"                <Offset>C</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>51</ID>\n"
		"              <Description>\"HP\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>118</Offset>\n"
		"                <Offset>C</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>52</ID>\n"
		"              <Description>\"HPmax\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>128</Offset>\n"
		"                <Offset>C</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>32</ID>\n"
		"              <Description>\"posX\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>64</Offset>\n"
		"                <Offset>C</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>42</ID>\n"
		"              <Description>\"posY\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>6c</Offset>\n"
		"                <Offset>C</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>1157</ID>\n"
		"              <Description>\"isVisible\"</Description>\n"
		"              <ShowAsHex>1</ShowAsHex>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>4 Bytes</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>24</Offset>\n"
		"                <Offset>154</Offset>\n"
		"                <Offset>C</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"          </CheatEntries>\n"
		"        </CheatEntry>\n"
		"        <CheatEntry>\n"
		"          <ID>39</ID>\n"
		"          <Description>\"EntityF5\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>10</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>530</ID>\n"
		"              <Description>\"isHovered\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Byte</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>231</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>53</ID>\n"
		"              <Description>\"HP\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>118</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>54</ID>\n"
		"              <Description>\"HPmax\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>128</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>33</ID>\n"
		"              <Description>\"posX\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>64</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>43</ID>\n"
		"              <Description>\"posX\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>6c</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>1157</ID>\n"
		"              <Description>\"isVisible\"</Description>\n"
		"              <ShowAsHex>1</ShowAsHex>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>4 Bytes</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>24</Offset>\n"
		"                <Offset>154</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"          </CheatEntries>\n"
		"        </CheatEntry>\n"
		"      </CheatEntries>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>55</ID>\n"
		"      <Description>\"GameState\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>4 Bytes</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"  </CheatEntries>\n"
		"  <UserdefinedSymbols/>\n"
		"</CheatTable>\n",

		(this->camPos) ? this->camPos->addrX : 0,
		(this->camPos) ? this->camPos->addrY : 0,
		(this->cam) ? this->cam->addrX : 0,
		(this->cam) ? this->cam ->addrY : 0,
		(this->champ) ? this->champ->addrX : 0,
		(this->champ) ? this->champ->addrY : 0,
		(this->mouse) ? this->mouse->addrX : 0,
		(this->mouse) ? this->mouse->addrY : 0,
		(this->dest) ? this->dest->addrX : 0,
		(this->dest) ? this->dest->addrY : 0,
		(this->entity_hovered_addr) ? this->entity_hovered_addr : 0,
		(this->win_is_opened_ptr) ? this->win_is_opened_ptr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->entities_addr : 0,
		(this->entities_addr) ? this->game_info_addr : 0
		);

		file_put_contents("out.ct", out, NULL);

	#endif
}
