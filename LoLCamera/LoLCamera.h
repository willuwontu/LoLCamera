// --- File		: LoLCamera.h
#pragma once

// ---------- Includes ------------
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <conio.h>

#include "../Win32Tools/Win32Tools.h"
#include "../IniParser/IniParser.h"
#include "../Vector/Vector2D.h"
#include "../MemPos/MemPos.h"
#include "../Patcher/Patcher.h"
#include "../Scanner/Scanner.h"
#include "../MemBuffer/MemBuffer.h"
#include "./Entity.h"
#include "./CameraSettings.h"

// ---------- Defines -------------
#define LOLCAMERA_SHOP_OPENED_VALUE 6
#define LOLCAMERA_CHAT_OPENED_VALUE 5

// ------ Class declaration -------
typedef struct _Camera Camera;

struct _Camera
{
	MemProc *mp;					// Process context

	// From .ini
	DWORD border_screen_addr;		// Address of the instructions moving the camera
	DWORD respawn_reset_addr;		// Address of the instructions when the camera resets when the champion respawns
	DWORD locked_camera_addr;		// Address of the instructions moving the camera when camera mode is active
	DWORD allies_cam_addr[2];		// Address of the instructions moving the camera when you press F2-3-4-5
	DWORD self_cam_addr;			// Address of the instructions moving the camera when you press F1
	DWORD minimap_addr[2];			// Address of the instructions moving the camera when you click on the minimap

	DWORD win_is_opened_ptr;		// Address of the data : address of the pointer to the variable containing "isShopOpen" (different of 0 if its the case)
	DWORD entities_addr;			// Address of the data : entities array start
	DWORD entities_addr_end;		// Address of the data : entities array end
	DWORD players_addr;				// Address of the data : entities array start
	DWORD players_addr_end;			// Address of the data : entities array end
	DWORD camx_addr, camy_addr; 	// Address of the data : cameraX, cameray
	DWORD champx_addr, champy_addr;	// Address of the data : championX / championY
	DWORD mousex_addr, mousey_addr; // Address of the data : mouseX / mouseY
	DWORD destx_addr, desty_addr;   // Address of the data : destX / destY (right click)
	DWORD mouse_screen_ptr;			// Address of the pointer to the pointer to the structure containing mouseScreenX/Y
	DWORD mouse_screen_addr;		// Address of the pointer to the structure containing mouseScreenX/Y
	DWORD win_is_opened_addr;		// Address of the data : address of the variable containing "isShopOpen" (different of 0 if its the case)

	DWORD loading_state_addr;		// Adress of the data : loading state
	DWORD game_struct_addr;			// Adress of the data : structure of the game
	DWORD game_state_addr;			// Adress of the data : state of the game
	DWORD victory_state_addr;		// Adress of the data : Victory or Defeat


	// Offsets in the game structure
	DWORD champx_offset;
	DWORD champy_offset;
	DWORD mousex_offset;
	DWORD mousey_offset;

	// Static addresses
	DWORD entity_ptr,entity_ptr_end;
	DWORD players_ptr,players_ptr_end;
	Entity *entity_hovered;
	DWORD entity_hovered_addr;

	// Translation
	Vector2D distance_translation;
	int translate_request;
	char translate_key;
	short int last_translate_state;

	// Settings
	CameraSettings all_settings[200]; // Champion specific settings
	CameraSettings champ_settings;	  // Current champion settings
	float lerp_rate;				// This controls smoothing, smaller values mean slower camera movement
	float threshold;				// Minimum threshold before calculations halted because camera is "close enough"
									// Controls the range at which these factors start falling off :
	float mouse_range_max,			// mouse-champ range
		  dest_range_max,			// dest-champ range
		  mouse_dest_range_max;		// mouse-dest range


	// Camera system settings
	int sleep_time;					// Sleep time at each start of main loop
	int poll_data;					// Number of loops required for polling data
	Vector2D drag_pos;				// Position IG where the drag started

	// List of patchs
	Patch *F1_pressed;				// Disables the behavior "Center the camera on the champion when F1 is pressed"
	Patch *F2345_pressed[2];		// Disables the behavior "Center the camera on the ally X when FX is pressed"
	Patch *border_screen;			// Disables the behavior "Move the camera when the mouse is near the screen border"
	Patch *respawn_reset;			// Disables the behavior "Center the camera on the champion when you respawn"
	Patch *locked_camera;			// Disables the behavior "Center the camera on the champion when locked camera mode is active"
	Patch *minimap[2];				// Disables the behavior "Center the camera on the champion when locked camera mode is active"
	BbQueue *patchlist;				// List of all patches

	// Entities
	Entity *champions[10];			// Current played champion + 4 allies + 5 ennemies - NULL if doesn't exist
	Entity *focused_entity;			// The focused entity champion (NULL if none or self)
	Entity *followed_entity;	    // The followed entity champion when you press Fx (NULL if none)
	Entity *hint_entity;			// Keep this entity in sight *if possible* (NULL if none or self)
	Entity *self;
	char self_name[17];
	Entity *nearby[10];				// Array of entities nearby the champion
	int nb_nearby;

	// Weights : Configurable in ini
	float focus_weight;
	float hint_weight;
	float champ_weight;
	float dest_weight;
	float mouse_weight;
	float global_weight;

	// Players information
	int team_size;					// Size of the -team- of the array actually, FIXME

	// Memory positions
	MemPos *cam;					// Camera ingame position
	MemPos *champ;					// User champion position
	MemPos *mouse;					// Mouse position
	MemPos *dest;					// Right click position
	MemPos *mouse_screen;			// Mouse screen position
	MemPos tmpcam;					// Temporary Camera state

	// Key states
	short int last_toggle_state;
	int mbutton_state;
	int lbutton_state;
	int fxstate;						// Fx is pressed ?

	// Program state
	BOOL global_entities;			        // Position the camera according to entities position
	BOOL restore_tmpcam;				// Request to restore the temporary camera
	BOOL drag_request;					// User requested a drag
	BOOL active;						// Loop state
	BOOL request_polling; 				// Force to poll data the next loop if TRUE
	int interface_opened;
	BOOL enabled;
	BOOL dbg_mode;
	BOOL wait_loading_screen;			// Wait for the start of the game
	BOOL output_cheatengine_table;		// Output the adresses in CheatEngineTable format in "out.ct"
	int victory_state;

	// We need it for loading champion settings
	IniParser *parser;
};

typedef enum {

	END_OF_LOLCAMERA,
	WAIT_FOR_END_OF_GAME,
	WAIT_FOR_NEW_GAME,
	PLAY


} LoLCameraState;

// --------- Constructors ---------
void camera_init (MemProc *mp);


// ----------- Methods ------------m
LoLCameraState camera_main ();
BOOL camera_update ();
void camera_load_ini ();
inline void camera_set_active (BOOL active);
Camera *camera_get_instance ();
BOOL camera_is_near (MemPos *po, float limit);
void camera_load_settings (char *section);
BOOL exit_request ();

// from LoLCameraMem.c
BOOL camera_scan_champions ();
BOOL camera_scan_patch ();
BOOL camera_scan_mouse_screen ();
BOOL camera_scan_win_is_opened ();
BOOL camera_scan_variables ();
BOOL camera_scan_loading ();
BOOL camera_scan_game_struct ();
BOOL camera_scan_hovered_champ ();
BOOL camera_scan_game_struct_offsets ();
BOOL camera_scan_champ_offsets ();
BOOL camera_scan_dest_offsets ();
BOOL camera_scan_victory ();

BOOL camera_refresh_champions ();
BOOL camera_refresh_entity_hovered ();
BOOL camera_refresh_win_is_opened ();
BOOL camera_refresh_self ();
BOOL camera_wait_for_ingame ();
BOOL camera_refresh_victory ();
BOOL camera_refresh_entities_nearby ();

// from CameraUnitTest.c
BOOL camera_ut_campos ();
BOOL camera_ut_champos ();
BOOL camera_ut_mousepos ();
BOOL camera_ut_destpos ();
BOOL camera_ut_is_win_opened ();
BOOL camera_ut_screen_mouse ();
BOOL camera_ut_loading_state ();

void camera_export_to_cheatengine ();

// --------- Destructors ----------
void camera_unload ();


