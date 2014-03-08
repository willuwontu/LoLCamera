#include "LoLCamera.h"

bool camera_ut_campos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->cam, &x, &y);

	return out_of_map(x, y) == false;
}

bool camera_ut_champos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->champ, &x, &y);

	return out_of_map(x, y) == false;
}

bool camera_ut_mousepos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get(this->mouse, &x, &y);

	return out_of_map(x, y) == false;
}

bool camera_ut_destpos ()
{
	Camera *this = camera_get_instance();
	float x, y;

	mempos_get (this->dest, &x, &y);

	return out_of_map(x, y) == false;
}

bool camera_ut_is_win_opened ()
{
	Camera *this = camera_get_instance();

	return (this->interface_opened >= 0
	&& 		this->interface_opened <  7);
}

bool camera_ut_entities ()
{
    Camera *this = camera_get_instance();

    if (this->playersCount <= 0 || this->playersCount > 10)
        return false;

    for (int i = 0; i < this->playersCount; i++)
    {
        Entity *entity = this->champions[i];

        if (out_of_map(entity->p.v.x, entity->p.v.y) == false)
            return false;
    }

    return true;
}
