#include "LoLCamera.h"

static BOOL 	 camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size, BbQueue *addresses);
static BOOL      camera_search_signature  (unsigned char *pattern, DWORD *addr, unsigned char **code_ptr, char *mask, char *name);
static Patch *   camera_get_patch         (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);
void     		 camera_get_patches       (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask);

BOOL camera_scan_patch ()
/// FIXED
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
			01011E07   ·  F30F1005 189C4103       movss xmm0, [dword ds:League_of_Legends.3419C18]      ; float 50.00000
			01011E0F   ·  F30F1146 2C             movss [dword ds:esi+2C], xmm0
			01011E14   ·  F30F1116                movss [dword ds:esi], xmm2
			01011E18   ·  F30F1166 04             movss [dword ds:esi+4], xmm4
			01011E1D   ·  F30F115E 08             movss [dword ds:esi+8], xmm3
			016A6A15      F30F114D D0        	  movss [dword ss:ebp-30], xmm1
			01011E28   ·  F30F5CF3                subss xmm6, xmm3
		*/
			0xF3,0x0F,0x10,0x05,0x18,0x9C,0x41,0x03,
			0xF3,0x0F,0x11,0x46,0x2C,
			0xF3,0x0F,0x11,0x16,
			0xF3,0x0F,0x11,0x66,0x04,
			0xF3,0x0F,0x11,0x5E,0x08,
			0xF3,0x0F,0x11,0x4D,0xD0,
			0xF3,0x0F,0x5C,0xF3
		},

			"xxxx????"
			"xxxxx"
			"xxx?" // << nop
			"xxx?x"
			"xxx?x" // << nop
			"xxxxx"
			"xxxx",

		(unsigned char[]) {
			0xF3,0x0F,0x10,0x05,0x18,0x9C,0x41,0x03,
			0xF3,0x0F,0x11,0x46,0x2C,
			0x90,0x90,0x90,0x90,
			0xF3,0x0F,0x11,0x66,0x04,
			0x90,0x90,0x90,0x90,0x90,
			0xF3,0x0F,0x11,0x4D,0xD0,
			0xF3,0x0F,0x5C,0xF3
		},
			"????????"
			"?????"
			"xxxx"
			"?????"
			"xxxxx"
			"?????"
			"????"
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

			"xxxx????"
			"xxxx????"
			"xxxx????"
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
				0107FE4E  ║·▼┌74 49                   jz short League_of_Legends.0107FE99
				0107FE50  ║· │85FF                    test edi, edi
				0107FE52  ║·▼│74 45                   jz short League_of_Legends.0107FE99
				0107FE54  ║· │803D CC9B4803 00        cmp [byte ds:League_of_Legends.3489BCC], 0
				0107FE5B  ║·▼│0F84 7D020000           je League_of_Legends.010800DE
				0107FE61  ║► │F30F1047 64             movss xmm0, [dword ds:edi+64]                        ; float 0.0004092904
				0107FE66  ║· │F30F1105 27994803       movss [dword ds:League_of_Legends.3489927], xmm0     ; float 0.0, 0.0, 0.0, 3036.157
				0107FE6E  ║· │F30F1047 68             movss xmm0, [dword ds:edi+68]                        ; float 0.0003991595
				0107FE73  ║· │F30F1105 2B994803       movss [dword ds:League_of_Legends.348992B], xmm0     ; float 0.0, 0.0, 0.0, 3036.157
				0107FE7B  ║· │F30F1047 6C             movss xmm0, [dword ds:edi+6C]                        ; float 0.0004054068
				0107FE80  ║· │F30F1105 2F994803       movss [dword ds:League_of_Legends.348992F], xmm0     ; float 0.0, 0.0, 0.0, 3036.157
				01698515  ║· │8B4D F4                 mov ecx, [dword ss:ebp-0C]
				0107FE8C  ║· │64:890D 00000000        mov [dword fs:0], ecx                                ; ASCII "H\xF4+"
		*/
			0x74,0x49,
			0x85,0xFF,
			0x74,0x45,
			0x80,0x3D,0xCC,0x9B,0x48,0x03,0x00,
			0x0F,0x84,0x7D,0x02,0x00,0x00,
			0xF3,0x0F,0x10,0x47,0x64,
			0xF3,0x0F,0x11,0x05,0x27,0x99,0x48,0x03,
			0xF3,0x0F,0x10,0x47,0x68,
			0xF3,0x0F,0x11,0x05,0x2B,0x99,0x48,0x03,
			0xF3,0x0F,0x10,0x47,0x6C,
			0xF3,0x0F,0x11,0x05,0x2F,0x99,0x48,0x03,
			0x8B,0x4D,0xF4,
			0x64,0x89,0x0D,0x00,0x00,0x00,0x00
		},
			"xx"
			"xx"
			"xx"
			"xx????x"
			"xx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xxxx????"
			"xx?"
			"xxxxxxx",

		(unsigned char[]) {
			0x74,0x49,
			0x85,0xFF,
			0x74,0x45,
			0x80,0x3D,0xCC,0x9B,0x48,0x03,0x00,
			0x0F,0x84,0x7D,0x02,0x00,0x00,
			0xF3,0x0F,0x10,0x47,0x64,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0xF3,0x0F,0x10,0x47,0x68,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0xF3,0x0F,0x10,0x47,0x6C,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x8B,0x4D,0xF4,
			0x64,0x89,0x0D,0x00,0x00,0x00,0x00
		},
			"??"
			"??"
			"??"
			"???????"
			"??????"
			"?????"
			"xxxxxxxx"
			"?????"
			"xxxxxxxx"
			"?????"
			"xxxxxxxx"
			"???"
			"???????"

	);

	this->patchlist = patch_list_get();

	return TRUE;
}

BOOL camera_scan_camval ()
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
		warning("Cannot find CameraX/CameraY address");
		return FALSE;
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
		return FALSE;
	}

	this->camx_val = camx_addr_ptr - this->mp->base_addr;
	this->camy_val = camy_addr_ptr - this->mp->base_addr;

	return TRUE;
}

BOOL camera_scan_campos ()
/// FIXED
{
	Camera *this = camera_get_instance();
	unsigned char *description = "CameraX_Memory/CameraY_Memory";

	BbQueue *res = memscan_search(this->mp, description,
		(unsigned char []) {
		/*
			014636B3       ║·  84C0                    test al, al
			014636B5       ║·▼ 74 34                   jz short League_of_Legends.014636EB
			014636B7       ║·  F30F1045 00             movss xmm0, [dword ss:ebp]
			014636BC       ║·  F30F1105 27999503       movss [dword ds:League_of_Legends.3959927], xmm0       ; float 11754.59 <- cameraX
			014636C4       ║·  F30F1045 04             movss xmm0, [dword ss:ebp+4]
			014636C9       ║·  F30F1105 2B999503       movss [dword ds:League_of_Legends.395992B], xmm0       ; float 0.0
			014636D1       ║·  F30F1045 08             movss xmm0, [dword ss:ebp+8]
			014636D6       ║·  8BCB                    mov ecx, ebx
			014636D8       ║·  F30F1105 2F999503       movss [dword ds:League_of_Legends.395992F], xmm0       ; float 9593.056  <- cameraY
			014636E0       ║·  E8 DBF8FFFF             call League_of_Legends.01462FC0                        ; [League_of_Legends.01462FC0
			014636E5       ║·  5F                      pop edi
			014636E6       ║·  5D                      pop ebp
			014636E7       ║·  5B                      pop ebx
			014636E8       ║·  C2 1000                 retn 10
		*/
			0x85,0xC9,
			0x74,0x24,
			0x8B,0x01,
			0x68,0x00,0x01,0x00,0x00,
			0x68,0x78,0xCE,0xE4,0x00,
			0xFF,0x50,0x1C
		},

		"xx"
		"xx"
		"xx"
		"xxxxx"
		"x????"
		"xxx",

		"xx"
		"xx"
		"xx"
		"xxxxx"
		"x????"
		"xxx"
	);

	if (!res)
	{
		warning("Cannot find CameraX/CameraY address");
		return FALSE;
	}

	DWORD camx_addr_ptr, camy_addr_ptr;
	Buffer *cameraX = bb_queue_pick_first(res);
	memcpy(&camx_addr_ptr, cameraX->data, sizeof(DWORD));
	camy_addr_ptr = camx_addr_ptr + 8;

	bb_queue_free_all(res, buffer_free);

	if (!camx_addr_ptr || !camy_addr_ptr)
	{
		warning("Cannot find camera position");
		return FALSE;
	}

	this->camx_addr  = camx_addr_ptr  - this->mp->base_addr;
	this->camy_addr  = camy_addr_ptr  - this->mp->base_addr;

	return TRUE;
}

BOOL camera_scan_hover_interface ()
/// FIXED
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
		return FALSE;
	}

	Buffer *interface_hovered_addr = bb_queue_pick_first(res);
	memcpy(&this->interface_hovered_addr, interface_hovered_addr->data, interface_hovered_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->interface_hovered_addr)
	{
		warning("Cannot scan HoverInterface");
		return FALSE;
	}

	camera_refresh_hover_interface();

	return TRUE;
}

BOOL camera_scan_loading ()
/// FIXED
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "loadingState",
	/*
			000CE371  ║·  8B35 40D1B002           mov esi, [dword ds:League_of_Legends.2B0D140]
			000CE377  ║·  85F6                    test esi, esi
			000CE379  ║·▼ 74 38                   jz short League_of_Legends.000CE3B3
			000CE37B  ║·  8B06                    mov eax, [dword ds:esi]
			000CE37D  ║·  85C0                    test eax, eax
			000CE37F  ║·▼ 74 28                   jz short League_of_Legends.000CE3A9
			000CE381  ║·  FF76 04                 push [dword ds:esi+4]                                  ; ╓Arg2
			000CE384  ║·  50                      push eax                                               ; ║Arg1 = ASCII "themewnd"
			000CE385  ║·  E8 36F2FAFF             call League_of_Legends.0007D5C0                        ; └League_of_Legends.0007D5C0
			000CE38A  ║·  FF36                    push [dword ds:esi]
			000CE38C  ║·  FF15 5CB6B800           call [dword ds:<&MSVCR110.operator delete>]
	*/
		(unsigned char[]) {
			0x8B,0x35,0x40,0xD1,0xB0,0x02,
			0x85,0xF6,
			0x74,0x38,
			0x8B,0x06,
			0x85,0xC0,
			0x74,0x28,
			0xFF,0x76,0x04,
			0x50,
			0xE8,0x36,0xF2,0xFA,0xFF,
			0xFF,0x36,
			0xFF,0x15,0x5C,0xB6,0xB8,0x00
		},

			"xx????"
			"xx"
			"x?"
			"xx",

			"xx????"
			"xx"
			"xx"
			"xx"


	);

	if (!res)
	{
		warning("Cannot find loading state address");
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

BOOL camera_scan_game_info ()
/// SEMI FIXED
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "gameState",
	/*
CPU Disasm
Address    Hex dump               Command                                           Comments
017F9295   ║·  57                 push edi                                          ; ╓hDC = 005B1F20
017F9296   ║·  8945 CC            mov [dword ss:local.13], eax                      ; ║
017F9299   ║·  FF15 88D0ED01      call [dword ds:<&GDI32.DeleteDC>]                 ; └GDI32.DeleteDC
017F929F   ║·  8B75 C4            mov esi, [dword ss:local.15]
017F92A2   ║·  8B7D CC            mov edi, [dword ss:local.13]                      ; ASCII "SUSU"
017F92A5   ║►  8B0D 64E0D603      mov ecx, [dword ds:League_of_Legends.3D6E064]
017F92AB   ║·  85C9               test ecx, ecx
017F92AD   ║·▼ 75 0B              jnz short League_of_Legends.017F92BA



	*/
		(unsigned char[]) {
			0x57,
			0x89,0x45,0xCC,
			0xFF,0x15,0x88,0xD0,0xED,0x01,
			0x8B,0x75,0xC4,
			0x8B,0x7D,0xCC,
			0x8B,0x0D,0x64,0xE0,0xD6,0x03,
			0x85,0xC9,
			0x75,0x0B
		},
			"x"
			"xx?"
			"xx????"
			"xx?"
			"xx?"
			"xx????"
			"xx"
			"xx",

			"x"
			"xxx"
			"xxxxxx"
			"xxx"
			"xxx"
			"xx????"
			"xx"
			"xx"
	);

	if (!res)
	{
		warning("Cannot find game state address");
		return 0;
	}

	Buffer *game_info_addr = bb_queue_pick_nth(res, 1);
	memcpy(&this->game_info_addr, game_info_addr->data, game_info_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->game_info_addr)
	{
		warning("Cannot scan game state");
		return FALSE;
	}

	read_from_memory(this->mp->proc, this->self_name, this->game_info_addr + 0x84, sizeof(this->self_name) - 1);

	if (strlen(this->self_name) <= 0)
	{
		warning("Cannot find self name");
		return FALSE;
	}

	else
		debug("Self name : <%s>", this->self_name);

	return TRUE;
}

BOOL camera_scan_cursor_champ ()
/// FIXED
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
		return FALSE;
	}

	Buffer *cursorX = bb_queue_pick_first(res);
	Buffer *champX  = bb_queue_pick_last(res);

	memcpy(&this->mousex_addr, cursorX->data, cursorX->size);
	memcpy(&this->champx_addr, champX->data,  champX->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->mousex_addr || !this->champx_addr)
	{
		warning("Cannot scan mouse or champion position");
		return FALSE;
	}

	this->champx_addr -= this->mp->base_addr;
	this->champy_addr  = this->champx_addr + 8;

	this->mousex_addr -= this->mp->base_addr;
	this->mousey_addr  = this->mousex_addr + 8;

	return TRUE;
}

BOOL camera_scan_dest ()
/// FIXED
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search(this->mp, "DestPos",
		/*
			0107EFDF  ║·  D91D F89A4803                    fstp [dword ds:League_of_Legends.3489AF8]                        ; float 3.000000
			0107EFE5  ║·  C605 FC9A4803 01                 mov [byte ds:League_of_Legends.3489AFC], 1
			0107EFEC  ║·  C705 009B4803 00000000           mov [dword ds:League_of_Legends.3489B00], 0
			0107EFF6  ║·  C705 289B4803 000000FF           mov [dword ds:League_of_Legends.3489B28], FF000000
			0107F000  ║·  C705 249B4803 00000000           mov [dword ds:League_of_Legends.3489B24], 0
			0107F00A  ║·  C705 2C9B4803 FFFFFFFF           mov [dword ds:League_of_Legends.3489B2C], -1
			0107F014  ║·  C705 309B4803 00000000           mov [dword ds:League_of_Legends.<<<3489B30>>>], 0
		*/
		(unsigned char []) {
			0xD9,0x1D,0xF8,0x9A,0x48,0x03,
			0xC6,0x05,0xFC,0x9A,0x48,0x03,0x01,
			0xC7,0x05,0x00,0x9B,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0x28,0x9B,0x48,0x03,0x00,0x00,0x00,0xFF,
			0xC7,0x05,0x24,0x9B,0x48,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0x2C,0x9B,0x48,0x03,0xFF,0xFF,0xFF,0xFF,
			0xC7,0x05,0x30,0x9B,0x48,0x03,0x00,0x00,0x00,0x00
		},

			"xx????"
			"xx????x"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx",

			"xxxxxx"
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
		return FALSE;
	}

	Buffer *dest = bb_queue_pick_first(res);

	memcpy(&this->destx_addr, dest->data, dest->size);

	this->destx_addr -= this->mp->base_addr;
	this->desty_addr = this->destx_addr + 8;

	bb_queue_free_all(res, buffer_free);

	return TRUE;
}

BOOL camera_scan_variables ()
/// OK
{
	BOOL res = TRUE;

	info("------------------------------------------------------------------");
	info("Searching for static variables address ...");

	BOOL (*scan_funcs[])(void) = {
		camera_scan_campos,
		camera_scan_camval,
		camera_scan_loading,
		camera_scan_dest,
		camera_scan_cursor_champ,
		camera_scan_game_info,
		camera_scan_win_is_opened,
		camera_scan_hovered_champ,
		camera_scan_victory,
		camera_scan_hover_interface
	};

	for (int i = 0; i < (sizeof(scan_funcs) / sizeof(BOOL (*)())); i++)
	{
		if (!scan_funcs[i]())
			res = FALSE;
	}

	info("------------------------------------------------------------------");
	info("Reading the content of pointers...");
	camera_scan_champions(TRUE);
	camera_refresh_self();

	return res;
}

BOOL camera_refresh_self ()
/// OK
{
	Camera *this = camera_get_instance();

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		if (strcmp(this->self_name, this->champions[i]->player_name) == 0)
		{
			this->self = this->champions[i];
			return TRUE;
		}
	}

	return FALSE;
}

BOOL camera_scan_win_is_opened ()
/// FIXED
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "winIsOpened",
	/*
		0145D640  ╓·  56                      push esi
		0145D641  ║·  8BF1                    mov esi, ecx
		0145D643  ║·  8B0D 4063E203           mov ecx, [dword ds:League_of_Legends.3E26340]
		0145D649  ║·  8B01                    mov eax, [dword ds:ecx]
		0145D64B  ║·  FF50 04                 call [dword ds:eax+4]
		0145D64E  ║·  8B10                    mov edx, [dword ds:eax]
		0145D650  ║·  6A 01                   push 1
		0145D652  ║·  8BC8                    mov ecx, eax
		0145D654  ║·  FF52 2C                 call [dword ds:edx+2C]
	*/
		(unsigned char[]) {
			0x56,
			0x8B,0xF1,
			0x8B,0x0D,0x40,0x63,0xE2,0x03,
			0x8B,0x01,
			0xFF,0x50,0x04,
			0x8B,0x10,
			0x6A,0x01,
			0x8B,0xC8,
			0xFF,0x52,0x2C
		},

			"x"
			"xx"
			"xx????"
			"xx"
			"xxx"
			"xx"
			"xx"
			"xx"
			"xxx",

			"x"
			"xx"
			"xx????"
			"xx"
			"xxx"
			"xx"
			"xx"
			"xx"
			"xxx"

	);

	if (!res)
	{
		warning("Cannot find win_is_opened_ptr address");
		return FALSE;
	}

	Buffer *win_is_opened_ptr = bb_queue_pick_first(res);
	memcpy(&this->win_is_opened_ptr, win_is_opened_ptr->data, win_is_opened_ptr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->win_is_opened_ptr)
	{
		warning("Cannot scan win_is_opened_ptr");
		return FALSE;
	}

	DWORD win_is_opened_addr;
	win_is_opened_addr = read_memory_as_int(this->mp->proc, this->win_is_opened_ptr);

	if (!win_is_opened_addr)
		return FALSE;

	this->win_is_opened_addr = win_is_opened_addr + 0x54;

	return TRUE;
}

BOOL camera_scan_hovered_champ ()
/// FIXED
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "entityHovered",
	/*
		012CEC1D  ║·▲└E9 B8FAFFFF                       jmp League_of_Legends.012CE6DA
		012CEC22  ║►  8A15 92D57203                     mov dl, [byte ds:League_of_Legends.372D592]
		012CEC28  ║►  B9 A0D57203                       mov ecx, offset League_of_Legends.0372D5A0
		012CEC2D  ║·  C705 A0D57203 00000000            mov [dword ds:League_of_Legends.372D5A0], 0
		012CEC37  ║·  C705 A4D57203 00000000            mov [dword ds:League_of_Legends.372D5A4], 0
		012CEC41  ║·  C705 A8D57203 00000000            mov [dword ds:League_of_Legends.372D5A8], 0
		012CEC4B  ║·  890D 94D57203                     mov [dword ds:League_of_Legends.372D594], ecx
		012CEC51  ║·  84D2                              test dl, dl
		012CEC53  ║·▲ 75 1A                             jnz short League_of_Legends.012CEC6F
		012CEC55  ║·  68 A018D600                       push League_of_Legends.00D618A0  ; ╓Arg1 = League_of_Legends.0D618A0
		012CEC5A  ║·  E8 6C2DE0FF                       call League_of_Legends.010D19CB  ; └League_of_Legends.010D19CB
	*/

		(unsigned char []) {
			0xE9,0xB8,0xFA,0xFF,0xFF,
			0x8A,0x15,0x92,0xD5,0x72,0x03,
			0xB9,0xA0,0xD5,0x72,0x03,
			0xC7,0x05,0xA0,0xD5,0x72,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0xA4,0xD5,0x72,0x03,0x00,0x00,0x00,0x00,
			0xC7,0x05,0xA8,0xD5,0x72,0x03,0x00,0x00,0x00,0x00,
			0x89,0x0D,0x94,0xD5,0x72,0x03,
			0x84,0xD2,
			0x75,0x1A,
			0x68,0xA0,0x18,0xD6,0x00,
			0xE8,0x6C,0x2D,0xE0,0xFF
		},

			"x????"
			"xx????"
			"x????"
			"xx????xxxx"
			"xx????xxxx"
			"xx????xxxx"
			"xx????"
			"xx"
			"x?"
			"x????"
			"x????",


			"xxxxx"
			"xxxxxx"
			"x????"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxxxxxx"
			"xxxxxx"
			"xx"
			"xx"
			"xxxxx"
			"xxxxx"

	);

	if (!res)
	{
		warning("Cannot find entity hovered address");
		return FALSE;
	}

	Buffer *entityHovered = bb_queue_pick_first(res);

	memcpy(&this->entity_hovered_addr, entityHovered->data, entityHovered->size);

	if (!this->entity_hovered_addr)
	{
		warning("Cannot scan entity hovered");
		return FALSE;
	}

	return TRUE;
}

BOOL camera_scan_victory ()
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
		return FALSE;
	}

	Buffer *victory_state_addr = bb_queue_pick_first(res);
	memcpy(&this->victory_state_addr, victory_state_addr->data, victory_state_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->victory_state_addr)
	{
		warning("Cannot scan victory_state_addr");
		return FALSE;
	}

	int victory_state = read_memory_as_int(this->mp->proc, this->victory_state_addr);

	if (victory_state != 0)
	{
		this->victory_state = victory_state;
	}

	return (victory_state != 0);
}

BOOL camera_scan_champions (BOOL display_error)
/// FIXED :)
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "entityArrayStart/entityArrayEnd",
	/*
		013EE469   ║► ┌8B35 9C9CD203      ╓mov esi, [dword ds:League_of_Legends.3D29C9C]
		013EE46F   ║· │8B3D A09CD203      ║mov edi, [dword ds:League_of_Legends.3D29CA0]
		013EE475   ║· │3BF7               ║cmp esi, edi
		013EE477   ║·▼│74 31              ║je short League_of_Legends.013EE4AA
		013EE479   ║► │8B0E               ║╓mov ecx, [dword ds:esi]
	*/
		(unsigned char[]) {
			0x8B,0x35,0x9C,0x9C,0xD2,0x03,
			0x8B,0x3D,0xA0,0x9C,0xD2,0x03,
			0x3B,0xF7,
			0x74,0x31,
			0x8B,0x0E
		},

			"xx????"
			"xx????"
			"xx"
			"xx"
			"xx",

			"xx????"
			"xx????"
			"xx"
			"xx"
			"xx"
	);

	if (!res)
	{
		if (display_error)
			warning("Cannot find entities array address");
		return FALSE;
	}

	Buffer *eArrStart = bb_queue_pick_first(res),
		   *eArrEnd   = bb_queue_pick_last(res);

	memcpy(&this->entities_addr, eArrStart->data, eArrStart->size);
	memcpy(&this->entities_addr_end, eArrEnd->data, eArrEnd->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->entities_addr || !this->entities_addr_end)
	{
		if (display_error)
			warning("Cannot scan entities");
		return FALSE;
	}

	this->entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	this->entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);

	if (!this->entity_ptr || !this->entity_ptr_end)
	{
		if (display_error)
			warning("Cannot read entity array boundaries");
		return FALSE;
	}

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = this->champions[i];

		if (e == NULL)
			this->champions[i] = e = entity_new(this->mp, cur);
		else
			entity_init(e, this->mp, cur);

		if (e == NULL)
			debug("  --> Ally %d not found", i);
		else
			debug(" - Entity %d found -> "
				  "%16s : pos={%5.0f,%5.0f} hp={%4.0f/%4.0f} team=%6s "
				  "pname=\"%16s\" - 0x%.8x)",
				  i, e->champ_name, e->p.v.x, e->p.v.y, e->hp, e->hp_max, (e->team == ENTITY_TEAM_BLUE) ? "blue" : "purple",
				  e->player_name, cur
			);
	}

	return TRUE;
}


/** Refreshers **/

BOOL camera_refresh_victory ()
/// OK
{
	Camera *this = camera_get_instance();

	this->victory_state = read_memory_as_int(this->mp->proc, this->victory_state_addr);

	return TRUE;
}

BOOL camera_refresh_champions ()
/// OK
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

BOOL camera_refresh_entity_hovered ()
/// FIXED
{
	Camera *this = camera_get_instance();

	DWORD entity_hovered = read_memory_as_int(this->mp->proc, this->entity_hovered_addr);

	if (entity_hovered != 0)
	{
		char name[16] = {[0 ... 15] = '\0'};
		entity_hovered = read_memory_as_int(this->mp->proc, entity_hovered);

		read_from_memory(this->mp->proc, name, entity_hovered + 0x28, sizeof(name));

		DWORD cur = this->entity_ptr;
		DWORD end = this->entity_ptr_end;

		for (int i = 0; cur != end && i < 10; cur += 4, i++)
		{
			Entity *e = this->champions[i];

			if (strcmp(e->player_name, name) == 0)
			{
				if (strcmp(name, this->self_name) == 0)
				{
					// We don't want to share the view with ourself, dont hover self
					break;
				}

				// Entity hovered
				this->entity_hovered = e;
				break;
			}
		}
	}
	else
		this->entity_hovered = NULL;

	return TRUE;
}

BOOL camera_refresh_entities_nearby ()
/// OK
{
	Camera *this = camera_get_instance();
	float in_screen = 2000.0;

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

	return TRUE;
}

BOOL camera_refresh_win_is_opened ()
/// FIXED
{
	Camera *this = camera_get_instance();

	this->interface_opened = read_memory_as_int(this->mp->proc, this->win_is_opened_addr + 0x244);
	// Shop opened : 4
	// Chat opened : 2
	// Nothing     : 1

	return this->interface_opened != 0;
}

BOOL camera_refresh_hover_interface ()
/// OK
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
static long int patch_id = 0;

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

void camera_get_patches (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask)
{
	BbQueue *addresses = bb_queue_new();
	unsigned char *code = NULL;
	int len;

	if (!camera_search_signatures (sig, sig_mask, description, addrs, size, addresses))
	{
		foreach_bbqueue_item (addresses, MemBuffer *mb)
		{
			DWORD addr;
			(void) mb;

			// Error : Try to restore the former saved patches
			code = camera_read_patch(patch_id, &len, &addr);
			if ((code != NULL)
			&&  (write_to_memory(mp->proc, code, addr, len)))
			{
				// Phew ... it worked
				info("Patch %d restored", patch_id);
				patch_id++;
			}

			else
			{
				important("Cannot load patch %d... Please restart the League of Legends client. Sorry :(", patch_id);
				return;
			}
		}
	}

	else
	{
		foreach_bbqueue_item (addresses, MemBuffer *mb)
		{
			DWORD addr = mb->addr;
			unsigned char *code = mb->buffer->data;
			camera_save_patch(patch_id, &addr, code, strlen(patch_mask));
			patch_id++;
		}
	}

	int loop = 0;

	foreach_bbqueue_item (addresses, MemBuffer *mb)
	{
		DWORD addr = mb->addr;
		unsigned char *code = mb->buffer->data;

		char *newdesc = str_dup_printf("%s (%d)", description, loop);
		patches[loop++] = patch_new (newdesc, mp, addr, sig, code, patch, patch_mask);
		free(newdesc);

		if (loop > size)
			break;
	}

	bb_queue_free_all(addresses, membuffer_free);
}

static BOOL camera_search_signature (unsigned char *pattern, DWORD *addr, unsigned char **code_ptr, char *mask, char *name)
{
	Camera *this = camera_get_instance();
	debug("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0)
	{
		warning("\"%s\" not found (already patched ?)", name);
		return FALSE;
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

	return TRUE;
}

static BOOL camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size, BbQueue *addresses)
{
	Camera *this = camera_get_instance();
	debugb("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0)
	{
		warning("\"%s\" not found (already patched ?)", name);
		for (int i = 0; i < size; i++)
		{
			MemBuffer *mb = membuffer_new(*(addr[i]), pattern, size);
			bb_queue_add(addresses, mb);
			debugb("  --> [%d] - 0x%.8x\n", i, (int) *(addr[i]));
		}

		return FALSE;
	}

	if (bb_queue_get_length(results) != size)
	{
		printf("\n");
		warning("Occurences excepted was %d, %d found.", size, bb_queue_get_length(results));

		if (bb_queue_get_length(results) < size)
		{
			for (int i = bb_queue_get_length(results); i < size; i++)
			{
				MemBuffer *mb = membuffer_new(*(addr[i]), pattern, size);
				bb_queue_add(addresses, mb);
				debugb("  --> [%d] - 0x%.8x\n", i, (int) *(addr[i]));
			}
		}
	}

	int loop = 0;

	debugb(" ->");

	foreach_bbqueue_item (results, memblock) {
		MemBuffer *mb = membuffer_new(memblock->addr, memblock->data, memblock->size);
		bb_queue_add(addresses, mb);
		*(addr[loop++]) = memblock->addr;
		debugb(" 0x%.8x ", (int) memblock->addr);

		if (!is_last_bbqueue_item(results))
			debugb("-");
	}

	debugb("\n");

	bb_queue_free_all(results, memblock_free);

	return TRUE;
}

void camera_export_to_cheatengine ()
{
	Camera *this = camera_get_instance();

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
		"          <Description>\"EntityChampF1\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>0</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>29</ID>\n"
		"              <Description>\"ChampF1_HP\"</Description>\n"
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
		"              <Description>\"ChampF1_HPmax\"</Description>\n"
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
		"              <Description>\"ChampF1_posX\"</Description>\n"
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
		"              <Description>\"ChampF1_posY\"</Description>\n"
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
		"              <Description>\"ChampF1_isVisible\"</Description>\n"
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
		"          <Description>\"EntityAllyF2\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>4</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>30</ID>\n"
		"              <Description>\"AllyF2_HP\"</Description>\n"
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
		"              <Description>\"AllyF2_HPmax\"</Description>\n"
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
		"              <Description>\"AllyF2_posY\"</Description>\n"
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
		"              <Description>\"AllyF2_posX\"</Description>\n"
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
		"              <Description>\"ChampF2_isVisible\"</Description>\n"
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
		"          <Description>\"EntityAllyF3\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>8</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>49</ID>\n"
		"              <Description>\"AllyF3_HP\"</Description>\n"
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
		"              <Description>\"AllyF3_HPmax\"</Description>\n"
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
		"              <Description>\"AllyF3_posX\"</Description>\n"
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
		"              <Description>\"AllyF3_posY\"</Description>\n"
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
		"              <Description>\"ChampF3_isVisible\"</Description>\n"
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
		"          <Description>\"EntityAllyF4\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>C</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>51</ID>\n"
		"              <Description>\"AllyF4_HP\"</Description>\n"
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
		"              <Description>\"AllyF4_HPmax\"</Description>\n"
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
		"              <Description>\"AllyF4_posX\"</Description>\n"
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
		"              <Description>\"AllyF4_posY\"</Description>\n"
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
		"              <Description>\"ChampF4_isVisible\"</Description>\n"
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
		"          <Description>\"EntityAllyF5\"</Description>\n"
		"          <ShowAsHex>1</ShowAsHex>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>10</Offset>\n"
		"          </Offsets>\n"
		"          <CheatEntries>\n"
		"            <CheatEntry>\n"
		"              <ID>53</ID>\n"
		"              <Description>\"AllyF5_HP\"</Description>\n"
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
		"              <Description>\"AllyF5_HPmax\"</Description>\n"
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
		"              <Description>\"AllyF5_posX\"</Description>\n"
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
		"              <Description>\"AllyF5_posX\"</Description>\n"
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
		"              <Description>\"ChampF5_isVisible\"</Description>\n"
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

		this->camPos->addrX,
		this->camPos->addrY,
		this->cam->addrX,
		this->cam ->addrY,
		this->champ->addrX,
		this->champ->addrY,
		this->mouse->addrX,
		this->mouse->addrY,
		this->dest->addrX,
		this->dest->addrY,
		this->entity_hovered_addr,
		this->win_is_opened_ptr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr, this->entities_addr,
		this->game_info_addr
	);

	file_put_contents("out.ct", out, NULL);
}
