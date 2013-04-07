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

	MemPos *cam;	// Camera ingame position
	MemPos *champ;	// User champion position
	MemPos *mouse;	// Mouse position
	MemPos *dest;	// Right click position

	float lerp_rate;
	float threshold;
	int sleep_time;
	int poll_data;

	BOOL active;			// Loop state
	BOOL request_polling; 	// Force to poll data the next loop if TRUE

	DWORD move_camera_addr;	// Address of the instructions moving the camera
	DWORD minimap_camera_addr;	// Address of the instructions moving the camera when you click on the minimap
};

// --------- Constructors ---------
void camera_init (MemProc *mp);


// ----------- Methods ------------

void camera_main (void);
BOOL camera_update ();

inline void camera_set_active (BOOL active);

void camera_default_set_patch (BOOL patch_active);

DWORD camera_search_signature (unsigned char *pattern, char *mask, char *name);

inline Camera *camera_get_instance ();

// --------- Destructors ----------

void camera_unload ();







#endif // Camera_INCLUDED
