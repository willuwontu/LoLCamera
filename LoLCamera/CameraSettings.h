// --- File		: CameraSettings.h
#pragma once

// ---------- Includes ------------
#include <stdlib.h>

// ---------- Defines -------------


// ------ Class declaration -------
typedef
struct _CameraSettings
{
	float camera_scroll_speed;
	float threshold;
	float camera_scroll_speed_bottom; // speed for going to the south

}	CameraSettings;

// --------- Constructors ---------

CameraSettings *
camera_settings_new (float camera_scroll_speed, float threshold);


// ----------- Methods ------------

void
camera_settings_init(CameraSettings *this, float camera_scroll_speed, float threshold);

void
camera_settings_debug (CameraSettings *this);


// --------- Destructors ----------

void
camera_settings_free (CameraSettings *camera_settings);


