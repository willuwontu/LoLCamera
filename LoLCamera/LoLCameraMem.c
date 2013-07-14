#include "LoLCamera.h"

static BbQueue * camera_search_signatures (unsigned char *pattern, char *mask, char *name, DWORD **addr, int size);
static BOOL      camera_search_signature  (unsigned char *pattern, DWORD *addr, char **code_ptr, char *mask, char *name);
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
		 /*
			00A48590      0F28C8               movaps xmm1, xmm0
			00A48593   ·  F30F594C24 10        mulss xmm1, [dword ss:esp+10]
			00A48599   ·  F30F59D0             mulss xmm2, xmm0
			00A4859D   ·  F30F594424 08        mulss xmm0, [dword ss:esp+8]
			00A485A3   ·  0F28DC               movaps xmm3, xmm4
			00A485A6   ·  F30F59D9             mulss xmm3, xmm1
			00A485AA   ·  0F28CC               movaps xmm1, xmm4
			00A485AD   ·  F30F59CA             mulss xmm1, xmm2
			00A485B1   ·  F30F1015 8C4BBA01    movss xmm2, [dword ds:League_of_Legends.CameraX]                        ; float 370.3301
			00A485B9   ·  F30F580D 904BBA01    addss xmm1, [dword ds:League_of_Legends.1BA4B90]                        ; float 0.0
			00A485C1   ·  F30F59C4             mulss xmm0, xmm4
			00A485C5   ·  F30F5805 944BBA01    addss xmm0, [dword ds:League_of_Legends.CameraY]                        ; float 300.0000
			00A485CD   ·  F30F58D3             addss xmm2, xmm3
			00A485D1   ·  F30F1115 8C4BBA01    movss [dword ds:League_of_Legends.CameraX], xmm2                        ; float 370.3301
			00A485D9   ·  F30F110D 904BBA01    movss [dword ds:League_of_Legends.1BA4B90], xmm1                        ; float 0.0
			00A485E1   ·  F30F1105 944BBA01    movss [dword ds:League_of_Legends.CameraY], xmm0                        ; float 300.0000
		*/
		(unsigned char []) {
			0x0F,0x28,0xC8,
			0xF3,0x0F,0x59,0x4C,0x24,0x10,
			0xF3,0x0F,0x59,0xD0,
			0xF3,0x0F,0x59,0x44,0x24,0x08,
			0x0F,0x28,0xDC,
			0xF3,0x0F,0x59,0xD9,
			0x0F,0x28,0xCC,
			0xF3,0x0F,0x59,0xCA,
			0xF3,0x0F,0x10,0x15,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x58,0x0D,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x59,0xC4,
			0xF3,0x0F,0x58,0x05,0x94,0x4B,0xBA,0x01,
			0xF3,0x0F,0x58,0xD3,
			0xF3,0x0F,0x11,0x15,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x0D,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x05,0x94,0x4B,0xBA,0x01
		},
			"xx?"
			"xxx???"
			"xxx?"
			"xxx???"
			"xx?"
			"xxx?"
			"xx?"
			"xxx?"
			"xxx?????"
			"xxx?????"
			"xxx?"
			"xxx?????"
			"xxx?"
			"xxx?????"
			"xxx?????"
			"xxx?????",

		(unsigned char []) {
			0xEB,0x57,								// jmp short League_of_Legends.00A485E9
			0x90,									// nop
			0xF3,0x0F,0x59,0x4C,0x24,0x10,
			0xF3,0x0F,0x59,0xD0,
			0xF3,0x0F,0x59,0x44,0x24,0x08,
			0x0F,0x28,0xDC,
			0xF3,0x0F,0x59,0xD9,
			0x0F,0x28,0xCC,
			0xF3,0x0F,0x59,0xCA,
			0xF3,0x0F,0x10,0x15,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x58,0x0D,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x59,0xC4,
			0xF3,0x0F,0x58,0x05,0x94,0x4B,0xBA,0x01,
			0xF3,0x0F,0x58,0xD3,
			0xF3,0x0F,0x11,0x15,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x0D,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x05,0x94,0x4B,0xBA,0x01
		},
			"xxx"
			"??????"
			"????"
			"??????"
			"???"
			"????"
			"???"
			"????"
			"????????"
			"????????"
			"????"
			"????????"
			"????"
			"????????"
			"????????"
			"????????"
	);


	this->minimap[0] = camera_get_patch (

		 this->mp, "Move the camera when you click on the minimap (0)",
		&this->minimap_addr[0],
		 /*
			00B92B3E  ║► └F30F105C24 10        movss xmm3, [dword ss:arg4]
			00B92B44  ║·  F30F106424 14        movss xmm4, [dword ss:arg5]
			00B92B4A  ║·  F30F106C24 18        movss xmm5, [dword ss:arg6]
			00B92B50  ║·  F30F104424 1C        movss xmm0, [dword ss:arg7]
			00B92B56  ║·  F30F104C24 20        movss xmm1, [dword ss:arg8]
			00B92B5C  ║·  F30F105424 24        movss xmm2, [dword ss:arg.9]
			00B92B62  ║·  F30F111D 8C4BBA01    movss [dword ds:League_of_Legends.01cad584], xmm3                        ; float 633.5077
			00B92B6A  ║·  F30F1125 904BBA01    movss [dword ds:League_of_Legends.1BA4B90], xmm4                        ; float 0.0
			00B92B72  ║·  F30F112D 944BBA01    movss [dword ds:League_of_Legends.CameraY], xmm5                        ; float 543.3905
		*/
		(unsigned char []) {
			0xF3,0x0F,0x10,0x5C,0x24,0x10,
			0xF3,0x0F,0x10,0x64,0x24,0x14,
			0xF3,0x0F,0x10,0x6C,0x24,0x18,
			0xF3,0x0F,0x10,0x44,0x24,0x1C,
			0xF3,0x0F,0x10,0x4C,0x24,0x20,
			0xF3,0x0F,0x10,0x54,0x24,0x24,
			0xF3,0x0F,0x11,0x1D,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x25,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x2D,0x94,0x4B,0xBA,0x01
		},
			"xxx???"
			"xxx???"
			"xxx???"
			"xxx???"
			"xxx???"
			"xxx???"
			"xxx?????"
			"xxx?????"
			"xxx?????",

		(unsigned char []) {
			0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
		},
			"??????"
			"??????"
			"??????"
			"??????"
			"??????"
			"??????"
			"xxxxxxxx"
			"xxxxxxxx"
			"xxxxxxxx"
	);

	this->minimap[1] = camera_get_patch (

		 this->mp, "Move the camera when you click on the minimap (1)",
		&this->minimap_addr[1],
		 /*
			00B929DC  ║·▼┌72 6A                      jb short League_of_Legends.00B92A48
			00B929DE  ║· │F30F1046 0D                movss xmm0, [dword ds:esi+0D]                         ; float 0.0, 0.0, 0.0, 0.0
			00B929E3  ║· │F30F1105 8C4BBA01          movss [dword ds:League_of_Legends.CameraX], xmm0      ; float 0.0, 0.0, 0.0, 0.0
			00B929EB  ║· │F30F1046 11                movss xmm0, [dword ds:esi+11]                         ; float 0.0, 0.0, 0.0, 0.0
			00B929F0  ║· │F30F1105 904BBA01          movss [dword ds:League_of_Legends.1BA4B90], xmm0      ; float 0.0, 0.0, 0.0, 0.0
			00B929F8  ║· │F30F1046 15                movss xmm0, [dword ds:esi+15]                         ; float 0.0, 0.0, 0.0, 0.0
			00B929FD  ║· │F30F1105 944BBA01          movss [dword ds:League_of_Legends.CameraY], xmm0      ; float 0.0, 0.0, 0.0, 0.0
			00B92A05  ║· │0F57C0                     xorps xmm0, xmm0                                      ; float 0.0, 0.0, 0.0, 0.0
		*/
		(unsigned char []) {
			0x72,0x6A,
			0xF3,0x0F,0x10,0x46,0x0D,
			0xF3,0x0F,0x11,0x05,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x10,0x46,0x11,
			0xF3,0x0F,0x11,0x05,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x10,0x46,0x15,
			0xF3,0x0F,0x11,0x05,0x94,0x4B,0xBA,0x01,
			0x0F,0x57,0xC0
		},
			"xx"
			"xxx??"
			"xxx?????"
			"xxx??"
			"xxx?????"
			"xxx??"
			"xxx?????"
			"xx?"
			,

		(unsigned char []) {
			0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90
		},
			"??"
			"?????"
			"xxxxxxxx"
			"xxxxx"
			"xxxxxxxx"
			"xxxxx"
			"xxxxxxxx"
			"???"
	);

	this->respawn_reset = camera_get_patch (

		this->mp, "Center the camera on the champion when you respawn",
	   &this->respawn_reset_addr,

		/*
			00A197C7  ║·  803D 60B7BC01 00     cmp [byte ds:League_of_Legends.1BCB760], 0
			00A197CE  ║·▼ 75 26                jne short League_of_Legends.00A197F6
			00A197D0  ║·  F30F1003             movss xmm0, [dword ds:ebx]
			00A197D4  ║·  F30F1105 8C4BBA01    movss [dword ds:League_of_Legends.CameraX], xmm0                        ; float 802.4597
			00A197DC  ║·  F30F1043 04          movss xmm0, [dword ds:ebx+4]
			00A197E1  ║·  F30F1105 904BBA01    movss [dword ds:League_of_Legends.1BA4B90], xmm0                        ; float 0.0
			00A197E9  ║·  F30F1043 08          movss xmm0, [dword ds:ebx+8]
			00A197EE  ║·  F30F1105 944BBA01    movss [dword ds:League_of_Legends.CameraY], xmm0                        ; float 570.0792
			00A197F6  ║►  68 4CA46901          push offset League_of_Legends.0169A44C                                  ; ╓Arg1 = ASCII "Death"
		*/
		(unsigned char []) {
			0x80,0x3D,		0x60,0xB7,0xBC,0x01,		0x00,
			0x75,0x26,
			0xF3,0x0F,0x10,	0x03,
			0xF3,0x0F,0x11,	0x05,0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x10,	0x43,0x04,
			0xF3,0x0F,0x11,	0x05,0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x10,	0x43,0x08,
			0xF3,0x0F,0x11,	0x05,0x94,0x4B,0xBA,0x01,
			0x68,			0x4C,0xA4,0x69,0x01
		},
			"xx????x"
			"xx"
			"xxx?"
			"xxx?????"
			"xxx??"
			"xxx?????"
			"xxx??"
			"xxx?????"
			"x????",

		(unsigned char []) {
			0x90,0x90,		0x90,0x90,0x90,0x90,0x90,
			0xEB,0x26,									// jne to jmp
			0x90,0x90,0x90,	0x90,
			0x90,0x90,0x90,	0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,	0x90,0x90,
			0x90,0x90,0x90,	0x90,0x90,0x90,0x90,0x90,
			0x90,0x90,0x90,	0x90,0x90,
			0x90,0x90,0x90,	0x90,0x90,0x90,0x90,0x90,
			0x90,			0x90,0x90,0x90,0x90
		},
			"???????"
			"x?"
			"????"
			"????????"
			"?????"
			"????????"
			"?????"
			"????????"
			"?????"
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
			0xF3,0x0F,0x10,0x40,0x6C,					// xxxx?
			0xF3,0x0F,0x11,0x05,0x3C,0x71,0xDF,0x03,	// xxxx????
			0xF3,0x0F,0x10,0x40,0x70,					// xxxx?
			0xF3,0x0F,0x11,0x05,0x40,0x71,0xDF,0x03,	// xxxx????
			0xF3,0x0F,0x10,0x40,0x74,					// xxxx?
			0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03		// xxxx????
		},	"xx"
			"xxxx?"
			"xxxx????"
			"xxxx?"
			"xxxx????"
			"xxxx?"
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
		/*
			00A47860  ║·  D946 68               fld [dword ds:esi+68]
			00A47863  ║·  D95B 14               fstp [dword ds:ebx+14]
			00A47866  ║·  D946 70               fld [dword ds:esi+70]
			00A47869  ║·  D95B 1C               fstp [dword ds:ebx+1C]
		*/

		(unsigned char []) {
			"\xD9\x46\x6C" // xx?
			"\xD9\x5B\x14" // xxx
			"\xD9\x46\x74" // xx?
			"\xD9\x5B\x1C" // xxx
		},	"xx?"
			"xxx"
			"xx?"
			"xxx",

		(unsigned char []) {
			"\x90\x90\x90" 	// xxx
			"\x90\x90\x90" 	// xxx
			"\x90\x90\x90" 	// xxx
			"\x90\x90\x90" 	// xxx
		},	"xxx"
			"xxx"
			"xxx"
			"xxx"
	);

	this->patchlist = patch_list_get();

	return TRUE;
}

BOOL camera_scan_campos ()
{
	Camera *this = camera_get_instance();
	char *description = "CameraX/CameraY";

	BbQueue *res = memscan_search(this->mp, description,
		(unsigned char []) {
		/*
			00A485D1   ·  F30F1115 8C4BBA01     movss [dword ds:League_of_Legends.1BA4B8C], xmm2            ; float 1258.562
			00A485D9      F30F110D 904BBA01     movss [dword ds:League_of_Legends.1BA4B90], xmm1            ; float 0.0
			00A485E1      F30F1105 944BBA01     movss [dword ds:League_of_Legends.1BA4B94], xmm0            ; float 957.6526
			00A485E9   ·▼ 74 05                 je short League_of_Legends.00A485F0
			00A485EB   ·  83F8 04               cmp eax, 4
			00A485EE   ·▼ 75 18                 jne short League_of_Legends.00A48608
		*/
			0xF3,0x0F,0x11,0x15,	0x8C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x0D,	0x90,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x05,	0x94,0x4B,0xBA,0x01,
			0x74,0x05,
			0x83,0xF8,0x04,
			0x75,0x18

		},	"xxx?????"
			"xxx?????"
			"xxx?????"
			"xx"
			"xxx"
			"xx",

			"xxxx????" // <camX
			"xxxxxxxx"
			"xxxx????" // <camY
			"xx"
			"xxx"
			"xx"
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

BOOL camera_refresh_hover_interface ()
{
	Camera *this = camera_get_instance();

	this->interface_hovered = read_memory_as_int(this->mp->proc, this->interface_hovered_addr);

	return TRUE;
}

BOOL camera_scan_hover_interface ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "HoverInterface",
	/*
		00AB08B8   ·  0F94C0                    sete al
		00AB08BB   ·  84C0                      test al, al
		00AB08BD   ·▼ 74 61                     je short League_of_Legends.00AB0920
		00AB08BF   ·  803D E278D201 00          cmp [byte ds:League_of_Legends.1D278E2], 0
		00AB08C6   ·▼ 75 56                     jne short League_of_Legends.00AB091E
		00AB08C8   ·  803D <<5448DE03>> 00      cmp [byte ds:League_of_Legends.3DE4854], 0
	*/
		(unsigned char[]) {
			0x0F,0x94,0xC0,
			0x84,0xC0,
			0x74,0x61,
			0x80,0x3D,	0xE2,0x78,0xD2,0x01,	0x00,
			0x75,0x56,
			0x80,0x3D,	0x54,0x48,0xDE,0x03,	0x00
		},
			"xxx"
			"xx"
			"xx"
			"xx????x"
			"xx"
			"xx????x",

			"xxx"
			"xx"
			"xx"
			"xxxxxxx"
			"xx"
			"xx????x"
	);

	if (!res)
	{
		warning("Cannot find HoverInterface address\nUsing the .ini value : 0x%.8x", this->interface_hovered_addr);
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
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "loadingState",
	/*
		00AD92C8  ║·  83C4 1C                     add esp, 1C
		00AD92CB  ║·  396E 18                     cmp [dword ds:esi+18], ebp
		00AD92CE  ║·  F30F1105 8C9CC701           movss [dword ds:League_of_Legends.1C79C8C], xmm0        ; float 0.004300000
		00AD92D6  ║·▼ 72 04                       jb short League_of_Legends.00AD92DC
		00AD92D8  ║·  8B1F                        mov ebx, [dword ds:edi]
		00AD92DA  ║·▼ EB 02                       jmp short League_of_Legends.00AD92DE
	*/
		(unsigned char[]) {
			0x83,0xC4,0x1C,
			0x39,0x6E,0x18,
			0xF3,0x0F,0x11,0x05,	0x8C,0x9C,0xC7,0x01,
			0x72,0x04,
			0x8B,0x1F,
			0xEB,0x02
		},

		"xxx"
		"xxx"
		"xxxx????"
		"xx"
		"xx"
		"xx",

		NULL
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

BOOL camera_scan_game_state ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "gameState",
	/*
		0047371B   ► └A1 <<98B2CB01>>   mov eax, [dword ds:League_of_Legends.1CBB298]
		00473720   ·  8B70 04       mov esi, [dword ds:eax+4]
		00473723   ·  3BF7          cmp esi, edi
		00473725   ·▲ 74 07         je short League_of_Legends.0047372E
		00473727   ·  8B40 08       mov eax, [dword ds:eax+8]
		0047372A   ·  2BC6          sub eax, esi
	*/
		(unsigned char[]) {
			0xA1,0x98,0xB2,0xCB,0x01,
			0x8B,0x70,0x04,
			0x3B,0xF7,
			0x74,0x07,
			0x8B,0x40,0x08,
			0x2B,0xC6
		},

		"x????"
		"xx?"
		"xx"
		"xx"
		"xx?"
		"xx",

		"x????"
		"xxx"
		"xx"
		"xx"
		"xxx"
		"xx"
	);

	if (!res)
	{
		warning("Cannot find game state address\nUsing the .ini value : 0x%.8x", this->game_state_addr);
		return FALSE;
	}

	Buffer *game_state_addr = bb_queue_pick_first(res);
	memcpy(&this->game_state_addr, game_state_addr->data, game_state_addr->size);

	bb_queue_free_all(res, buffer_free);

	if (!this->game_state_addr)
	{
		warning("Cannot scan game state");
		return FALSE;
	}

	read_from_memory(this->mp->proc, this->self_name, this->game_state_addr + 0x2F4, sizeof(this->self_name) - 1);

	if (strlen(this->self_name) <= 0)
	{
		warning("Cannot find self name");
		return FALSE;
	}

	else
		debug("Self name : <%s>", this->self_name);

	return TRUE;
}

BOOL camera_scan_game_struct ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "gameStruct",
	/*
		00984587  ║·  8915 584BBA01         mov [dword ds:League_of_Legends.1BA4B58], edx
		0098458D  ║·  F30F1105 5C4BBA01     movss [dword ds:League_of_Legends.1BA4B5C], xmm0            ; float 0.0
		00984595  ║·  F30F1105 604BBA01     movss [dword ds:League_of_Legends.1BA4B60], xmm0            ; float 0.0
		0098459D  ║·  F30F110D 644BBA01     movss [dword ds:League_of_Legends.1BA4B64], xmm1            ; float 0.2500000 (const 1/4.)
		009845A5  ║·  F30F1105 <<684BBA01>> movss [dword ds:League_of_Legends.GameStruct], xmm0         ; float 0.0
		009845AD  ║·  8915 6C4BBA01         mov [dword ds:League_of_Legends.1BA4B6C], edx
	*/

		(unsigned char[]) {
			0x89,0x15, 				0x58,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x05,	0x5C,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x05,	0x60,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x0D,	0x64,0x4B,0xBA,0x01,
			0xF3,0x0F,0x11,0x05,	0x68,0x4B,0xBA,0x01,
			0x89,0x15,				0x6C,0x4B,0xBA,0x01
		},

		"xx????"
		"xxxx????"
		"xxxx????"
		"xxxx????"
		"xxxx????"
		"xx????",

		"xxxxxx"
		"xxxxxxxx"
		"xxxxxxxx"
		"xxxxxxxx"
		"xxxx????"
		"xxxxxx"
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

	camera_scan_game_struct_offsets();

	// 00A38400    ║·  D99E F0010000                fstp [dword ds:esi+1F0]
	this->champx_addr = this->game_struct_addr + this->champx_offset - this->mp->base_addr;
	// 00A38412    ║·  D99E F8010000                fstp [dword ds:esi+1F8]
	this->champy_addr = this->game_struct_addr + this->champy_offset - this->mp->base_addr;

	// 00A383D5    ║·  F30F1186 FC010000            movss [dword ds:esi+1FC], xmm0
	this->mousex_addr = this->game_struct_addr + this->mousex_offset - this->mp->base_addr;
	// 00A383F5    ║·  F30F1186 04020000            movss [dword ds:esi+204], xmm0
	this->mousey_addr = this->game_struct_addr + this->mousey_offset - this->mp->base_addr;

	return TRUE;
}

BOOL camera_scan_game_struct_offsets ()
{
	return (camera_scan_champ_offsets());
}

BOOL camera_scan_dest ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search(this->mp, "DestX/DestY",
		/*
			00AB9A58  ║·  C605 9748DE03 FF       mov [byte ds:League_of_Legends.3DE4897], 0FF
			00AB9A5F  ║·  891D 9048DE03          mov [dword ds:League_of_Legends.3DE4890], ebx
			00AB9A65  ║·  892D 9848DE03          mov [dword ds:League_of_Legends.3DE4898], ebp
			00AB9A6B  ║·  F30F1105 <<9C48DE03>>  movss [dword ds:League_of_Legends.3DE489C], xmm0            ; float 449.7802
			00AB9A73  ║·  F30F1105 A048DE03      movss [dword ds:League_of_Legends.3DE48A0], xmm0            ; float 107.7351
			00AB9A7B  ║·  F30F1105 <<A448DE03>>  movss [dword ds:League_of_Legends.3DE48A4], xmm0            ; float 2566.354
		*/
		(unsigned char []) {
			0xC6,0x05,0x97,0x48,0xDE,0x03,0xFF,
			0x89,0x1D,0x90,0x48,0xDE,0x03,
			0x89,0x2D,0x98,0x48,0xDE,0x03,
			0xF3,0x0F,0x11,0x05,0x9C,0x48,0xDE,0x03,
			0xF3,0x0F,0x11,0x05,0xA0,0x48,0xDE,0x03,
			0xF3,0x0F,0x11,0x05,0xA4,0x48,0xDE,0x03
		},

		"xx????x"
		"xx????"
		"xx????"
		"xxxx????"
		"xxxx????"
		"xxxx????",

		"xxxxxxx"
		"xxxxxx"
		"xxxxxx"
		"xxxx????"
		"xxxxxxxx"
		"xxxx????"
	);

	if (!res)
	{
		warning("Cannot find Dest offsets\n");
		return FALSE;
	}

	Buffer *destx = bb_queue_pick_first(res);
	Buffer *desty = bb_queue_pick_last(res);

	memcpy(&this->destx_addr, destx->data, destx->size);
	memcpy(&this->desty_addr, desty->data, desty->size);

	this->destx_addr -= this->mp->base_addr;
	this->desty_addr -= this->mp->base_addr;

	bb_queue_free_all(res, buffer_free);

	return TRUE;
}

BOOL camera_scan_champ_offsets ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "MouseOffX/MouseOffY/ChampOffX/ChampOffY",
	/*
		00ACA22D  ║·  F30F1005 744DCE01  		movss xmm0, [dword ds:League_of_Legends.1CE4D74]      ; float 647.3337
		00ACA235  ║·  F30F1186 <<3401>>0000  	movss [dword ds:esi+134], xmm0
		00ACA23D  ║·  F30F1005 784DCE01  		movss xmm0, [dword ds:League_of_Legends.1CE4D78]      ; float 168.0568
		00ACA245  ║·  F30F1186 38010000  		movss [dword ds:esi+138], xmm0
		00ACA24D  ║·  F30F1005 7C4DCE01  		movss xmm0, [dword ds:League_of_Legends.1CE4D7C]      ; float 467.0539
		00ACA255  ║·  F30F1186 <<3C01>>0000  	movss [dword ds:esi+13C], xmm0
		00ACA25D  ║·  D940 68            		fld [dword ds:eax+68]
		00ACA260  ║·  D99E <<2801>>0000      	fstp [dword ds:esi+128]
		00ACA266  ║·  D940 6C            		fld [dword ds:eax+6C]
		00ACA269  ║·  D99E 2C010000      		fstp [dword ds:esi+12C]
		00ACA26F  ║·  D940 70            		fld [dword ds:eax+70]
		00ACA272  ║·  D99E <<3001>>0000      	fstp [dword ds:esi+130]
		00ACA278  ║►  FF15 9C9BD600      		call [dword ds:<&USER32.GetActiveWindow>]             ; [USER32.GetActiveWindow]
		00ACA27E  ║·  3B05 A4A0CA01      		cmp eax, [dword ds:League_of_Legends.1CAA0A4]
		00ACA284  ║·▼ 0F85 A9020000      		jne League_of_Legends.00ACA533
		00ACA28A  ║·  833D 085DCF01 00   		cmp [dword ds:League_of_Legends.1CF5D08], 0
	*/
		(unsigned char[]) {
			0xF3,0x0F,0x10,0x05,0x74,0x4D,0xCE,0x01,
			0xF3,0x0F,0x11,0x86,0x34,0x01,0x00,0x00,
			0xF3,0x0F,0x10,0x05,0x78,0x4D,0xCE,0x01,
			0xF3,0x0F,0x11,0x86,0x38,0x01,0x00,0x00,
			0xF3,0x0F,0x10,0x05,0x7C,0x4D,0xCE,0x01,
			0xF3,0x0F,0x11,0x86,0x3C,0x01,0x00,0x00,
			0xD9,0x40,0x68,
			0xD9,0x9E,0x28,0x01,0x00,0x00,
			0xD9,0x40,0x6C,
			0xD9,0x9E,0x2C,0x01,0x00,0x00,
			0xD9,0x40,0x70,
			0xD9,0x9E,0x30,0x01,0x00,0x00,
			0xFF,0x15,0x9C,0x9B,0xD6,0x00,
			0x3B,0x05,0xA4,0xA0,0xCA,0x01,
			0x0F,0x85,0xA9,0x02,0x00,0x00,
			0x83,0x3D,0x08,0x5D,0xCF,0x01,0x00
		},
		"xxxx????"
		"xxxx????"
		"xxxx????"
		"xxxx????"
		"xxxx????"
		"xxxx????"
		"xx?"
		"xx????"
		"xx?"
		"xx????"
		"xx?"
		"xx????"
		"xx????"
		"xx????"
		"xx????"
		"xx????x",

		"xxxxxxxx"
		"xxxx????"
		"xxxxxxxx"
		"xxxxxxxx"
		"xxxxxxxx"
		"xxxx????"
		"xxx"
		"xx????"
		"xxx"
		"xxxxxx"
		"xxx"
		"xx????"
		"xxxxxx"
		"xxxxxx"
		"xxxxxx"
		"xxxxxxx"
	);

	if (!res)
	{
		warning("Cannot find Dest/Champion offsets\n");
		return FALSE;
	}

	Buffer *mousex = bb_queue_get_first(res);
	Buffer *mousey = bb_queue_get_first(res);
	Buffer *champx = bb_queue_get_first(res);
	Buffer *champy = bb_queue_get_first(res);

	memcpy(&this->champx_offset, champx->data, champx->size);
	memcpy(&this->champy_offset, champy->data, champy->size);
	memcpy(&this->mousex_offset, mousex->data, mousex->size);
	memcpy(&this->mousey_offset, mousey->data, mousey->size);

	buffer_free(mousex);
	buffer_free(mousey);
	buffer_free(champx);
	buffer_free(champy);
	bb_queue_free_all(res, buffer_free);

	// It appears that the game_struct = esi - 0x10
	this->champx_offset += 0x10;
	this->champy_offset += 0x10;
	this->mousex_offset += 0x10;
	this->mousey_offset += 0x10;

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
		camera_scan_dest,
		camera_scan_game_struct,
		camera_scan_game_state,
		camera_scan_win_is_opened,
		camera_scan_hovered_champ,
		camera_scan_victory,
		camera_scan_hover_interface,
	};

	for (int i = 0; i < (sizeof(scan_funcs) / sizeof(BOOL (*)())); i++)
	{
		if (!scan_funcs[i]())
			res = FALSE;
	}

	info("------------------------------------------------------------------");
	info("Reading the content of pointers...");
	camera_scan_champions();
	camera_refresh_self();

	return res;
}

BOOL camera_refresh_self ()
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

BOOL camera_scan_hovered_champ ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "entityHovered",
	/*
		004D24C0  ║·▼┌74 0C                    je short League_of_Legends.004D24CE
		004D24C2  ║· │A2 181CCC01              mov [byte ds:League_of_Legends.1CC1C18], al
		004D24C7  ║· │C605 50B4CB01 01         mov [byte ds:League_of_Legends.1CBB450], 1
		004D24CE  ║► └3805 50B4CB01            cmp [byte ds:League_of_Legends.1CBB450], al
		004D24D4  ║·  A3 10B4CB01              mov [dword ds:League_of_Legends.<<1CBB410>>], eax  <--- hovered champ
		004D24D9  ║·  A3 14B4CB01              mov [dword ds:League_of_Legends.1CBB414], eax
		004D24DE  ║·  A3 18B4CB01              mov [dword ds:League_of_Legends.1CBB418], eax
		004D24E3  ║·  C705 141CCC01 10B4CB01   mov [dword ds:League_of_Legends.1CC1C14], offset League_of_Legends.01CBB410
	*/

		"\x74\x0C"
		"\xA2\x18\x1C\xCC\x01"
		"\xC6\x05\x50\xB4\xCB\x01\x01"
		"\x38\x05\x50\xB4\xCB\x01"
		"\xA3\x10\xB4\xCB\x01"
		"\xA3\x14\xB4\xCB\x01"
		"\xA3\x18\xB4\xCB\x01"
		"\xC7\x05\x14\x1C\xCC\x01\x10\xB4\xCB\x01",

		"xx"
		"x????"
		"xx????x"
		"xx????"
		"x????"
		"x????"
		"x????"
		"xx????????",

		"xx"
		"xxxxx"
		"xxxxxxx"
		"xxxxxx"
		"x????"
		"xxxxx"
		"xxxxx"
		"xxxxxxxxxx"
	);

	if (!res)
	{
		warning("Cannot find entity hovered address\nUsing the .ini value : 0x%.8x", this->entity_hovered_addr);
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
		warning("Cannot find VictoryState address\nUsing the .ini value : 0x%.8x", this->victory_state_addr);
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

BOOL camera_scan_champions ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "eArrChampsEnd/eArrChampsStart",
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

	if (!this->entities_addr || !this->entities_addr_end)
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

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = this->champions[i];

		if (e == NULL)
			this->champions[i] = e = entity_new(this->mp, cur);
		else
			entity_init(e, this->mp, cur);

		if (e == NULL) // 0 = self
			debug("  --> Ally %d not found", i);
		else
			debug("  --> Entity %d found (pos: x=%.0f y=%.0f hp=%.0f hpmax=%.0f pname=\"%s\" cname=\"%s\" - 0x%.8x)", i, e->p.v.x, e->p.v.y, e->hp, e->hp_max, e->player_name, e->champ_name, cur);
	}

	return TRUE;
}

BOOL camera_refresh_victory ()
{
	Camera *this = camera_get_instance();

	this->victory_state = read_memory_as_int(this->mp->proc, this->victory_state_addr);

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


BOOL camera_scan_win_is_opened ()
{
	Camera *this = camera_get_instance();

	BbQueue *res = memscan_search (this->mp, "winIsOpened",
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
		warning("Cannot find win_is_opened_ptr address\nUsing the .ini value : 0x%.8x", this->win_is_opened_ptr);
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

	// Shop is open is the address of the pointer to the "isShopOpened"
	DWORD win_is_opened_addr = read_memory_as_int(this->mp->proc, this->win_is_opened_ptr);

	if (!win_is_opened_addr)
		return FALSE;

	// isShopOpen = edi+7c
	this->win_is_opened_addr = win_is_opened_addr + 0x7c;

	return TRUE;
}

BOOL camera_refresh_entity_hovered ()
{
	Camera *this = camera_get_instance();

	DWORD entity_hovered = read_memory_as_int(this->mp->proc, this->entity_hovered_addr);

	if (entity_hovered != 0)
	{
		char name[16] = {[0 ... 15] = '\0'};
		entity_hovered = read_memory_as_int(this->mp->proc, entity_hovered);

		read_from_memory(this->mp->proc, name, entity_hovered + 0x2C, sizeof(name));


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
{
	Camera *this = camera_get_instance();
	float in_screen = 2000.0;

	float distance;
	int index = 0;

	memset(this->nearby, 0, sizeof(this->nearby));

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	this->nb_nearby = 0;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = this->champions[i];
		distance = vector2D_distance(&e->p.v, &this->cam->v);

		if (distance < in_screen)
		{
			this->nearby[index++] = e;
			this->nb_nearby = index;
		}
	}

	return TRUE;
}

BOOL camera_refresh_win_is_opened ()
{
	Camera *this = camera_get_instance();

	unsigned char buffer[1] = {0xFF};
	read_from_memory(this->mp->proc, buffer, this->win_is_opened_addr, 1);
	this->interface_opened = (int) buffer[0];

	return (buffer[0] != 0xFF);
}

// ------------ Scanners ------------

static Patch *camera_get_patch (MemProc *mp, char *description, DWORD *addr, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask)
{
	char *code;

	// Get the address of the signature
	if (!camera_search_signature (sig, addr, &code, sig_mask, description))
		return NULL;

	// Create a new patch
	return patch_new (description, mp, *addr, code, sig, patch, patch_mask);
}

static void camera_get_patches (Patch **patches, int size, MemProc *mp, char *description, DWORD **addrs, unsigned char *sig, char *sig_mask, unsigned char *patch, char *patch_mask)
{
	BbQueue *occs = camera_search_signatures (sig, sig_mask, description, addrs, size);
	int loop = 0;

	foreach_bbqueue_item (occs, MemBuffer *mb)
	{
		DWORD addr = mb->addr;
		unsigned char *code = mb->buffer->data;

		char *newdesc = str_dup_printf("%s (%d)", description, loop);
		patches[loop++] = patch_new (newdesc, mp, addr, sig, code, patch, patch_mask);
		free(newdesc);

		if (loop > size)
			break;
	}

	bb_queue_free_all(occs, membuffer_free);
}

static BOOL camera_search_signature (unsigned char *pattern, DWORD *addr, char **code_ptr, char *mask, char *name)
{
	Camera *this = camera_get_instance();
	debugb("Looking for \"%s\" ...", name);

	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	MemBlock *memblock;

	if (bb_queue_get_length(results) <= 0)
	{
		printf("\n");
		warning("\"%s\" not found (already patched ?)\nUsing the current .ini value : 0x%.8x", name, *addr);
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
			MemBuffer *mb = membuffer_new(*(addr[i]), pattern, size);
			bb_queue_add(addresses, mb);
			debugb("  --> [%d] - 0x%.8x\n", i, (int) *(addr[i]));
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

	return addresses;
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
		"      <Description>\"CameraX\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Float</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>60</ID>\n"
		"      <Description>\"CameraY\"</Description>\n"
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
		"      <Description>\"IsShopOpenedPtr\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>Byte</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"      <Offsets>\n"
		"        <Offset>7C</Offset>\n"
		"      </Offsets>\n"
		"    </CheatEntry>\n"
		"    <CheatEntry>\n"
		"      <ID>57</ID>\n"
		"      <Description>\"MouseScreen\"</Description>\n"
		"      <Color>80000008</Color>\n"
		"      <VariableType>4 Bytes</VariableType>\n"
		"      <Address>%.8x</Address>\n"
		"      <Offsets>\n"
		"        <Offset>0</Offset>\n"
		"      </Offsets>\n"
		"      <CheatEntries>\n"
		"        <CheatEntry>\n"
		"          <ID>55</ID>\n"
		"          <Description>\"MouseScreenX\"</Description>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>4C</Offset>\n"
		"          </Offsets>\n"
		"        </CheatEntry>\n"
		"        <CheatEntry>\n"
		"          <ID>56</ID>\n"
		"          <Description>\"MouseScreenY\"</Description>\n"
		"          <Color>80000008</Color>\n"
		"          <VariableType>4 Bytes</VariableType>\n"
		"          <Address>%.8x</Address>\n"
		"          <Offsets>\n"
		"            <Offset>50</Offset>\n"
		"          </Offsets>\n"
		"        </CheatEntry>\n"
		"      </CheatEntries>\n"
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
		"                <Offset>120</Offset>\n"
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
		"                <Offset>130</Offset>\n"
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
		"                <Offset>68</Offset>\n"
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
		"                <Offset>F0</Offset>\n"
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
		"                <Offset>120</Offset>\n"
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
		"                <Offset>130</Offset>\n"
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
		"                <Offset>F0</Offset>\n"
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
		"                <Offset>68</Offset>\n"
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
		"                <Offset>120</Offset>\n"
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
		"                <Offset>130</Offset>\n"
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
		"                <Offset>68</Offset>\n"
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
		"                <Offset>F0</Offset>\n"
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
		"                <Offset>120</Offset>\n"
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
		"                <Offset>130</Offset>\n"
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
		"                <Offset>68</Offset>\n"
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
		"                <Offset>F0</Offset>\n"
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
		"              <Description>\"AllyF4_HP\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>120</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"            <CheatEntry>\n"
		"              <ID>54</ID>\n"
		"              <Description>\"AllyF4_HPmax\"</Description>\n"
		"              <Color>80000008</Color>\n"
		"              <VariableType>Float</VariableType>\n"
		"              <Address>%.8x</Address>\n"
		"              <Offsets>\n"
		"                <Offset>130</Offset>\n"
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
		"                <Offset>68</Offset>\n"
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
		"                <Offset>F0</Offset>\n"
		"                <Offset>10</Offset>\n"
		"              </Offsets>\n"
		"            </CheatEntry>\n"
		"          </CheatEntries>\n"
		"        </CheatEntry>\n"
		"      </CheatEntries>\n"
		"    </CheatEntry>\n"
		"  </CheatEntries>\n"
		"  <UserdefinedSymbols/>\n"
		"</CheatTable>\n",

		this->cam->addrX,
		this->cam->addrY,
		this->champ->addrX,
		this->champ->addrY,
		this->mouse->addrX,
		this->mouse->addrY,
		this->dest->addrX,
		this->dest->addrY,
		this->entity_hovered_addr,
		this->win_is_opened_ptr,
		this->mouse_screen_ptr,
		this->mouse_screen_ptr,
		this->mouse_screen_ptr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr,
		this->entities_addr, this->entities_addr, this->entities_addr, this->entities_addr,	this->entities_addr, this->entities_addr
	);

	file_put_contents("out.ct", out, NULL);
}
