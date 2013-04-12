#include "LoLCamera.h"

// Todo : Find a userfriendly way to put TOGGLE_KEY in the .ini
#define TOGGLE_KEY	 VK_F11   // which key toggles the camera adjustments

// Singleton
static Camera *this = NULL;

static BOOL camera_is_enabled ()
{
	static short last_toggle_state = 0;
	static BOOL space_pressed = FALSE;
	static BOOL left_button_pressed = FALSE;

	// listen for toggle key
	short new_toggle_state = GetKeyState(TOGGLE_KEY);
	if (new_toggle_state != last_toggle_state && new_toggle_state >= 0)
	{
		this->enabled = !this->enabled;
		last_toggle_state = new_toggle_state;

		// Enable / Disable patches
		patch_set_active(this->border_screen, this->enabled);
		patch_set_active(this->F2345_pressed[0], this->enabled);
		patch_set_active(this->F2345_pressed[1], this->enabled);
		patch_set_active(this->respawn_reset, this->enabled);
	}

	// Disable when space is pressed
	if (GetKeyState(VK_SPACE) < 0 || (GetKeyState(VK_F1) < 0))
	{
		/*	BUGFIX:
		*	bug :	When space (or F1) is kept pressed, LoLCamera saves the last camera position (before it gets centered on the champion)
		*		  	When space (or F1) is released, the camera returns in the last position saved -> weird camera moves
		*	fix :	request polling data for the next loop, so the data saved are synchronized with the new camera
		*/
		space_pressed = TRUE;
		return 0;
	}
	else if (space_pressed == TRUE) // on release
	{
		this->request_polling = TRUE;
		space_pressed = FALSE;
	}

	// to allow minimap navigation, also disabled if LMB is down
	if (GetKeyState(VK_LBUTTON) < 0)
	{
		/*	BUGFIX:
		*	bug: 	When the left mouse button is pressed on the camera and then released, the camera travels all the distance
		*			from the old position to the champion
		*	fix:	???
		*/
		if (!left_button_pressed) {
			left_button_pressed = TRUE;
		}

		return 0;
	}
	else
	{
		if (left_button_pressed) {
		}

		left_button_pressed = FALSE;
	}

	// Disable camera when shop opened
	if (camera_shop_is_opened()) {
		return 0;
	}


	// skip the next loop if not enabled
	return this->enabled;
}

void camera_load_ini ()
{
	// Loading parameters from .ini file :
	IniParser *parser = ini_parser_new("LoLCamera.ini");
	ini_parser_reg_and_read(parser);

	if (this->mp->base_addr == 0)
	{
		warning("Base address not found. Using default (0x00400000)");
		this->mp->base_addr = 0x00400000;
	}

	DWORD camx_addr   = strtol(ini_parser_get_value(parser, "camera_posx_addr"), NULL, 16);
	DWORD camy_addr   = strtol(ini_parser_get_value(parser, "camera_posy_addr"), NULL, 16);
	DWORD champx_addr = strtol(ini_parser_get_value(parser, "champion_posx_addr"), NULL, 16);
	DWORD champy_addr = strtol(ini_parser_get_value(parser, "champion_posy_addr"), NULL, 16);
	DWORD mousex_addr = strtol(ini_parser_get_value(parser, "mouse_posx_addr"), NULL, 16);
	DWORD mousey_addr = strtol(ini_parser_get_value(parser, "mouse_posy_addr"), NULL, 16);
	DWORD destx_addr  = strtol(ini_parser_get_value(parser, "dest_posx_addr"), NULL, 16);
	DWORD desty_addr  = strtol(ini_parser_get_value(parser, "dest_posy_addr"), NULL, 16);

	this->lerp_rate	  = atof  (ini_parser_get_value(parser, "lerp_rate")); // this controls smoothing, smaller values mean slower camera movement
	this->threshold	  = atof  (ini_parser_get_value(parser, "threshold")); // minimum threshold before calculations halted because camera is "close enough"
	this->sleep_time  = strtol(ini_parser_get_value(parser, "sleep_time"), NULL, 10); // Time slept between two camera updates (in ms)
	this->poll_data	  = strtol(ini_parser_get_value(parser, "poll_data"), NULL, 5); // Retrieve data from client every X loops
	this->mouse_range_max = atof(ini_parser_get_value(parser, "mouse_range_max"));
	this->dest_range_max  = atof(ini_parser_get_value(parser, "dest_range_max"));
	this->shop_is_opened_addr = strtol(ini_parser_get_value(parser, "shop_is_opened_addr"), NULL, 16);
	this->respawn_reset_addr = strtol(ini_parser_get_value(parser, "respawn_reset_addr"), NULL, 16);
	this->border_screen_addr = strtol(ini_parser_get_value(parser, "border_screen_addr"), NULL, 16);
	this->allies_cam_addr[0] = strtol(ini_parser_get_value(parser, "allies_cam_addr0"), NULL, 16);
	this->allies_cam_addr[1] = strtol(ini_parser_get_value(parser, "allies_cam_addr1"), NULL, 16);
	this->self_cam_addr = strtol(ini_parser_get_value(parser, "self_cam_addr"), NULL, 16);
	this->entities_array_addr = strtol(ini_parser_get_value(parser, "entities_array_addr"), NULL, 16);

	// Input checking
	if (!this->sleep_time) this->sleep_time = 1;
	if (!this->poll_data)  this->poll_data  = 5;

	// Init data from the client
	this->cam   = mempos_new(this->mp, camx_addr,   camy_addr);
	this->champ = mempos_new(this->mp, champx_addr, champy_addr);
	this->mouse = mempos_new(this->mp, mousex_addr, mousey_addr);
	this->dest  = mempos_new(this->mp, destx_addr,  desty_addr);

	ini_parser_free(parser);
}

void camera_init (MemProc *mp)
{
	if (this == NULL)
		this = calloc(sizeof(Camera), 1);

	this->enabled = TRUE;
	this->mp = mp;
	this->active = FALSE;

	this->F2345_pressed[0] = NULL;
	this->F2345_pressed[1] = NULL;

	camera_load_ini();

	// We wait for the client to be fully ready (in game) before patching
	this->request_polling = TRUE;
	while (!camera_update()) {
		warning("Loading screen detected");
	}

	// Signature scanning
	camera_scan_champions();
	camera_scan_patch();

	patch_set_active(this->border_screen, TRUE);
	patch_set_active(this->F2345_pressed[0], TRUE);
	patch_set_active(this->F2345_pressed[1], TRUE);
	patch_set_active(this->respawn_reset, TRUE);

	this->active = TRUE;
}

BOOL camera_refresh_champions ()
{
	// We can't rely on this information to decide if LoLCam is synchronized;
	// MemPos already do that job correctly
	// returns TRUE so it's ignored in camera_update()
	if (!this->active)
		return TRUE;

	for (int i = 0; i < 5; i++)
	{
		if (!entity_refresh(this->champions[i]))
		{
			if (this->champions[i])
				warning("Entity 0x%.8x cannot be refreshed", this->champions[i]->addr);

			return FALSE;
		}
	}

	return TRUE;
}

inline void camera_set_active (BOOL active)
{
	this->active = active;
}

BOOL camera_update ()
{
	static unsigned int frame_count = 0;
	static BOOL trying_sync = FALSE;

	if (frame_count++ % this->poll_data == 0 || this->request_polling)
	{
		if (!mempos_refresh(this->cam)
		||  !mempos_refresh(this->champ)
		||  !mempos_refresh(this->dest)
		||  !mempos_refresh(this->mouse)
		||  !camera_refresh_champions())
		{
			// Synchronization seems not possible
			if (!memproc_refresh_handle(this->mp))
			{
				info("Client not detected anymore.");
				this->active = FALSE;
				return FALSE;
			}

			trying_sync = TRUE;
			warning("Synchronization with the client isn't possible - Retrying in 3s.");
			Sleep(3000);
			return FALSE;
		}

		if (trying_sync) {
			info("LoLCamera is working now\n");
			trying_sync = FALSE;
		}

		this->request_polling = FALSE;
	}

	return TRUE;
}

void camera_main ()
{
	Vector2D target;

	while (this->active)
	{
		Sleep(this->sleep_time);

		// Check if enabled
		if (!camera_is_enabled())
			continue;

		 // Retrieve the positions from the client
		if (!camera_update())
			continue;

        float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);
        float distance_dest_champ = vector2D_distance(&this->dest->v, &this->champ->v);
        {
            // weighted averages
            float champ_weight = 1.0;
            float mouse_weight = 1.0;
            float dest_weight = 1.0;

            // these values control how quickly the weights fall off the further you are
            // from the ceiling distance
            float dest_falloff_rate = 0.001;
            float mouse_falloff_rate = 0.002;

            // adjust weights based on distance
            if (distance_dest_champ > this->dest_range_max)
                dest_weight = 1 / (((distance_dest_champ - this->dest_range_max) * dest_falloff_rate) + 1.0);
            if (distance_mouse_champ > this->mouse_range_max)
                mouse_weight = 1 / (((distance_mouse_champ - this->mouse_range_max) * mouse_falloff_rate) + 1.0);

            float weight_sum = champ_weight + mouse_weight + dest_weight;

            // Compute the target (weighted averages)
			vector2D_set_pos(&target,
                (
                    (this->champ->v.x * champ_weight) +
                    (this->mouse->v.x * mouse_weight) +
                    (this->dest->v.x * dest_weight)
                 ) / weight_sum,
                (
                    (this->champ->v.y * champ_weight) +
                    (this->mouse->v.y * mouse_weight) +
                    (this->dest->v.y * dest_weight)
                ) / weight_sum
            );

            // The camera goes farther when the camera is moving to the south
            if (this->mouse->v.y < this->champ->v.y)
                target.y -= distance_mouse_champ / 10.0; // <-- 10.0 is an arbitrary value
        }

		// Smoothing
		if (abs(target.x - this->cam->v.x) > this->threshold)
			this->cam->v.x += (target.x - this->cam->v.x) * this->lerp_rate;

		if (abs(target.y - this->cam->v.y) > this->threshold)
			this->cam->v.y += (target.y - this->cam->v.y) * this->lerp_rate;

		// Update the camera client
		mempos_set(this->cam, this->cam->v.x, this->cam->v.y);
	}
}

inline Camera *camera_get_instance ()
{
	return this;
}

void camera_unload ()
{
	// Process still active, unpatch
	if (memproc_refresh_handle(this->mp))
	{
		patch_set_active(this->border_screen, FALSE);
		patch_set_active(this->F2345_pressed[0], FALSE);
		patch_set_active(this->F2345_pressed[1], FALSE);
		patch_set_active(this->respawn_reset, FALSE);
	}
}
