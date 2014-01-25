#pragma once

// ---------- Includes ------------
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <conio.h>
#include <direct.h>

#include "../Win32Tools/Win32Tools.h"
#include "../IniParser/IniParser.h"
#include "../Vector/Vector2D.h"
#include "../MemPos/MemPos.h"
#include "../Patcher/Patcher.h"
#include "../Scanner/Scanner.h"
#include "../MemBuffer/MemBuffer.h"
#include "../Event/Event.h"
#include "../EasySocket/EasySocket.h"
#include "../Utils/Utils.h"
#include "./Entity.h"
#include "./CameraSettings.h"

// ---------- Defines -------------
#define LOLCAMERA_SHOP_OPENED_VALUE 4
#define LOLCAMERA_CHAT_OPENED_VALUE 2
#define LOLCAMERA_PING_OR_SKILL_PRESSED 5

#define IN_SCREEN_DISTANCE 2000.0
#define MEDIUM_BOX 		   1500.0
#define SMALL_BOX 		   1000.0

// ------ Class declaration -------
typedef struct _Camera Camera;
typedef struct _Minimap Minimap;


struct _Minimap
{
    // Screen pixels position
    int xLeft, xRight, yTop, yBot;
};

struct _Camera
{
	MemProc *mp;					// Process context

	// From .ini
	DWORD border_screen_addr;		// Address of the instructions moving the camera
	DWORD locked_camera_addr;		// Address of the instructions moving the camera on locked camera
	DWORD camera_movement_addr;		// Address of the instructions moving the camera when the mouse reaches the border of the screen
	DWORD win_is_opened_ptr;		// Address of the data : address of the pointer to the variable containing "isShopOpen" (different of 0 if its the case)
	DWORD entities_addr;			// Address of the data : entities array start
	DWORD entities_addr_end;		// Address of the data : entities array end
	DWORD players_addr;				// Address of the data : entities array start
	DWORD players_addr_end;			// Address of the data : entities array end
	DWORD camx_addr, camy_addr; 	// Address of the data : cameraX, cameray (to write)
	DWORD camx_val, camy_val; 		// Address of the data : cameraX, cameray (values)
	DWORD champx_addr, champy_addr;	// Address of the data : championX / championY
	DWORD mousex_addr, mousey_addr; // Address of the data : mouseX / mouseY
	DWORD destx_addr, desty_addr;   // Address of the data : destX / destY (right click)
	DWORD win_is_opened_addr;		// Address of the data : address of the variable containing "isShopOpen" (different of 0 if its the case)

	DWORD loading_state_addr;		// Adress of the data : loading state
	DWORD game_info_addr;			// Adress of the data : info of the game
	DWORD victory_state_addr;		// Adress of the data : Victory or Defeat

	DWORD interface_hovered_addr;	// Address of the data : Is the interface hovered ?
	DWORD mmsize_addr;              // Address of the array containing the size of the minimap on the screen
	DWORD ping_state_addr;          // Address of the data : Has ping button been pressed ?

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
	short int last_translate_state;

	// Hotkeys
	char toggle_key;
	char translate_key;
	char center_key;
	char global_key;
	char drag_key;

	// Settings
	CameraSettings all_settings[200]; // Champion specific settings
	CameraSettings champ_settings;	  // Current champion settings
	float camera_scroll_speed;		// This controls smoothing, smaller values mean slower camera movement
	float threshold;				// Minimum threshold before calculations halted because camera is "close enough"
									// Controls the range at which these factors start falling off :
	float mouse_range_max,			// mouse-champ range
		  dest_range_max,			// dest-champ range
		  mouse_dest_range_max;		// mouse-dest range


	// Camera system settings
	int sleep_time;					// Sleep time at each start of main loop
	int poll_data;					// Number of loops required for polling data

	// List of patchs
	Patch *camera_movement;			// Disables the behavior "Move the camera"
	Patch *border_screen;			// Disables the behavior "Move the camera when the mouse is near the screen border"
	BbQueue *patchlist;				// List of all patches

	// Entities
	Entity *champions[10];			// Current played champion + 4 allies + 5 ennemies - NULL if doesn't exist
	Entity *focused_entity;			// The focused entity champion (NULL if none or self)
	Entity *followed_entity;	    // The followed entity champion when you press Fx (NULL if none)
	Entity *hint_entity;			// Keep this entity in sight *if possible* (NULL if none or self)
	Entity *self;					// The player champion
	char self_name[17];
	Entity *nearby[10];				// Array of entities nearby the champion
	Entity *nearby_allies[5];		// Array of allies entities nearby the champion
	Entity *nearby_ennemies[5];		// Array of ennemies entities nearby the champion
	int nb_allies_nearby;
	int nb_ennemies_nearby;

	// Weights : Configurable in ini
	float focus_weight;
	float hint_weight;
	float champ_weight;
	float dest_weight;
	float mouse_weight;
	float global_weight_allies;
	float global_weight_ennemies;
	float lmb_weight;

	// Players information
	int playersCount;				// Size of the entities array

	// Vector positions
	MemPos *cam;					// Camera ingame position (values only)
	MemPos *camPos;					// Camera ingame position (real position to write)
	MemPos *champ;					// User champion position
	MemPos *dest;					// Last Right mouse button click position
	MemPos *mouse;					// Mouse position
	MemPos tmpcam;					// Temporary Camera state
	Vector2D lmb;					// Last Left mouse button click position
	Vector2D last_campos;			// Last camera position
	Vector2D last_champpos;			// Last champion position
	Vector2D drag_pos;				// Position IG where the drag started
	Vector2D translate_lmb;			// Translation value from lmb clicking

	// Key states
	short int last_toggle_state;
	int mbutton_state;
	int lbutton_state;
	int fxstate;						// Fx is pressed ?

	// Program state
	bool restore_tmpcam;				// Request to restore the temporary camera
	bool drag_request;					// User requested a drag
	bool active;						// Loop state
	bool request_polling; 				// Force to poll data the next loop if TRUE
	int interface_opened;				// Interface state (window focus : shop, chat, etc)
	bool enabled;						// LoLCamera enabled ?
	bool dbg_mode;						// For unit tests
	bool wait_loading_screen;			// Wait for the start of the game
	bool output_cheatengine_table;		// Output the adresses in CheatEngineTable format in "out.ct"
	bool interface_hovered;
	int victory_state;
	int ms_after_minimap_click;
	char *section_settings_name;
	bool wait_for_end_of_pause;
	bool dead_mode;
	bool global_weight_activated;
	bool patch_border_screen_moving;
	Minimap minimap;
	POINT mouse_screen;
	int ping_state;

	// Events
	Event reset_after_minimap_click;

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


// ----------- Methods ------------
LoLCameraState camera_main ();
bool camera_update ();
void camera_load_ini ();
inline void camera_set_active (bool active);
Camera *camera_get_instance ();
bool camera_is_near (MemPos *po, float limit);
void camera_load_settings (char *section);
bool exit_request (int key);
bool update_request (int c);
int get_kb ();
void camera_check_version (void);
void camera_set_pos (float x, float y);
short int camera_getkey (int key);
void camera_reset ();

// -- Hotkeys
bool global_key_pressed ();

// From LoLCameraMem.c
bool camera_scan_champions (bool display_error);
bool camera_scan_patch ();
bool camera_scan_win_is_opened ();
bool camera_scan_minimap_size ();
bool camera_scan_variables ();
bool camera_scan_loading ();
bool camera_scan_cursor_champ ();
bool camera_scan_hovered_champ ();
bool camera_scan_champ_offsets ();
bool camera_scan_dest_offsets ();
bool camera_scan_victory ();
bool camera_scan_ping_or_skill_waiting ();

bool camera_refresh_champions ();
bool camera_refresh_entity_hovered ();
bool camera_refresh_win_is_opened ();
bool camera_refresh_self ();
bool camera_wait_for_ingame ();
bool camera_refresh_victory ();
bool camera_refresh_entities_nearby ();
bool camera_refresh_hover_interface ();
bool camera_refresh_screen_border ();
bool camera_refresh_mouse_screen ();
bool camera_refresh_ping_state ();

// from CameraUnitTest.c
bool camera_ut_campos ();
bool camera_ut_champos ();
bool camera_ut_mousepos ();
bool camera_ut_destpos ();
bool camera_ut_is_win_opened ();
bool camera_ut_screen_mouse ();
bool camera_ut_loading_state ();
bool camera_ut_entities ();

void camera_export_to_cheatengine ();

// --------- Destructors ----------
void camera_unload ();



/*
	That Kirby must stay in the sources somewhere, whatever happens!

                ██████████
            ████░░      ░░████
          ██░░              ░░██
        ██░░                  ██
        ██                    ░░██
      ██              ██  ██  ░░██
    ██░░              ██  ██      ██
    ██                ██  ██      ██
    ██          ░░░░        ░░░░  ██
    ██░░    ░░                ░░  ██
      ██░░  ██        ██      ██░░██
        ████░░              ░░████
          ████░░░░        ░░████
        ██░░░░██████████████░░░░██
      ██░░░░░░░░░░████████░░░░░░░░██
        ██████████        ████████
*/
