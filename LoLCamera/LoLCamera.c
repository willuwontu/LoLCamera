#include "LoLCamera.h"

// Todo : Find a userfriendly way to put TOGGLE_KEY in the .ini
#define TOGGLE_KEY	 VK_F11   // which key toggles the camera adjustments

// Singleton
static Camera *this = NULL;

typedef enum {

    Normal,
    CenterCam,
    RestoreCam,
    NoMove,
    NoUpdate,
    EndOfGame,
    ForeGround,
    Free,
    FocusSelf,
    FollowEntity,
    Translate,
    isTranlating

} CameraTrackingMode;

// Static functions declaration
static void camera_compute_target (Vector2D *target, CameraTrackingMode camera_mode);
static void camera_compute_lerp_rate (float *lerp_rate, CameraTrackingMode camera_mode);
BOOL camera_entity_is_near (Entity *e, float limit);
static BOOL camera_follow_champion_requested ();
static BOOL camera_restore_requested ();
static void camera_toggle (BOOL enable);
static BOOL camera_is_translated ();
static void camera_translate_toggle (BOOL enable);

static void camera_toggle (BOOL enable)
{
	this->enabled = !enable;

	// Enable / Disable patches
	patch_list_set(this->patchlist, this->enabled);
}


static BOOL camera_is_enabled ()
{
	short toggle_state = GetKeyState(TOGGLE_KEY);

	// listen for toggle key
	if (toggle_state != this->last_toggle_state && toggle_state < 0)
	{
		this->last_toggle_state = toggle_state;
		camera_toggle(this->enabled);
	}

	// skip the next loop if not enabled
	return this->enabled;
}

static void camera_translation_reset ()
{
	vector2D_set_pos(&this->distance_translation, 0.0, 0.0);
}

static BOOL camera_center_requested ()
{
	// Disable when space / F1 is pressed
	if ((GetKeyState(VK_SPACE) < 0 || (GetKeyState(VK_F1) < 0)) && (this->interface_opened != LOLCAMERA_CHAT_OPENED_VALUE))
    {
    	// Polling data is requested because we want to center the camera exactly where the champion is
        this->request_polling = TRUE;
        this->focused_entity = NULL;
        this->hint_entity = NULL;
        camera_translation_reset();
        return TRUE;
    }

    return FALSE;
}

static BOOL camera_restore_requested ()
{
	return this->restore_tmpcam;
}

static void camera_save_state ()
{
	memcpy(&this->tmpcam, this->cam, sizeof(this->cam));
	memcpy(&this->tmpcam.v, &this->cam->v, sizeof(this->cam->v));
}

static BOOL camera_left_click ()
{
	if (GetKeyState(VK_LBUTTON) < 0)
	{
		// Attempt to fix issue #8 (Minimap click-hold, then return problem)
		float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);

		// we don't want to stop the camera when we click around the champion
		if (distance_mouse_champ > 2500.0)
		{
			switch (this->lbutton_state)
			{
				case 0:
					// Force polling before saving the camera
					this->request_polling = TRUE;
					this->lbutton_state = 1;
				break;

				case 1:
					camera_save_state();
					this->lbutton_state = 2;
				break;

				case 2:
				break;
			}

			return TRUE;
		}
	}

	else
	{
		if (this->lbutton_state == 2)
		{
			// On release
			this->restore_tmpcam = TRUE;
		}

		this->lbutton_state = 0;
	}


	return FALSE;
}

static void camera_translate_toggle (int state)
{
	switch (state)
	{
		case 0:
			vector2D_set_zero(&this->distance_translation);
			this->translate_request = 1;
		break;

		case 1:
			this->distance_translation.x = this->cam->v.x - this->champ->v.x;
			this->distance_translation.y = this->cam->v.y - this->champ->v.y;
			this->translate_request = 0;
		break;
	}
}

static BOOL camera_translate ()
{
	short translate_state = GetKeyState(this->translate_key);

	// listen for translate toggle key
	if (translate_state != this->last_translate_state && translate_state < 0
	&& this->interface_opened != LOLCAMERA_CHAT_OPENED_VALUE)
	{
		this->last_translate_state = translate_state;
		camera_translate_toggle(this->translate_request);
	}

	return this->translate_request;
}

static BOOL camera_is_translated ()
{
	return (
		this->distance_translation.x != 0.0
	&&  this->distance_translation.y != 0.0);
}

static void camera_debug_mode ()
{
	if (GetKeyState('D') < 0
	&&  GetKeyState('B') < 0
	&&  GetKeyState('G') < 0)
	{
		if (!this->dbg_mode)
		{
			// Debug mode activated
			this->dbg_mode = TRUE;
		}
	}

	else
	{
		if (this->dbg_mode)
		{
			info("------ Tests ------");
			// On release
			struct { char *str; BOOL (*fct)(); } unit_tests [] = {
				{"Camera Position",   camera_ut_campos},
				{"Champion Position", camera_ut_champos},
				{"Mouse position",    camera_ut_mousepos},
				{"Dest position",     camera_ut_destpos},
				{"Window opened", 	  camera_ut_is_win_opened},
				{"Screen mouse",      camera_ut_screen_mouse},
				{"Loading state",     camera_ut_loading_state}
			};

			for (int i = 0; i < sizeof(unit_tests) / sizeof(*unit_tests); i++)
			{
				info("Unit test %s : %s", unit_tests[i].str, (unit_tests[i].fct()) ? "OK" : "FAILED");
			}
		}

		this->dbg_mode = FALSE;
	}
}

static BOOL camera_window_is_active ()
{
	return (this->mp->hwnd == GetForegroundWindow());
}

BOOL camera_victory_state ()
{
	return (this->victory_state == 3);
}

static CameraTrackingMode camera_get_mode ()
{
	// End of game = end of LoLCamera
	if (camera_victory_state())
		return EndOfGame;

	if (!camera_window_is_active())
		return ForeGround;

	if (!camera_is_enabled())
        return NoUpdate;

	// user pressed space or F1 or anything requesting to center the camera on the champion
	if (camera_center_requested())
		return CenterCam;

	// A temporary camera has been saved, now is coming the time to restore it
	if (camera_restore_requested())
		return RestoreCam;

	if (camera_translate())
		return isTranlating;

	if (camera_is_translated())
		return Translate;

	// to allow minimap navigation, also disabled if LMB is down
	if (camera_left_click() || this->interface_opened == LOLCAMERA_SHOP_OPENED_VALUE)
		return NoMove;

	// Following ally & ennemies champions
	if (camera_follow_champion_requested ())
		return FollowEntity;

	// If our champion is dead, set free mode
	if (entity_is_dead(this->self))
		return Free;

	// The champion has been teleported far, focus on the champion
	if (!camera_is_near(this->champ, 3000.0) && !entity_is_dead(this->self))
		return FocusSelf;

    return Normal;
}

static BOOL camera_follow_champion_requested ()
{
	BOOL fx_pressed = FALSE;
	int keys[] = {VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10};

	for (int i = 1; i < 10; i++)
	{
		if (GetKeyState(keys[i]) < 0 && this->team_size > i)
		{
			this->followed_entity = this->champions[i];
			this->fxstate = (this->fxstate) ? this->fxstate : 1;
			fx_pressed = TRUE;
		}
	}

	if (!fx_pressed)
	{
		if (this->fxstate == 2)
			this->restore_tmpcam = TRUE;

		this->fxstate = 0;
	}

	if (this->fxstate == 1)
	{
		camera_save_state();
		this->fxstate = 2;
	}

	return fx_pressed;
}

void camera_init (MemProc *mp)
{
	if (this == NULL)
		this = calloc(sizeof(Camera), 1);

	this->enabled = TRUE;
	this->mp = mp;
	this->active = FALSE;
	this->drag_pos = vector2D_new();

	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	// Zeroing stuff
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
	this->dest  	   = (MemPos[]) {{
		.addrX = this->destx_addr,
		.addrY = this->desty_addr,
		.ctxt  = this->mp
	}};
	mempos_refresh(this->dest);

	this->mouse_screen = mempos_int_new (
		this->mp,
		this->mouse_screen_addr + 0x4C - mp->base_addr,
		this->mouse_screen_addr + 0x50 - mp->base_addr
	);

	patch_list_set(this->patchlist, TRUE);

	// Export to CE
	if (this->output_cheatengine_table)
		camera_export_to_cheatengine();

	// Load settings associated with champ name
	camera_load_settings(this->self->champ_name);
	ini_parser_free(this->parser);

	this->entities_nearby = bb_queue_new();
	this->active = TRUE;
}

BOOL camera_wait_for_ingame ()
{
	BOOL waited = FALSE;

	if (!this->wait_loading_screen)
		return TRUE;

	// Wait here
	while (!read_memory_as_int(this->mp->proc, this->loading_state_addr))
	{
		waited = TRUE;
		info("Loading screen detected - Retry in 3seconds.", this->loading_state_addr);
		Sleep(3000);
	}

	return waited;
}

inline void camera_set_active (BOOL active)
{
	this->active = active;
}

static void camera_entity_manager ()
{
	// An entity has been hovered - share the view (hint)
	if (this->entity_hovered != NULL)
	{
		this->hint_entity = this->entity_hovered;

		// Left click on the entity : focus it
		if (GetKeyState(VK_LBUTTON) < 0)
			this->focused_entity = this->entity_hovered;
	}

	if (this->focused_entity != NULL)
	{
		// When we are in "dead" spectator mode, it's not important to change camera mode.
		if (entity_is_dead(this->focused_entity)
		|| !camera_entity_is_near(this->focused_entity, 2000.0))
			this->focused_entity = NULL;
	}

	if (this->hint_entity != NULL)
	{
		if (entity_is_dead(this->hint_entity)
		|| !camera_entity_is_near(this->hint_entity, 2000.0))
			this->hint_entity = NULL;
	}
}

static void camera_middle_click ()
{
	if (GetKeyState(VK_MBUTTON) < 0)
	{
		// Drag
		switch (this->mbutton_state)
		{
			case 0:
				this->drag_pos = this->mouse->v;
				this->request_polling = TRUE;
				this->mbutton_state = 1;
			break;

			case 1:
				this->drag_pos = this->mouse->v;
				this->mbutton_state = 2;
			break;

			case 2:
			break;
		}

		this->drag_request = TRUE;
	}

	else
	{
		this->mbutton_state = 0;
		this->drag_request = FALSE;
	}
}

BOOL camera_update ()
{
	static unsigned int frame_count = 0;

	// Manage camera states
	camera_entity_manager();
	camera_middle_click();
	camera_debug_mode();

	struct refreshFunctions { BOOL (*func)(); void *arg; char *desc; } refresh_funcs [] =
	{
		{.func = mempos_refresh, 				.arg = this->cam,  			.desc = "this->cam MemPos"},
		{.func = mempos_refresh, 				.arg = this->champ,			.desc = "this->champ MemPos"},
		{.func = mempos_refresh,				.arg = this->dest, 			.desc = "this->dest MemPos"},
		{.func = mempos_refresh,				.arg = this->mouse, 		.desc = "this->mouse MemPos"},
		{.func = mempos_int_refresh,			.arg = this->mouse_screen,	.desc = "this->mouse_screen MemPos"},
		{.func = camera_refresh_champions,		.arg = NULL,				.desc = "Entities array"},
		{.func = camera_refresh_win_is_opened,	.arg = NULL,				.desc = "Window opened"},
		{.func = camera_refresh_entity_hovered,	.arg = NULL,				.desc = "Entity hovered"},
		{.func = camera_refresh_self,	        .arg = NULL,				.desc = "Self champion detection"},
		{.func = camera_refresh_victory,	    .arg = NULL,				.desc = "Victory State"},
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

	mode = camera_get_mode();

	last_mode = mode;
	*out_mode = mode;
}

LoLCameraState camera_main ()
{
	Vector2D target;
	float lerp_rate;
	CameraTrackingMode camera_mode;

	while (this->active)
	{
		if (kbhit())
		{
			char c = getch();
			// Interrupt request

			if (c == 'X' || c == 'x')
				return END_OF_LOLCAMERA;
		}

		Sleep(this->sleep_time);

		// Check if enabled.
		camera_set_tracking_mode(&camera_mode);

		// End of game = End of LoLCamera
		if (camera_mode == EndOfGame)
			return WAIT_FOR_END_OF_GAME;

		// Client has been alt-tabbed
		if (camera_mode == ForeGround)
		{
			//  Detect if client is disconnected
			if (!memproc_refresh_handle(this->mp))
			{
				info("Client not detected anymore.");
				this->active = FALSE;
			}

			continue;
		}

		if (camera_mode == NoUpdate || !camera_update())
			continue;

		// Compute target
		camera_compute_target(&target, camera_mode);

		// Compute lerp rate
		camera_compute_lerp_rate(&lerp_rate, camera_mode);

        // Smoothing
		if (abs(target.x - this->cam->v.x) > this->champ_settings.threshold)
			this->cam->v.x += (target.x - this->cam->v.x) * lerp_rate;

		if (abs(target.y - this->cam->v.y) > this->champ_settings.threshold)
			this->cam->v.y += (target.y - this->cam->v.y) * lerp_rate;

		// Keep this just before mempos_set(this->cam, x, y)
        if (camera_mode == NoMove)
            continue;

        // update the ingame gamera position
		mempos_set(this->cam, this->cam->v.x, this->cam->v.y);
	}

	return WAIT_FOR_NEW_GAME;
}

static void camera_compute_lerp_rate (float *lerp_rate, CameraTrackingMode camera_mode)
{
	float local_lerp_rate = this->champ_settings.lerp_rate;

	switch (camera_mode)
	{
		case Translate:
			local_lerp_rate = local_lerp_rate * 5;
		break;

		case isTranlating:
			local_lerp_rate = local_lerp_rate * 1;
		break;

		case RestoreCam:
		break;

		case CenterCam:
			// adjust camera smoothing rate when center camera
			local_lerp_rate = local_lerp_rate * 5;
		break;

		case FollowEntity:
		break;

		case Free:
			local_lerp_rate = local_lerp_rate * 2;
		break;

		case Normal:
		break;

		case FocusSelf:
		break;

		case NoMove:
		case NoUpdate:
		break;

		default:
		break;
	}

	*lerp_rate = local_lerp_rate;
}

BOOL camera_is_near (MemPos *pos, float limit)
{
	if (pos == NULL)
		return FALSE;

	float distance_cam_champ = vector2D_distance(&pos->v, &this->cam->v);

	return (distance_cam_champ < limit);
}

BOOL camera_entity_is_near (Entity *e, float limit)
{
	return camera_is_near(&e->p, limit);
}

void camera_compute_target (Vector2D *target, CameraTrackingMode camera_mode)
{
	float focus_x = 0.0, focus_y = 0.0;
	float drag_x = 0.0, drag_y = 0.0;
	float hint_x = 0.0, hint_y = 0.0;

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

		case RestoreCam :
			vector2D_set_pos(target, this->tmpcam.v.x, this->tmpcam.v.y);
			mempos_set(this->cam,    this->tmpcam.v.x, this->tmpcam.v.y);
			this->restore_tmpcam = FALSE;
		break;

		case FollowEntity :
			vector2D_set_pos(target, this->followed_entity->p.v.x, this->followed_entity->p.v.y);
			mempos_set(this->cam, this->followed_entity->p.v.x, this->followed_entity->p.v.y);
		break;

		case FocusSelf :
			vector2D_set_pos(target, this->champ->v.x, this->champ->v.y);
			mempos_set(this->cam, this->champ->v.x, this->champ->v.y);
		break;

		case Normal:
		{
			float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);
			float distance_dest_champ  = vector2D_distance(&this->dest->v, &this->champ->v);
			float distance_mouse_dest  = vector2D_distance(&this->dest->v, &this->mouse->v);

			// Always activated
			float champ_weight = this->champ_weight;
			float mouse_weight = this->mouse_weight;
			float dest_weight  = this->dest_weight;

			// Optional weights
			float hint_weight   = 0.0;
			float focus_weight  = 0.0;

			if (this->drag_request)
			{
				drag_x = (this->drag_pos.x - this->mouse->v.x) * 10;
				drag_y = (this->drag_pos.y - this->mouse->v.y) * 10;
			}

			if (this->focused_entity != NULL)
			{
				// ShareEntity is a Normal camera behavior + ally weight
				focus_weight = this->focus_weight;
				focus_x = this->focused_entity->p.v.x;
				focus_y = this->focused_entity->p.v.y;
			}

			if (this->hint_entity != NULL)
			{
				hint_weight = this->hint_weight;
				hint_x = this->hint_entity->p.v.x;
				hint_y = this->hint_entity->p.v.y;
			}

			float weight_sum;
			{
				// weighted averages
				// these values control how quickly the weights fall off the further you are
				// from the falloff distance
				float dest_falloff_rate  = 0.001;
				float mouse_falloff_rate = 0.002;

				// adjust weights based on distance
				if (distance_dest_champ > this->champ_settings.dest_range_max)
					dest_weight = 1.0 / (((distance_dest_champ - this->champ_settings.dest_range_max) * dest_falloff_rate) + 1.0);

				if (distance_mouse_champ > this->champ_settings.mouse_range_max)
					mouse_weight = 1.0 / (((distance_mouse_champ - this->champ_settings.mouse_range_max) * mouse_falloff_rate) + 1.0);

				if (distance_mouse_dest > this->champ_settings.mouse_dest_range_max)
					// if the mouse is far from dest, reduce dest weight (mouse is more important)
					dest_weight = dest_weight / (((distance_mouse_dest - this->champ_settings.mouse_dest_range_max) / 1500.0) + 1);

				weight_sum = champ_weight + mouse_weight + dest_weight + focus_weight + hint_weight;
			}

            // Compute the target (weighted averages)
			vector2D_set_pos(target,
                (
                    (this->champ->v.x * champ_weight) +
                    (this->mouse->v.x * mouse_weight) +
                    (this->dest->v.x * dest_weight) +
                    (focus_x * focus_weight) +
					(hint_x * hint_weight)
                 ) / weight_sum
					+ drag_x,
                (
                    (this->champ->v.y * champ_weight) +
                    (this->mouse->v.y * mouse_weight) +
                    (this->dest->v.y * dest_weight) +
                    (focus_y * focus_weight) +
					(hint_y * hint_weight)
                ) / weight_sum
					+ drag_y
            );

            // The camera goes farther when the camera is moving to the south
            float distance_mouse_champ_y = this->champ->v.y - this->mouse->v.y;
            if (distance_mouse_champ_y > 0)
                target->y -= distance_mouse_champ_y / 8.0; // <-- arbitrary value
		}
		break;

		case isTranlating:
			vector2D_set_pos(target,
				this->distance_translation.x + this->mouse->v.x,
				this->distance_translation.y + this->mouse->v.y
		);
		break;

		case Translate:
			vector2D_set_pos(target, this->champ->v.x + this->distance_translation.x, this->champ->v.y + this->distance_translation.y);
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

void camera_load_settings (char *section)
{
	BbQueue *default_settings = ini_parser_get_section(this->parser, section);

	if (bb_queue_is_empty(default_settings))
	{
		warning("Can't load \"%s\" settings in .ini (the section #%s doesn't exist)", section, section);
		return;
	}

	while (bb_queue_get_length(default_settings))
	{
		KeyVal *kv    = bb_queue_get_first(default_settings);

		char *setting = kv->key;
		char *value   = kv->res;

		char *possible_settings[] = {
			"lerp_rate", "threshold", "mouse_range_max", "dest_range_max", "mouse_dest_range_max"
		};

		for (int i = 0; i < sizeof(possible_settings) / sizeof(*possible_settings); i++)
		{
			if (strcmp(setting, possible_settings[i]) == 0)
			{
				switch (i)
				{
					case 0: // lerp rate
						this->champ_settings.lerp_rate = atof (value); // this controls smoothing, smaller values mean slower camera movement
						info("%s lerprate = %f", section, this->champ_settings.lerp_rate);
					break;

					case 1: // threshold
						this->champ_settings.threshold = atof (value); // minimum threshold before calculations halted because camera is "close enough"
						info("%s threshold = %f", section, this->champ_settings.threshold);
					break;

					case 2: // mouse_range_max
						this->champ_settings.mouse_range_max = atof(value);
						info("%s mouse_range_max = %f", section, this->champ_settings.mouse_range_max);
					break;

					case 3: // dest_range_max
						this->champ_settings.dest_range_max = atof(value);
						info("%s dest_range_max = %f", section, this->champ_settings.dest_range_max);
					break;

					case 4: // mouse_dest_range_max
						this->champ_settings.mouse_dest_range_max = atof(value);
						info("%s mouse_dest_range_max = %f", section, this->champ_settings.mouse_dest_range_max);
					break;
				}
			}
		}

		free(value);
		free(setting);
		free(kv);
	}

}

void camera_load_ini ()
{
	char *ini_file = "./LoLCamera.ini";
	// Loading parameters from .ini file :
	IniParser *parser = ini_parser_new(ini_file);
	this->parser = parser;

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
	this->win_is_opened_ptr = strtol(ini_parser_get_value(parser, "win_is_opened_ptr"), NULL, 16);
	this->respawn_reset_addr = strtol(ini_parser_get_value(parser, "respawn_reset_addr"), NULL, 16);
	this->border_screen_addr = strtol(ini_parser_get_value(parser, "border_screen_addr"), NULL, 16);
	this->allies_cam_addr[0] = strtol(ini_parser_get_value(parser, "allies_cam_addr0"), NULL, 16);
	this->allies_cam_addr[1] = strtol(ini_parser_get_value(parser, "allies_cam_addr1"), NULL, 16);
	this->self_cam_addr = strtol(ini_parser_get_value(parser, "self_cam_addr"), NULL, 16);
	this->entities_addr = strtol(ini_parser_get_value(parser, "entities_addr"), NULL, 16);
	this->entities_addr_end = strtol(ini_parser_get_value(parser, "entities_addr_end"), NULL, 16);
	this->locked_camera_addr = strtol(ini_parser_get_value(parser, "locked_camera_addr"), NULL, 16);
	this->loading_state_addr = strtol(ini_parser_get_value(parser, "loading_state_addr"), NULL, 16);
	this->wait_loading_screen = strtol(ini_parser_get_value(parser, "wait_loading_screen"), NULL, 16);
	this->output_cheatengine_table = strtol(ini_parser_get_value(parser, "output_cheatengine_table"), NULL, 16);
	this->camx_addr   = str_hex(ini_parser_get_value(parser, "camera_posx_addr"));
	this->camy_addr   = str_hex(ini_parser_get_value(parser, "camera_posy_addr"));
	this->champx_addr = str_hex(ini_parser_get_value(parser, "champion_posx_addr"));
	this->champy_addr = str_hex(ini_parser_get_value(parser, "champion_posy_addr"));
	this->mousex_addr = str_hex(ini_parser_get_value(parser, "mouse_posx_addr"));
	this->mousey_addr = str_hex(ini_parser_get_value(parser, "mouse_posy_addr"));
	this->destx_addr  = str_hex(ini_parser_get_value(parser, "dest_posx_addr"));
	this->desty_addr  = str_hex(ini_parser_get_value(parser, "dest_posy_addr"));
	this->mouse_screen_ptr = str_hex(ini_parser_get_value(parser, "mouse_screen_ptr"));
	this->win_is_opened_ptr = str_hex(ini_parser_get_value(parser, "win_is_opened_ptr"));
	this->respawn_reset_addr = str_hex(ini_parser_get_value(parser, "respawn_reset_addr"));
	this->border_screen_addr = str_hex(ini_parser_get_value(parser, "border_screen_addr"));
	this->allies_cam_addr[0] = str_hex(ini_parser_get_value(parser, "allies_cam_addr0"));
	this->allies_cam_addr[1] = str_hex(ini_parser_get_value(parser, "allies_cam_addr1"));
	this->self_cam_addr = str_hex(ini_parser_get_value(parser, "self_cam_addr"));
	this->entities_addr = str_hex(ini_parser_get_value(parser, "entities_addr"));
	this->entities_addr_end = str_hex(ini_parser_get_value(parser, "entities_addr_end"));
	this->locked_camera_addr = str_hex(ini_parser_get_value(parser, "locked_camera_addr"));
	this->loading_state_addr = str_hex(ini_parser_get_value(parser, "loading_state_addr"));
	this->victory_state_addr = str_hex(ini_parser_get_value(parser, "victory_state_addr"));

	// Hotkeys
	this->translate_key = ini_parser_get_char(parser, "translate_key");

	// Settings
	this->focus_weight = atof(ini_parser_get_value(parser, "focus_weight"));
	this->hint_weight  = atof(ini_parser_get_value(parser, "hint_weight"));
	this->champ_weight = atof(ini_parser_get_value(parser, "champ_weight"));
	this->dest_weight  = atof(ini_parser_get_value(parser, "dest_weight"));
	this->mouse_weight = atof(ini_parser_get_value(parser, "mouse_weight"));

	this->sleep_time  = strtol(ini_parser_get_value(parser, "sleep_time"), NULL, 10); // Time slept between two camera updates (in ms)
	this->poll_data	  = strtol(ini_parser_get_value(parser, "poll_data"), NULL, 10); // Retrieve data from client every X loops

	// Champion Settings
	camera_load_settings("Default");

	// Addresses - Input checking
	struct AddrStr { DWORD addr; char *str; } tabAddr [] = {
        { .addr = this->win_is_opened_ptr, .str = "win_is_opened_ptr" }, //
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
			info("\"%s\" cannot be read in %s", tabAddr[i].str, ini_file);

	// Settings - Input checking
	struct SettingVal {
		union {float *f; int *i;} p;
		union {float  f; int  i;} v;
	} tabSet [] = {
		// If the settings is not found in the .ini, set the value to its default value :
		{ .p.i = &this->sleep_time,	.v.i = 1.0},
		{ .p.i = &this->poll_data,	.v.i = 5.0},
	};

	for (int i = 0; i < sizeof(tabSet) / sizeof(struct SettingVal); i++)
	{
		if (*(tabSet[i].p.i) == 0)
			(*tabSet[i].p.i) = tabSet[i].v.i;
	}
}

inline Camera *camera_get_instance ()
{
	return this;
}

void camera_unload ()
{
	if (this == NULL)
		return;

	if (this->mp == NULL)
		return;

	// Process still active, unpatch
	if (memproc_refresh_handle(this->mp))
	{
		patch_list_set(this->patchlist, FALSE);

		this->mp = NULL;
		this = NULL;
	}
}
