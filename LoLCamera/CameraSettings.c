#include "CameraSettings.h"
#include <stdio.h>

CameraSettings *
camera_settings_new (float camera_scroll_speed, float threshold, float mouse_range_max, float dest_range_max, float mouse_dest_range_max)
{
	CameraSettings *this;

	if ((this = malloc(sizeof(CameraSettings))) == NULL)
		return NULL;

	camera_settings_init(this, camera_scroll_speed, threshold, mouse_range_max, dest_range_max, mouse_dest_range_max);

	return this;
}

void
camera_settings_debug (CameraSettings *this)
{
	printf (
		"float camera_scroll_speed = %f\n"
		"float threshold = %f\n"
		"float mouse_range_max = %f\n"
		"float dest_range_max = %f\n"
		"float mouse_dest_range_max = %f\n",
		 this->camera_scroll_speed,
		 this->threshold,
		 this->mouse_range_max,
		 this->dest_range_max,
		 this->mouse_dest_range_max
	);
}

void
camera_settings_init(CameraSettings *this, float camera_scroll_speed, float threshold, float mouse_range_max, float dest_range_max, float mouse_dest_range_max)
{
	this->camera_scroll_speed = camera_scroll_speed;
	this->threshold = threshold;
	this->mouse_range_max = mouse_range_max;
	this->dest_range_max = dest_range_max;
	this->mouse_dest_range_max = mouse_dest_range_max;
}

void
camera_settings_free (CameraSettings *this)
{
	if (this != NULL)
	{
		free (this);
	}
}
