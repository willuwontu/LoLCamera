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

// ---------- Defines -------------


// ------ Class declaration -------
typedef struct _Camera Camera;

struct _Camera
{
	MemProc *mp;	// Process context

	// Internal
	BOOL active;			// Loop state
	BOOL request_polling; 	// Force to poll data the next loop if TRUE

	MemPos *cam;	// Camera ingame position
	MemPos *champ;	// User champion position
	MemPos *mouse;	// Mouse position
	MemPos *dest;	// Right click position

	DWORD default_camera_addr;	// Address of the instructions moving the camera
	DWORD minimap_camera_addr;	// Address of the instructions moving the camera when you click on the minimap
	DWORD minimap_camera_addr2;	// Another address for the same purpose
	DWORD reset_cam_respawn_addr;	// Address of the instructions when the camera resets when the champion respawns

	// From .ini
	float lerp_rate;		// This controls smoothing, smaller values mean slower camera movement
	float threshold;		// Minimum threshold before calculations halted because camera is "close enough"
	int sleep_time;			// Sleep time at each start of main loop
	int poll_data;			// Number of loops required for polling data

	float mouse_range_max,  // controls the range at which these factors start falling off
		  dest_range_max;

	float camera_far_limit;		// Beyond this limit, the camera is considered "far"

	BOOL enabled;
};

// --------- Constructors ---------
void camera_init (MemProc *mp);


// ----------- Methods ------------

void camera_main (void);
BOOL camera_update ();
inline void camera_set_active (BOOL active);
inline Camera *camera_get_instance ();

// Patchers
DWORD camera_search_signature (unsigned char *pattern, char *mask, char *name);
void camera_default_set_patch (BOOL patch_active);
void camera_reset_when_respawn_set_patch (BOOL patch_active);


// --------- Destructors ----------
void camera_unload ();







#endif // Camera_INCLUDED
