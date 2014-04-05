#include "CameraSettings.h"
#include <stdio.h>

CameraSettings *
camera_settings_new (float camera_scroll_speed, float threshold)
{
	CameraSettings *this;

	if ((this = malloc(sizeof(CameraSettings))) == NULL)
		return NULL;

	camera_settings_init(this, camera_scroll_speed, threshold);

	return this;
}

void
camera_settings_debug (CameraSettings *this)
{
	printf (
		"float camera_scroll_speed = %f\n"
		"float threshold = %f\n",
		 this->camera_scroll_speed,
		 this->threshold
	);
}

void
camera_settings_init(CameraSettings *this, float camera_scroll_speed, float threshold)
{
	this->camera_scroll_speed = camera_scroll_speed;
	this->threshold = threshold;
}

void
camera_settings_free (CameraSettings *this)
{
	if (this != NULL)
	{
		free (this);
	}
}
