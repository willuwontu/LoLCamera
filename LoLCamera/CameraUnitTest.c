#include "LoLCamera.h"

#define MAP_WIDTH  15000.0
#define MAP_HEIGHT 15200.0

bool camera_ut_campos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->cam, &x, &y);

	return (x >= 0.0       && y >= 0.0
	&&		x <= MAP_WIDTH && y <= MAP_HEIGHT);
}

bool camera_ut_champos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->champ, &x, &y);

	return (x >= 0.0       && y >= 0.0
	&&		x <= MAP_WIDTH && y <= MAP_HEIGHT);
}

bool camera_ut_mousepos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->mouse, &x, &y);

	return (x >= 0.0       && y >= 0.0
	&&		x <= MAP_WIDTH && y <= MAP_HEIGHT);
}

bool camera_ut_destpos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->dest, &x, &y);

	return (x >= 0.0       && y >= 0.0
	&&		x <= MAP_WIDTH && y <= MAP_HEIGHT);
}

bool camera_ut_is_win_opened ()
{
	Camera *this = camera_get_instance();

	return (this->interface_opened >= 0
	&& 		this->interface_opened <  7);
}

bool camera_ut_loading_state ()
{
	Camera *this = camera_get_instance();

	int state = read_memory_as_int(this->mp->proc, this->loading_state_addr);

	return (state > 0);
}
