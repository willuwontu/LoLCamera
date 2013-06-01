#include "CameraSettings.h"


CameraSettings *
camera_settings_new (float lerp_rate, float threshold, float mouse_range_max, float dest_range_max, float mouse_dest_range_max)
{
	CameraSettings *this;

	if ((this = malloc(sizeof(CameraSettings))) == NULL)
		return NULL;

	camera_settings_init(this, lerp_rate, threshold, mouse_range_max, dest_range_max, mouse_dest_range_max);

	return this;
}

void
camera_settings_init(CameraSettings *this, float lerp_rate, float threshold, float mouse_range_max, float dest_range_max, float mouse_dest_range_max)
{
	this->lerp_rate = lerp_rate;
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
