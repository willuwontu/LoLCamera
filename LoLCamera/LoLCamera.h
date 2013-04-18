// --- File		: LoLCamera.h

#ifndef Camera_H_INCLUDED
#define Camera_H_INCLUDED

// ---------- Includes ------------
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>

#include "../Win32Tools/Win32Tools.h"
#include "../IniParser/IniParser.h"
#include "../Vector/Vector2D.h"
#include "../MemPos/MemPos.h"
#include "../Patcher/Patcher.h"
#include "../Scanner/Scanner.h"
#include "./Entity.h"

// ---------- Defines -------------

// ------ Class declaration -------
typedef struct _Camera Camera;

struct _Camera
{
	MemProc *mp;					// Process context

	// Internal
	BOOL active;					// Loop state
	BOOL request_polling; 			// Force to poll data the next loop if TRUE

	// From .ini
	DWORD border_screen_addr;		// Address of the instructions moving the camera
	DWORD respawn_reset_addr;		// Address of the instructions when the camera resets when the champion respawns
	DWORD locked_camera_addr;		// Address of the instructions moving the camera when camera mode is active
	DWORD allies_cam_addr[2];		// Address of the instructions moving the camera when you press F2-3-4-5
	DWORD self_cam_addr;			// Address of the instructions moving the camera when you press F1

	DWORD shop_is_opened_ptr;		// Address of the data : address of the pointer to the variable containing "isShopOpen" (different of 0 if its the case)
	DWORD entities_addr;			// Address of the data : entities array start
	DWORD entities_addr_end;		// Address of the data : entities array end
	DWORD camx_addr, camy_addr; 	// Address of the data : cameraX, cameray
	DWORD champx_addr, champy_addr;	// Address of the data : championX / championY
	DWORD mousex_addr, mousey_addr; // Address of the data : mouseX / mouseY
	DWORD destx_addr, desty_addr;   // Address of the data : destX / destY (right click)
	DWORD mouse_screen_ptr;			// Address of the pointer to the pointer to the structure containing mouseScreenX/Y
	DWORD mouse_screen_addr;		// Address of the pointer to the structure containing mouseScreenX/Y
	DWORD shop_is_opened_addr;		// Address of the data : address of the variable containing "isShopOpen" (different of 0 if its the case)

	// Static addresses
	DWORD entity_ptr,entity_ptr_end;//

	float lerp_rate;				// This controls smoothing, smaller values mean slower camera movement
	float threshold;				// Minimum threshold before calculations halted because camera is "close enough"
	int sleep_time;					// Sleep time at each start of main loop
	int poll_data;					// Number of loops required for polling data

									// Controls the range at which these factors start falling off :
	float mouse_range_max,			// mouse-champ range
		  dest_range_max,			// dest-champ range
		  mouse_dest_range_max;		// mouse-dest range

	// List of patchs
	Patch *F1_pressed;				// Disables the behavior "Center the camera on the champion when F1 is pressed"
	Patch *F2345_pressed[2];		// Disables the behavior "Center the camera on the ally X when FX is pressed"
	Patch *border_screen;			// Disables the behavior "Move the camera when the mouse is near the screen border"
	Patch *respawn_reset;			// Disables the behavior "Center the camera on the champion when you respawn"
	Patch *locked_camera;			// Disables the behavior "Center the camera on the champion when locked camera mode is active"

	// Entities
	Entity *champions[5];			// Current played champion + 4 allies - NULL if doesn't exist

	// Memory positions
	MemPos *cam;					// Camera ingame position
	MemPos *champ;					// User champion position
	MemPos *mouse;					// Mouse position
	MemPos *dest;					// Right click position
	MemPos *mouse_screen;			// Mouse screen position

	BOOL shop_opened;

	BOOL enabled;
};


// --------- Constructors ---------
void camera_init (MemProc *mp);


// ----------- Methods ------------
void camera_main (void);
BOOL camera_update ();
void camera_load_ini ();
inline void camera_set_active (BOOL active);
Camera *camera_get_instance ();

// from LoLCameraMem.c
BOOL camera_scan_champions ();
void camera_scan_patch ();
BOOL camera_scan_mouse_screen ();
BOOL camera_refresh_shop_is_opened ();
BOOL camera_scan_shop_is_opened ();
void camera_wait_ingame ();


void camera_scan_variables ();


// --------- Destructors ----------
void camera_unload ();




#endif // Camera_INCLUDED
