// --- File		: CameraSettings.h
#pragma once

// ---------- Includes ------------
#include <stdlib.h>

// ---------- Defines -------------


// ------ Class declaration -------
typedef
struct _CameraSettings
{
	float lerp_rate;
	float threshold;
	float mouse_range_max;
	float dest_range_max;
	float mouse_dest_range_max;

}	CameraSettings;

// --------- Constructors ---------

CameraSettings *
camera_settings_new (float lerp_rate, float threshold, float mouse_range_max, float dest_range_max, float mouse_dest_range_max);


// ----------- Methods ------------

void
camera_settings_init(CameraSettings *this, float lerp_rate, float threshold, float mouse_range_max, float dest_range_max, float mouse_dest_range_max);



// --------- Destructors ----------

void
camera_settings_free (CameraSettings *camera_settings);


