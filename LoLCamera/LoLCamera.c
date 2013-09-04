#include "LoLCamera.h"

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
static void camera_compute_camera_scroll_speed (float *camera_scroll_speed, CameraTrackingMode camera_mode);
BOOL camera_entity_is_near (Entity *e, float limit);
static BOOL camera_follow_champion_requested ();
static BOOL camera_restore_requested ();
static void camera_toggle (BOOL enable);
static BOOL camera_is_translated ();
static void camera_translate_toggle (BOOL enable);
BOOL camera_interface_is_hovered ();

static void camera_toggle (BOOL enable)
{
	this->enabled = !enable;

	// Enable / Disable patches
	patch_list_set(this->patchlist, this->enabled);

	info("LoLCamera %s.", (this->enabled) ? "enabled" : "disabled");
}

static BOOL camera_is_enabled ()
{
	short toggle_state = GetKeyState(this->toggle_key);

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

static void camera_sensor_reset ()
{
    // Polling data is requested because we want to center the camera exactly where the champion is
	this->request_polling = TRUE;

	this->focused_entity = NULL;
	this->hint_entity = NULL;
	camera_translation_reset();
	mempos_set(this->cam, this->champ->v.x, this->champ->v.y);
	vector2D_set_zero(&this->lmb);

	// Events reset
	event_stop(&this->reset_after_minimap_click);
	this->wait_for_end_of_pause = FALSE;
}

static BOOL camera_center_requested ()
{
	// Disable when space / F1 is pressed
	if ((GetKeyState(this->center_key) < 0 || (GetKeyState(VK_F1) < 0))
	 && (this->interface_opened != LOLCAMERA_CHAT_OPENED_VALUE))
    {
		camera_sensor_reset();
        return TRUE;
    }

    return FALSE;
}

static BOOL camera_restore_requested ()
{
	if (this->restore_tmpcam)
	{
		camera_sensor_reset();
		return TRUE;
	}

	return FALSE;
}

static void camera_save_state (Vector2D *vector)
{
	memcpy(&this->tmpcam.v, vector, sizeof(Vector2D));
}

static void save_lmb_pos ()
{
	float x, y;
	vector2D_get_pos(&this->mouse->v, &x, &y);
	vector2D_set_pos(&this->lmb, x, y);
}

static BOOL camera_left_click ()
{
	if (GetKeyState(VK_LBUTTON) < 0)
	{
		this->lbutton_state = 1;

		// We want to move the camera only when we click on the minimap
		if (camera_interface_is_hovered())
		{
			switch (this->lbutton_state)
			{
				case 0:
				break;

				case 1:
					this->lbutton_state = 2;
					camera_save_state(&this->champ->v);
				break;

				case 2:
				break;
			}

			return TRUE;
		}
	}

	else
	{
		// On release
		switch (this->lbutton_state)
		{
			default :
				this->lbutton_state = 0;
			break;

			case 1:
				// On release click anywhere :

				// Save LMB position
				save_lmb_pos();
				this->lbutton_state = 0;
			break;

			case 2:
				// On release click on minimap :

				// Wait before reseting the view
				if (!this->dead_mode)
				{
					event_start_now(&this->reset_after_minimap_click);
					this->lbutton_state = 3;
					this->wait_for_end_of_pause = TRUE;
				}
				else
				{
					// Dead mode : we need to refresh the camera position
					camera_save_state(&this->cam->v);
					this->lbutton_state = 0;
					this->restore_tmpcam = TRUE;
				}
			break;

			case 3:
				// Wait the end of the event before reseting
				if (event_update(&this->reset_after_minimap_click))
				{
					event_stop(&this->reset_after_minimap_click);
					this->restore_tmpcam = TRUE;
					this->wait_for_end_of_pause = FALSE;
					this->lbutton_state  = 0;
				}
				else
				{
					this->wait_for_end_of_pause = TRUE;
				}
			break;
		}

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
	if (GetKeyState('G') < 0
	&&  GetKeyState('H') < 0)
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

BOOL camera_interface_is_hovered ()
{
	return (this->interface_hovered == 1);
}

BOOL camera_is_freezing ()
{
	return (this->wait_for_end_of_pause == TRUE);
}

BOOL camera_reset_conditions ()
{
	return (
		!camera_is_near(this->champ, 3000.0)
	&&  !this->dead_mode
	&&  !this->wait_for_end_of_pause);
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

	if (camera_is_freezing())
		return NoMove;

	if (camera_translate())
		return isTranlating;

	if (camera_is_translated())
		return Translate;

	// to allow minimap navigation, also disabled if LMB is down
	if (camera_left_click() || this->interface_opened == LOLCAMERA_SHOP_OPENED_VALUE)
		return NoMove;

	// Following ally & ennemies champions
	if (camera_follow_champion_requested ())
		return NoMove;

	// If our champion is dead, set free mode
	if (this->dead_mode)
		return Free;

	// The champion has been teleported far, focus on the champion
	if (camera_reset_conditions())
		return FocusSelf;

	// User hovered the interface, moving the camera uselessly
	if (camera_interface_is_hovered())
		return NoMove;

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
			//this->followed_entity = this->champions[i];
			//this->fxstate = (this->fxstate) ? this->fxstate : 1;
			//fx_pressed = TRUE;
			return TRUE;
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
		camera_save_state(&this->champ->v);
		this->fxstate = 2;
	}

	return fx_pressed;
}

void camera_init (MemProc *mp)
{
	if (this == NULL)
		this = calloc(sizeof(Camera), 1);

	// Allocation error
	if (this == NULL)
		fatal_error("Not enough memory for starting LoLCamera.");

	this->mp = mp;
	this->drag_pos = vector2D_new();

	// Initialize states
	this->enabled = TRUE;
	this->global_weight_activated = TRUE;
	this->active = FALSE;

	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	// Zeroing stuff
	memset(this->champions, 0, sizeof(Entity *));

	// Read static vars from .ini
	this->section_settings_name = "Default";
	camera_load_ini();

	// Get loading screen address
	info("Process detected");
	Sleep(1000);

	info("Dumping process...");
	memproc_dump(this->mp, text_section, text_section + text_size);
	camera_scan_loading();

	// Wait for client ingame
	if (camera_wait_for_ingame())
	{
		info("Game start detected ... Initializing");
		
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

	patch_list_set(this->patchlist, TRUE);

	// Export to CE
	if (this->output_cheatengine_table)
		camera_export_to_cheatengine();

	// Load settings associated with champ name
	this->section_settings_name = this->self->champ_name;
	camera_load_settings(this->self->champ_name);

	memset(this->nearby, 0, sizeof(this->nearby));
	this->active = TRUE;
}

BOOL bypass_login_screen_request (int key)
{
	return (key == 'p' || key == 'P');
}

BOOL camera_wait_for_ingame ()
{
	BOOL waited = FALSE;

	if (!this->wait_loading_screen)
		return TRUE;

	// Wait here
	int already_displayed_message = FALSE;
	while (!read_memory_as_int(this->mp->proc, this->loading_state_addr))
	{
		int key;

		if ((key = get_kb()) != -1)
		{
			// User input
			if (bypass_login_screen_request(key))
				break;

			if (exit_request(key))
				exit(0);
		}

		waited = TRUE;

		if (!already_displayed_message)
		{
			infob("Loading screen detected.\nKeep pressing 'P' if you wish to bypass that detection (only if you are already in game).\n");
			already_displayed_message = TRUE;
		}
		else
		{
			infobn(".");
		}

		Sleep(1000);
	}

	if (already_displayed_message)
		printf("\n");

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
	camera_left_click();
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
		{.func = camera_refresh_entities_nearby,.arg = NULL,				.desc = "Nearby champions"},
		{.func = camera_refresh_hover_interface,.arg = NULL,				.desc = "Hover Interface"},
		{.func = camera_refresh_screen_border,  .arg = NULL,                .desc = "ScreenBorder detection"}
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

int get_kb ()
{
	if (kbhit())
	{
		int c = (int) getch();
		return c;
	}

	return -1;
}

BOOL exit_request (int c)
{
	return (c == 'X' || c == 'x');
}

BOOL reload_ini_request (int c)
{
	return (c == 'R' || c == 'r');
}

void camera_save_last_campos (Vector2D *campos)
{
	memcpy(&this->last_campos, campos, sizeof(*campos));
}

void camera_update_states ()
{
	this->dead_mode = entity_is_dead(this->self);
}

BOOL global_key_toggle (int key)
{
	return (c == this->global_key);
}

LoLCameraState camera_main ()
{
	Vector2D target;
	float camera_scroll_speed;
	CameraTrackingMode camera_mode;

	while (this->active)
	{
		int key;

		// User input
		if ((key = get_kb()) != -1)
		{
			if (reload_ini_request(key))
			{
				info("Reloading ini...");
				camera_load_ini();
			}
			
			if (global_key_toggle(key))
			{
				this->global_weight_activated = ! (this->global_weight_activated);
			}

			if (exit_request(key))
				return END_OF_LOLCAMERA;
		}

		Sleep(this->sleep_time);

		// Update states
		camera_update_states();

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

		// Compute Camera Scroll Speed
		camera_compute_camera_scroll_speed (&camera_scroll_speed, camera_mode);

        // Smoothing
		if (abs(target.x - this->cam->v.x) > this->champ_settings.threshold)
			this->cam->v.x += (target.x - this->cam->v.x) * camera_scroll_speed;

		if (abs(target.y - this->cam->v.y) > this->champ_settings.threshold)
			this->cam->v.y += (target.y - this->cam->v.y) * camera_scroll_speed;

		// Keep this just before mempos_set(this->cam, x, y)
        if (camera_mode == NoMove)
            continue;

        // update the ingame gamera position
		mempos_set(this->cam, this->cam->v.x, this->cam->v.y);

		// Save last camera position
		camera_save_last_campos(&this->cam->v);
	}

	return WAIT_FOR_NEW_GAME;
}

static void camera_compute_camera_scroll_speed (float *camera_scroll_speed, CameraTrackingMode camera_mode)
{
	float local_camera_scroll_speed = this->champ_settings.camera_scroll_speed;

	switch (camera_mode)
	{
		case Translate:
			local_camera_scroll_speed = local_camera_scroll_speed * 5;
		break;

		case isTranlating:
			local_camera_scroll_speed = local_camera_scroll_speed * 1;
		break;

		case RestoreCam:
		break;

		case CenterCam:
			// adjust camera smoothing rate when center camera
			local_camera_scroll_speed = local_camera_scroll_speed * 5;
		break;

		case FollowEntity:
		break;

		case Free:
			local_camera_scroll_speed = local_camera_scroll_speed * 2;
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

	*camera_scroll_speed = local_camera_scroll_speed;
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
	float average_x = 0.0, average_y = 0.0;
	float lmb_x = 0.0, lmb_y = 0.0;

	switch (camera_mode)
	{
		case Free:
			if (!camera_interface_is_hovered())
			{
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
			}
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

			// Always activated
			float champ_weight = this->champ_weight;
			float mouse_weight = this->mouse_weight;
			float dest_weight  = this->dest_weight;
			float lmb_weight   = this->lmb_weight;

			// Optional weights
			float hint_weight    = 0.0;
			float focus_weight   = 0.0;
			float global_weight  = 0.0;
			lmb_x = this->lmb.x;
			lmb_y = this->lmb.y;

			// Fix the perspective
            if (mouse_weight)
            {
            	// The camera goes farther when the camera is moving to the south
				float distance_mouse_champ_y = this->champ->v.y - this->mouse->v.y;
				if (distance_mouse_champ_y > 0.0) {
					this->mouse->v.y -= distance_mouse_champ_y * 0.1; // <-- arbitrary value
				}
            }

			// Distances
			float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);
			float distance_dest_champ  = vector2D_distance(&this->dest->v, &this->champ->v);
			float distance_mouse_dest  = vector2D_distance(&this->dest->v, &this->mouse->v);

			// LMB is not set
			if (lmb_x == 0 || lmb_y == 0)
			{
				lmb_weight = 0.0;
			}

			if (this->drag_request)
			{
				drag_x = (this->drag_pos.x - this->mouse->v.x) * 10;
				drag_y = (this->drag_pos.y - this->mouse->v.y) * 10;
			}

			if (this->global_weight && this->global_weight_activated)
			{
				if (this->nb_nearby == 0)
				{
					global_weight = 0.0;
				}

				else
				{
					global_weight = this->global_weight;

					float sum_x = 0, sum_y = 0;

					for (int i = 0; i < this->nb_nearby; i++)
					{
						sum_x += this->nearby[i]->p.v.x;
						sum_y += this->nearby[i]->p.v.y;
					}

					average_x = sum_x / (float) this->nb_nearby;
					average_y = sum_y / (float) this->nb_nearby;
				}
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
				float dest_falloff_rate  = 0.00001;
				float mouse_falloff_rate = 0.00001;

				// adjust weights based on distance
				if (distance_dest_champ > this->champ_settings.dest_range_max)
					dest_weight = dest_weight / (((distance_dest_champ - this->champ_settings.dest_range_max) * dest_falloff_rate) + 1.0);

				if (distance_mouse_champ > this->champ_settings.mouse_range_max)
					mouse_weight = mouse_weight / (((distance_mouse_champ - this->champ_settings.mouse_range_max) * mouse_falloff_rate) + 1.0);

				// if the mouse is far from dest, reduce dest weight (mouse is more important)
				if (distance_mouse_dest > this->champ_settings.mouse_dest_range_max)
					dest_weight = dest_weight / (((distance_mouse_dest - this->champ_settings.mouse_dest_range_max) / 1500.0) + 1.0);

				weight_sum = champ_weight + mouse_weight + dest_weight + focus_weight + hint_weight + global_weight + lmb_weight;
			}

            // Compute the target (weighted averages)
			vector2D_set_pos(target,
                (
                    (this->champ->v.x * champ_weight) +
                    (this->mouse->v.x * mouse_weight) +
                    (this->dest->v.x * dest_weight) +
                    (focus_x * focus_weight) +
					(hint_x * hint_weight) +
                    (average_x * global_weight) +
                    (this->lmb.x * lmb_weight)
                 ) / weight_sum
					+ drag_x,
                (
                    (this->champ->v.y * champ_weight) +
                    (this->mouse->v.y * mouse_weight) +
                    (this->dest->v.y * dest_weight) +
                    (focus_y * focus_weight) +
					(hint_y * hint_weight) +
                    (average_y * global_weight) +
                    (this->lmb.y * lmb_weight)
                ) / weight_sum
					+ drag_y
            );
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

	info("------------------------------------------------------------------");
	while (bb_queue_get_length(default_settings))
	{
		KeyVal *kv    = bb_queue_get_first(default_settings);

		char *setting = kv->key;
		char *value   = kv->res;

		char *possible_settings[] = {
			"camera_scroll_speed", "threshold", "mouse_range_max", "dest_range_max", "mouse_dest_range_max"
		};

		for (int i = 0; i < sizeof(possible_settings) / sizeof(*possible_settings); i++)
		{
			if (strcmp(setting, possible_settings[i]) == 0)
			{
				switch (i)
				{
					case 0: // lerp rate
						this->champ_settings.camera_scroll_speed = atof (value); // this controls smoothing, smaller values mean slower camera movement
						info("%s camera_scroll_speed = %f", section, this->champ_settings.camera_scroll_speed);
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
	info("------------------------------------------------------------------");

}

void camera_load_ini ()
{
	IniParser *parser = this->parser;

	// Loading parameters from .ini file :
	parser = ini_parser_new("./LoLCamera.ini");
	this->parser = parser;

	ini_parser_reg_and_read(parser);

	// Hotkeys
	if ((this->translate_key = ini_parser_get_char(parser, "translate_key")) == 0)
		this->translate_key  = strtol(ini_parser_get_value(parser, "translate_key"), NULL, 16);

	if ((this->toggle_key = ini_parser_get_char(parser, "toggle_key")) == 0)
		this->toggle_key  = strtol(ini_parser_get_value(parser, "toggle_key"), NULL, 16);

	if ((this->center_key = ini_parser_get_char(parser, "center_key")) == 0)
		this->center_key  = strtol(ini_parser_get_value(parser, "center_key"), NULL, 16);

	if ((this->global_key = ini_parser_get_char(parser, "global_key")) == 0)
		this->global_key  = strtol(ini_parser_get_value(parser, "global_key"), NULL, 16);

	// Settings
	this->champ_weight = atof(ini_parser_get_value(parser, "champ_weight"));
	this->mouse_weight = atof(ini_parser_get_value(parser, "mouse_weight"));
	this->dest_weight  = atof(ini_parser_get_value(parser, "dest_weight"));
	this->lmb_weight   = atof(ini_parser_get_value(parser, "lmb_weight"));

	this->focus_weight = atof(ini_parser_get_value(parser, "focus_weight"));
	this->hint_weight  = atof(ini_parser_get_value(parser, "hint_weight"));
	this->global_weight  = atof(ini_parser_get_value(parser, "global_weight"));

	this->sleep_time  = strtol(ini_parser_get_value(parser, "sleep_time"), NULL, 10); // Time slept between two camera updates (in ms)
	this->poll_data	  = strtol(ini_parser_get_value(parser, "poll_data"), NULL, 10); // Retrieve data from client every X loops

	this->wait_loading_screen = strtol(ini_parser_get_value(parser, "wait_loading_screen"), NULL, 10);
	this->ms_after_minimap_click = strtol(ini_parser_get_value(parser, "ms_after_minimap_click"), NULL, 10);

	// Champion Settings
	camera_load_settings(this->section_settings_name);

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

	// Events init
	event_init(&this->reset_after_minimap_click, this->ms_after_minimap_click, this->ms_after_minimap_click);
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
