// --- File		: CameraSettings.h
#pragma once

// ---------- Includes ------------
#include <stdlib.h>

// ---------- Defines -------------


// ------ Struct declaration -------
typedef
struct _CameraSettings
{
	float camera_scroll_speed;
	float threshold;
	float camera_scroll_speed_vertical;
	float camera_scroll_speed_horizontal;
	float camera_scroll_speed_bottom;

}	CameraSettings;

typedef enum {
	CAMERA_SETTING_SCROLL_SPEED,
	CAMERA_SETTING_THRESHOLD,
	CAMERA_SETTING_SCROLL_SPEED_HORIZONTAL,
	CAMERA_SETTING_SCROLL_SPEED_VERTICAL,
	CAMERA_SETTING_SCROLL_SPEED_BOTTOM

} CameraSettingsType;

// --------- Constructors ---------


// ----------- Functions ------------

void
camera_settings_debug (CameraSettings *this);


// --------- Destructors ----------

void
camera_settings_free (CameraSettings *camera_settings);


