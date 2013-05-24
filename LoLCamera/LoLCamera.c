#include "LoLCamera.h"

// Todo : Find a userfriendly way to put TOGGLE_KEY in the .ini
#define TOGGLE_KEY	 VK_F11   // which key toggles the camera adjustments

// Singleton
static Camera *this = NULL;

typedef enum {

    Normal,
    CenterCam,
    NoMove,
    NoUpdate,
    FollowAlly,
    ShareAlly,
    Free,
    Drag

} CameraTrackingMode;

// Static functions declaration
static void camera_compute_target (Vector2D *target, CameraTrackingMode camera_mode);
static void camera_compute_lerp_rate (float *lerp_rate, CameraTrackingMode camera_mode);
BOOL camera_entity_is_near (Entity *e);


void camera_focus_entity (Entity *e)
{
	this->focused_ally = e;
}

static CameraTrackingMode camera_is_enabled ()
{
	static short last_toggle_state = 0;
	static int mbutton_pressed = 0;
	static int lbutton_pressed = 0;
	BOOL champ_is_dead = entity_is_dead(this->champions[0]);

	// listen for toggle key
	short new_toggle_state = GetKeyState(TOGGLE_KEY);
	if (new_toggle_state != last_toggle_state && new_toggle_state >= 0)
	{
		this->enabled = !this->enabled;
		last_toggle_state = new_toggle_state;

		// Enable / Disable patches
		patch_set_activated(this->border_screen, this->enabled);
		patch_set_activated(this->F2345_pressed[0], this->enabled);
		patch_set_activated(this->F2345_pressed[1], this->enabled);
		patch_set_activated(this->respawn_reset, this->enabled);
		patch_set_activated(this->locked_camera, this->enabled);
		patch_set_activated(this->minimap[0], this->enabled);
		patch_set_activated(this->minimap[1], this->enabled);
	}

	// skip the next loop if not enabled
	if (!this->enabled)
        return NoUpdate;

	// Disable when space / F1 is pressed
	if (GetKeyState(VK_SPACE) < 0 || (GetKeyState(VK_F1) < 0))
    {
    	// Polling data is requested because we want to center the camera exactly where the champion is
    	// Also disable the focusing champ feature if enabled
        this->request_polling = TRUE;
        this->focused_ally = NULL;
        return CenterCam;
    }

	// to allow minimap navigation, also disabled if LMB is down
	if (GetKeyState(VK_LBUTTON) < 0)
	{
		// Attempt to fix issue #8 (Minimap click-hold, then return problem)
		float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);

		if (distance_mouse_champ > 1000.0)
		// we don't want to stop the camera when we click around the champion
		{
			switch (lbutton_pressed)
			{
				case 0:
					// Force polling
					this->request_polling = TRUE;
					lbutton_pressed = 1;
				break;

				case 1:
					memcpy(&this->cam_saved, this->cam, sizeof(MemPos));
					lbutton_pressed = 2;
				break;

				case 2:
				break;
			}

			return NoMove;
		}
	}

	else
	{
		if (lbutton_pressed == 2)
		{
			// On release
			Sleep(1);
			mempos_set(this->cam, this->cam_saved.v.x, this->cam_saved.v.y);
		}

		lbutton_pressed = 0;
	}

	// Disable camera when shop opened
	if (this->shop_opened)
        return NoMove;

	// Drag : TODO
	if (GetKeyState(VK_MBUTTON) < 0)
	{
		switch (mbutton_pressed)
		{
			case 0:
				this->drag_pos = this->mouse->v;
				this->request_polling = TRUE;
				mbutton_pressed = 1;
			break;

			case 1:
				this->drag_pos = this->mouse->v;
				mbutton_pressed = 2;
			break;

			case 2:
			break;
		}

		return Drag;
	}
	else
		mbutton_pressed = 0;

	// Following ally & ennemies champions
	if (GetKeyState(VK_F2)  < 0 && this->team_size > 1) camera_focus_entity(this->champions[1]);
	if (GetKeyState(VK_F3)  < 0 && this->team_size > 2) camera_focus_entity(this->champions[2]);
	if (GetKeyState(VK_F4)  < 0 && this->team_size > 3) camera_focus_entity(this->champions[3]);
	if (GetKeyState(VK_F5)  < 0 && this->team_size > 4) camera_focus_entity(this->champions[4]);
	if (GetKeyState(VK_F6)  < 0 && this->team_size > 5) camera_focus_entity(this->champions[5]);
	if (GetKeyState(VK_F7)  < 0 && this->team_size > 6) camera_focus_entity(this->champions[6]);
	if (GetKeyState(VK_F8)  < 0 && this->team_size > 7) camera_focus_entity(this->champions[7]);
	if (GetKeyState(VK_F9)  < 0 && this->team_size > 8) camera_focus_entity(this->champions[8]);
	if (GetKeyState(VK_F10) < 0 && this->team_size > 9) camera_focus_entity(this->champions[9]);

	if (this->focused_ally != NULL)
	{
		// When we are in "dead" spectator mode, it's not important to change camera mode.
		if (entity_is_dead(this->focused_ally) && !champ_is_dead)
			this->focused_ally = NULL;
		else
		{
			if (camera_entity_is_near(this->focused_ally))
				return ShareAlly;
			else
				return FollowAlly;
		}
	}

	// If our champion is dead, set free mode
	if (champ_is_dead)
		return Free;

    return Normal;
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
	this->focused_ally = NULL;
	this->shop_opened = FALSE;
	this->drag_pos = vector2D_new();

	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	// Zeroing
	memset(this->champions, 0, sizeof(Entity *));

	// Read static vars from .ini
	camera_load_ini();

	// Get loading screen address
	info("Dumping process...");
	memproc_dump(this->mp, text_section, text_section + text_size);
	camera_scan_loading();

	// Wait for client ingame
	if (camera_wait_for_ingame())
	{
		// Dumping the process again (loading screen detected)
		memproc_clear(this->mp);
		info("Dumping process again after loading screen...");
		memproc_dump(this->mp, text_section, text_section + text_size);
	}

	// Scanning for variables
	camera_scan_variables();

	// Signature scanning for patches
	camera_scan_patch();

	// Init data from the client
	this->cam   	   = mempos_new (this->mp, this->camx_addr,     this->camy_addr);
	this->champ 	   = mempos_new (this->mp, this->champx_addr,   this->champy_addr);
	this->mouse 	   = mempos_new (this->mp, this->mousex_addr,   this->mousey_addr);
	this->dest  	   = mempos_new (this->mp, this->destx_addr,    this->desty_addr);

	this->mouse_screen = mempos_int_new (
		this->mp,
		this->mouse_screen_addr + 0x4C - mp->base_addr,
		this->mouse_screen_addr + 0x50 - mp->base_addr
	);

	patch_set_activated(this->border_screen, TRUE);
	patch_set_activated(this->F2345_pressed[0], TRUE);
	patch_set_activated(this->F2345_pressed[1], TRUE);
	patch_set_activated(this->respawn_reset, TRUE);
	patch_set_activated(this->locked_camera, TRUE);
	patch_set_activated(this->minimap[0], TRUE);
	patch_set_activated(this->minimap[1], TRUE);

	this->active = TRUE;
}

BOOL camera_wait_for_ingame ()
{
	BOOL waited = FALSE;

	while (!read_memory_as_int(this->mp->proc, this->loading_state_addr))
	{
		waited = TRUE;
		warning("Loading screen detected. Sleep during 3s.");
		Sleep(3000);
	}

	return waited;
}

inline void camera_set_active (BOOL active)
{
	this->active = active;
}

BOOL camera_update ()
{
	static unsigned int frame_count = 0;

	struct refreshFunctions { BOOL (*func)(); void *arg; char *desc; } refresh_funcs [] =
	{
		{.func = mempos_refresh, 				.arg = this->cam,  			.desc = "this->cam MemPos"},
		{.func = mempos_refresh, 				.arg = this->champ,			.desc = "this->champ MemPos"},
		{.func = mempos_refresh,				.arg = this->dest, 			.desc = "this->dest MemPos"},
		{.func = mempos_refresh,				.arg = this->mouse, 		.desc = "this->mouse MemPos"},
		{.func = mempos_int_refresh,			.arg = this->mouse_screen,	.desc = "this->mouse_screen MemPos"},
		{.func = camera_refresh_champions,		.arg = NULL,				.desc = "Entities array"},
		{.func = camera_refresh_shop_is_opened,	.arg = NULL,				.desc = "Shop opened"},
	};

	if (frame_count++ % this->poll_data == 0 || this->request_polling)
	{
		for (int i = 0; i < sizeof(refresh_funcs) / sizeof(struct refreshFunctions); i++)
		{
			BOOL (*func)() = refresh_funcs[i].func;
			void *arg = refresh_funcs[i].arg;
			char *desc = refresh_funcs[i].desc;

			if (!(func(arg)))
			{
				warning("\"%s\" : Refresh failed", desc);

				//  Detect if client is disconnected
				if (!memproc_refresh_handle(this->mp))
				{
					info("Client not detected anymore.");
					this->active = FALSE;
					return FALSE;
				}

				// Resynchronize with the process
				if (!camera_scan_variables()
				||	!camera_scan_mouse_screen()
				||	!camera_scan_champions())
				{
					warning("Synchronization with the client isn't possible - Retrying in 3s.");
					Sleep(3000);
				}
				else
				{
					info("LoLCamera re-sync done");
					return FALSE;
				}

				return FALSE;
			}
		}

		this->request_polling = FALSE;
	}

	return TRUE;
}

void camera_set_tracking_mode (CameraTrackingMode *out_mode)
{
	CameraTrackingMode mode;
	static CameraTrackingMode last_mode = Normal;

	mode = camera_is_enabled();

	if (mode == FollowAlly && last_mode == ShareAlly)
	{
		// We don't want to focus on an entity when it goes too far, we want to focus on our champion (Normal mode)
		mode = Normal;
		this->focused_ally = NULL;
	}

	last_mode = mode;
	*out_mode = mode;
}

void camera_main ()
{
	Vector2D target;
	float lerp_rate;
	CameraTrackingMode camera_mode;

	while (this->active)
	{
		Sleep(this->sleep_time);

		// Check if enabled.
		camera_set_tracking_mode(&camera_mode);

		if (camera_mode == NoUpdate || !camera_update())
			continue;

		// Compute target
		camera_compute_target(&target, camera_mode);

		// Compute lerp rate
		camera_compute_lerp_rate(&lerp_rate, camera_mode);

        // Smoothing
		if (abs(target.x - this->cam->v.x) > this->threshold)
			this->cam->v.x += (target.x - this->cam->v.x) * lerp_rate;

		if (abs(target.y - this->cam->v.y) > this->threshold)
			this->cam->v.y += (target.y - this->cam->v.y) * lerp_rate;

		// Keep this just before mempos_set(this->cam, x, y)
        if (camera_mode == NoMove)
            continue;

        // update the ingame gamera position
		mempos_set(this->cam, this->cam->v.x, this->cam->v.y);
	}
}

static void camera_compute_lerp_rate (float *lerp_rate, CameraTrackingMode camera_mode)
{
	float local_lerp_rate = this->lerp_rate;

	switch (camera_mode)
	{
		case CenterCam:
			// adjust camera smoothing rate when center camera
				local_lerp_rate = local_lerp_rate * 5;

			// clamp the lerp rate
			if (local_lerp_rate > 0.9)
				local_lerp_rate = 0.9;
		break;

		case Free:
			local_lerp_rate = local_lerp_rate * 2;
		break;

		case FollowAlly:
			local_lerp_rate = local_lerp_rate * 5;
		break;

		case ShareAlly:
		break;

		case Normal:
		break;

		case NoMove:
		case NoUpdate:
		break;

		case Drag:
		break;

		default:
		break;
	}

	*lerp_rate = local_lerp_rate;
}

BOOL camera_entity_is_near (Entity *e)
{
	if (e == NULL)
		return FALSE;

	float distance_ally_champ = vector2D_distance(&e->p.v, &this->champ->v);

	return (distance_ally_champ < 2000.0);
}

void camera_compute_target (Vector2D *target, CameraTrackingMode camera_mode)
{
	float ally_weight = 0.0;
	float ally_x = 0.0, ally_y = 0.0;
	float drag_x = 0.0, drag_y = 0.0;

	switch (camera_mode)
	{
		case Free:
			vector2D_set_pos(target,
                (
                    (this->mouse->v.x) +
                    (this->cam->v.x)
                 ) / 2.0,
                (
                    (this->mouse->v.y) +
                    (this->cam->v.y)
                ) / 2.0
            );
		break;


		case FollowAlly:
		{
			Entity *ally = this->focused_ally;

			if (!camera_entity_is_near(ally))
			{
				vector2D_set_pos(target, ally->p.v.x, ally->p.v.y);
				break;
			}
		}
		break;

		case Drag:
			drag_x = (this->drag_pos.x - this->mouse->v.x) * 10;
			drag_y = (this->drag_pos.y - this->mouse->v.y) * 10;
			goto ShareAllyMode; // Drag *must* follow with case ShareAlly then NormalMode

		ShareAllyMode:
		case ShareAlly:
			if (this->focused_ally != NULL)
			{
				// ShareAlly is a Normal camera behavior + ally weight
				ally_weight = 1.0;
				ally_x = this->focused_ally->p.v.x * ally_weight;
				ally_y = this->focused_ally->p.v.y * ally_weight;
			}
			goto NormalMode; // We must go to case Normal

		NormalMode:
		case Normal:
		{
			float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);
			float distance_dest_champ  = vector2D_distance(&this->dest->v, &this->champ->v);
			float distance_mouse_dest  = vector2D_distance(&this->dest->v, &this->mouse->v);

			float champ_weight = 1.0;
			float mouse_weight = 1.0;
			float dest_weight  = 1.0;

			float weight_sum;
			{
				// weighted averages
				// these values control how quickly the weights fall off the further you are
				// from the falloff distance
				float dest_falloff_rate  = 0.001;
				float mouse_falloff_rate = 0.002;

				// adjust weights based on distance
				if (distance_dest_champ > this->dest_range_max)
					dest_weight = 1 / (((distance_dest_champ - this->dest_range_max) * dest_falloff_rate) + 1.0);

				if (distance_mouse_champ > this->mouse_range_max)
					mouse_weight = 1 / (((distance_mouse_champ - this->mouse_range_max) * mouse_falloff_rate) + 1.0);

				else if (distance_mouse_dest > this->mouse_dest_range_max)
					// increase mouse weight if far from dest
					mouse_weight += (distance_mouse_dest - this->mouse_dest_range_max) / 1000.0;

				weight_sum = champ_weight + mouse_weight + dest_weight + ally_weight;
			}

            // Compute the target (weighted averages)
			vector2D_set_pos(target,
                (
                    (this->champ->v.x * champ_weight) +
                    (this->mouse->v.x * mouse_weight) +
                    (this->dest->v.x * dest_weight) +
                    (ally_x * ally_weight)
                 ) / weight_sum
					+ drag_x,
                (
                    (this->champ->v.y * champ_weight) +
                    (this->mouse->v.y * mouse_weight) +
                    (this->dest->v.y * dest_weight) +
                    (ally_y * ally_weight)
                ) / weight_sum
					+ drag_y
            );

            // The camera goes farther when the camera is moving to the south
            if (this->mouse->v.y < this->champ->v.y)
                target->y -= distance_mouse_champ / 8.0; // <-- arbitrary value
		}
		break;

		case CenterCam:
			// Target the champion
            vector2D_set_pos(target, this->champ->v.x, this->champ->v.y);
		break;

		case NoMove:
		case NoUpdate:
			// Do nothing, this code isn't going to be reached anyway
		break;

		default:
			warning("Unknown camera behavior (%d)", camera_mode);
		break;
	}
}

void camera_load_ini ()
{
	// Loading parameters from .ini file :
	IniParser *parser = ini_parser_new("LoLCamera.ini");
	ini_parser_reg_and_read(parser);

	// Addresses
	this->camx_addr   = strtol(ini_parser_get_value(parser, "camera_posx_addr"), NULL, 16);
	this->camy_addr   = strtol(ini_parser_get_value(parser, "camera_posy_addr"), NULL, 16);
	this->champx_addr = strtol(ini_parser_get_value(parser, "champion_posx_addr"), NULL, 16);
	this->champy_addr = strtol(ini_parser_get_value(parser, "champion_posy_addr"), NULL, 16);
	this->mousex_addr = strtol(ini_parser_get_value(parser, "mouse_posx_addr"), NULL, 16);
	this->mousey_addr = strtol(ini_parser_get_value(parser, "mouse_posy_addr"), NULL, 16);
	this->destx_addr  = strtol(ini_parser_get_value(parser, "dest_posx_addr"), NULL, 16);
	this->desty_addr  = strtol(ini_parser_get_value(parser, "dest_posy_addr"), NULL, 16);
	this->mouse_screen_ptr = strtol(ini_parser_get_value(parser, "mouse_screen_ptr"), NULL, 16);
	this->shop_is_opened_ptr = strtol(ini_parser_get_value(parser, "shop_is_opened_ptr"), NULL, 16);
	this->respawn_reset_addr = strtol(ini_parser_get_value(parser, "respawn_reset_addr"), NULL, 16);
	this->border_screen_addr = strtol(ini_parser_get_value(parser, "border_screen_addr"), NULL, 16);
	this->allies_cam_addr[0] = strtol(ini_parser_get_value(parser, "allies_cam_addr0"), NULL, 16);
	this->allies_cam_addr[1] = strtol(ini_parser_get_value(parser, "allies_cam_addr1"), NULL, 16);
	this->self_cam_addr = strtol(ini_parser_get_value(parser, "self_cam_addr"), NULL, 16);
	this->entities_addr = strtol(ini_parser_get_value(parser, "entities_addr"), NULL, 16);
	this->entities_addr_end = strtol(ini_parser_get_value(parser, "entities_addr_end"), NULL, 16);
	this->locked_camera_addr = strtol(ini_parser_get_value(parser, "locked_camera_addr"), NULL, 16);
	this->loading_state_addr = strtol(ini_parser_get_value(parser, "loading_state_addr"), NULL, 16);

	// Settings
	this->lerp_rate	  = atof  (ini_parser_get_value(parser, "lerp_rate")); // this controls smoothing, smaller values mean slower camera movement
	this->threshold	  = atof  (ini_parser_get_value(parser, "threshold")); // minimum threshold before calculations halted because camera is "close enough"
	this->sleep_time  = strtol(ini_parser_get_value(parser, "sleep_time"), NULL, 10); // Time slept between two camera updates (in ms)
	this->poll_data	  = strtol(ini_parser_get_value(parser, "poll_data"), NULL, 10); // Retrieve data from client every X loops
	this->mouse_range_max = atof(ini_parser_get_value(parser, "mouse_range_max"));
	this->dest_range_max  = atof(ini_parser_get_value(parser, "dest_range_max"));
	this->mouse_dest_range_max  = atof(ini_parser_get_value(parser, "mouse_dest_range_max"));

	// Addresses - Input checking
	struct AddrStr { DWORD addr; char *str; } tabAddr [] = {
        { .addr = this->shop_is_opened_ptr, .str = "shop_is_opened_ptr" }, //
        { .addr = this->respawn_reset_addr, .str = "respawn_reset_addr" }, //
        { .addr = this->border_screen_addr, .str = "border_screen_addr" }, //                  ██████████
        { .addr = this->champx_addr,        .str = "champion_posx_addr" }, //              ████░░      ░░████
        { .addr = this->champy_addr,        .str = "champion_posy_addr" }, //            ██░░              ░░██
        { .addr = this->locked_camera_addr, .str = "locked_camera_addr" }, //          ██                    ░░██
        { .addr = this->mouse_screen_ptr,   .str = "mouse_screen_ptr" },   //        ██              ██  ██  ░░██
                                                                           //      ██░░              ██  ██      ██
        { .addr = this->camx_addr,          .str = "camera_posx_addr" },   //      ██                ██  ██      ██
        { .addr = this->camy_addr,          .str = "camera_posy_addr" },   //      ██          ░░░░        ░░░░  ██
        { .addr = this->allies_cam_addr[0], .str = "allies_cam_addr0" },   //      ██░░    ░░                ░░  ██
        { .addr = this->allies_cam_addr[1], .str = "allies_cam_addr1" },   //        ██░░  ██        ██      ██░░██
        { .addr = this->mousex_addr,        .str = "mouse_posx_addr" },    //          ████░░              ░░████
        { .addr = this->mousey_addr,        .str = "mouse_posy_addr" },    //            ████░░░░        ░░████
        { .addr = this->destx_addr,         .str = "dest_posx_addr" },     //          ██░░░░██████████████░░░░██
        { .addr = this->desty_addr,         .str = "dest_posy_addr" },     //        ██░░░░░░░░░░████████░░░░░░░░██
        { .addr = this->self_cam_addr,      .str = "self_cam_addr" },      //          ██████████        ████████
        { .addr = this->entities_addr,      .str = "entities_addr" },      //
        { .addr = this->entities_addr_end,  .str = "entities_addr_end" },  //
        { .addr = this->loading_state_addr, .str = "loading_state_addr" }, //
	};

	// Todo : corresponding scanning function corresponding to each data to get directly in the client
	for (int i = 0; i < sizeof(tabAddr) / sizeof(struct AddrStr); i++)
		if (!tabAddr[i].addr)
			warning("\"%s\" cannot be read in .ini file", tabAddr[i].str);

	// Settings - Input checking
	struct SettingVal {
		union {float *f; int *i;} p;
		union {float  f; int  i;} v;
	} tabSet [] = {
		// If the settings is not found in the .ini, set the value to its default value :
		{ .p.i = &this->sleep_time,	.v.i = 1.0},
		{ .p.i = &this->poll_data,	.v.i = 5.0}
	};

	for (int i = 0; i < sizeof(tabSet) / sizeof(struct SettingVal); i++)
	{
		if (*(tabSet[i].p.i) == 0)
			(*tabSet[i].p.i) = tabSet[i].v.i;
	}

	// Cleaning
	ini_parser_free(parser);
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
		patch_set_activated(this->border_screen, FALSE);
		patch_set_activated(this->F2345_pressed[0], FALSE);
		patch_set_activated(this->F2345_pressed[1], FALSE);
		patch_set_activated(this->minimap[0], FALSE);
		patch_set_activated(this->minimap[1], FALSE);
		patch_set_activated(this->respawn_reset, FALSE);
		patch_set_activated(this->locked_camera, FALSE);
	}
}
