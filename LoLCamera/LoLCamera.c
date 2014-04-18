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
static bool camera_entity_is_near (Entity *e, float limit);
static bool camera_follow_champion_requested ();
static bool camera_restore_requested ();
static void camera_toggle (bool enable);
static bool camera_is_translated ();
static void camera_translate_toggle (int state);
static bool camera_interface_is_hovered ();
static bool camera_mouse_in_minimap ();
static float camera_compute_camera_scroll_speed (CameraTrackingMode camera_mode);

static void camera_toggle (bool enable)
{
	this->enabled = !enable;

	// Enable / Disable patches
	patch_list_set(this->patchlist, this->enabled);

	info("LoLCamera %s.", (this->enabled) ? "enabled" : "disabled");
}

static bool camera_is_enabled ()
{
	short toggle_state = camera_getkey(this->toggle_key);

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

static void camera_sensor_reset (bool reset_translation)
{
    // Polling data is requested because we want to center the camera exactly where the champion is
	this->request_polling = true;
	this->focused_entity = NULL;
	this->hint_entity = NULL;

	if (reset_translation)
		camera_translation_reset();

	camera_set_pos(this->champ->v.x, this->champ->v.y);
	vector2D_set_zero(&this->lmb);

	// Events reset
	event_stop(&this->reset_after_minimap_click);
	this->wait_for_end_of_pause = false;
}

static bool camera_center_requested ()
{
	// Disable when center_key or F1 is pressed
	if ((camera_getkey(this->center_key) < 0 || (camera_getkey(VK_F1) < 0))
	 && (this->interface_opened != LOLCAMERA_CHAT_OPENED_VALUE))
    {
		camera_sensor_reset(true);
        return true;
    }

    return false;
}

static bool camera_restore_requested ()
{
	if (this->restore_tmpcam)
	{
		camera_sensor_reset(false);
		return true;
	}

	return false;
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

void camera_reset ()
{
	if (this == NULL)
		return;

	this->entity_hovered = NULL;
	vector2D_set_zero(&this->distance_translation);
	this->translate_request = 0;
	this->last_translate_state = 0;

	this->camera_movement = NULL;
	this->border_screen = NULL;
	this->patchlist = NULL;

	// Entities
	for (int i = 0; i < 10; i++)
	{
		this->champions[i] = NULL;
		this->nearby[i] = NULL;
	}

	this->focused_entity = NULL;
	this->followed_entity = NULL;
	this->hint_entity = NULL;
	this->self = NULL;
	memset(this->self_name, '\0', sizeof(this->self_name));

	for (int i = 0; i < 5; i++)
	{
		this->nearby_allies[i] = NULL;
		this->nearby_ennemies[i] = NULL;
	}

	this->nb_allies_nearby = 0;
	this->nb_ennemies_nearby = 0;
	this->section_settings_name = "Default";
}

static bool camera_not_pinging ()
{
    return (
            !(camera_getkey(VK_LMENU) < 0)    // not alt pressed
        &&  !(camera_getkey(VK_LCONTROL) < 0) // not ctrl pressed
        &&  !(this->ping_state == LOLCAMERA_PING_OR_SKILL_PRESSED)
    );
}

static bool camera_left_click ()
{
	if (camera_getkey(VK_LBUTTON) < 0)
	{
		if (this->lbutton_state == 0)
			this->lbutton_state = 1;

		// We want to move the camera only when we click on the minimap
		if (
			(camera_interface_is_hovered())
		&&  (this->interface_opened != LOLCAMERA_SHOP_OPENED_VALUE)
        &&  (camera_mouse_in_minimap())
        &&  (camera_not_pinging())
		) {
			switch (this->lbutton_state)
			{
				case 0:
				break;

				case 1:
					this->lbutton_state = 2;
					camera_save_state(&this->champ->v);
					save_lmb_pos();
					camera_set_pos(this->lmb.x, this->lmb.y);
				break;

				case 3:
                    event_stop(&this->reset_after_minimap_click);
					this->lbutton_state = 2;
				case 2:
					save_lmb_pos();
					camera_set_pos(this->lmb.x, this->lmb.y);
					this->request_polling = TRUE;
				break;
			}

			return true;
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
				// On release click anywhere
                // Save LMB position
                save_lmb_pos();
                this->lbutton_state = 0;
                this->translate_lmb.x = this->champ->v.x - this->lmb.x;
                this->translate_lmb.y = this->champ->v.y - this->lmb.y;
			break;

			case 2:
				// On release click on minimap :
				if (camera_mouse_in_minimap())
                {
                    if (!this->dead_mode)
                    {
                        float distance_cam_champ = vector2D_distance(&this->champ->v, &this->cam->v);

                        if (distance_cam_champ > IN_SCREEN_DISTANCE)
                        {
                            // Wait before reseting the view
                            event_start_now(&this->reset_after_minimap_click);
                            this->lbutton_state = 3;
                            this->wait_for_end_of_pause = true;
                            this->request_polling = true;
                        }
                        else
                        {
                            // We actualize the camera position to the current view
                            camera_save_state(&this->cam->v);
                            this->restore_tmpcam = true;
                            this->lbutton_state = 0;
                        }
                    }
                }
                else
                {
                    // Release click on minimap somewhere else
                    this->lbutton_state = 0;
                }
			break;

			case 3:
				// Wait the end of the event before reseting
				if (event_update(&this->reset_after_minimap_click))
                {
					event_stop(&this->reset_after_minimap_click);
					this->restore_tmpcam = true;
					this->wait_for_end_of_pause = false;
					this->lbutton_state  = 0;
					this->request_polling = true;
				}
				else
				{
					this->wait_for_end_of_pause = true;
				}
			break;
		}
	}

	return false;
}

static void camera_translate_toggle (int state)
{
	switch (state)
	{
		case 0: // Translate request
			vector2D_set_zero(&this->distance_translation);
			this->translate_request = 1;
			this->old_threshold = this->champ_settings.threshold;
			this->champ_settings.threshold = 0;
		break;

		case 1: // Release
			this->distance_translation.x = this->cam->v.x - this->champ->v.x;
			this->distance_translation.y = this->cam->v.y - this->champ->v.y;
			this->translate_request = 0;
			this->champ_settings.threshold = this->old_threshold;
		break;
	}
}

static bool camera_translate ()
{
	short translate_state = camera_getkey(this->translate_key);

	// Listen for translate toggle key
	if (translate_state != this->last_translate_state && translate_state < 0
	&& this->interface_opened != LOLCAMERA_CHAT_OPENED_VALUE)
	{
		this->last_translate_state = translate_state;
		camera_translate_toggle(this->translate_request);
	}

	return this->translate_request;
}

static bool camera_is_translated ()
{
	return (
		this->distance_translation.x != 0.0
	&&  this->distance_translation.y != 0.0);
}

static void camera_debug_mode ()
{
	if (camera_getkey('G') < 0
	&&  camera_getkey('H') < 0)
	{
		if (!this->dbg_mode)
		{
			// Debug mode activated
			this->dbg_mode = true;
		}
	}

	else
	{
		if (this->dbg_mode)
		{
			info("------ Tests ------");
			// On release
			struct { char *str; bool (*fct)(); } unit_tests [] = {
				{"Camera Position",   camera_ut_campos},
				{"Champion Position", camera_ut_champos},
				{"Mouse position",    camera_ut_mousepos},
				{"Dest position",     camera_ut_destpos},
				{"Window opened", 	  camera_ut_is_win_opened},
				{"Entities array",    camera_ut_entities}
			};

			for (int i = 0; i < sizeof_array(unit_tests); i++)
			{
				info("Unit test %s : %s", unit_tests[i].str, (unit_tests[i].fct()) ? "OK" : "FAILED");
			}
		}

		this->dbg_mode = false;
	}
}
static bool camera_mouse_in_minimap ()
{
    return (
        this->mouse_screen.x >= this->minimap.xLeft
    &&  this->mouse_screen.x <= this->minimap.xRight
    &&  this->mouse_screen.y >= this->minimap.yTop
    &&  this->mouse_screen.y <= this->minimap.yBot
    );
}

static bool camera_window_is_active ()
{
	return (this->mp->hwnd == GetForegroundWindow());
}

bool camera_victory_state ()
{
	return (this->victory_state == 3);
}

bool camera_interface_is_hovered ()
{
	return (this->interface_hovered == 1);
}

bool camera_is_freezing ()
{
	return (this->wait_for_end_of_pause == true);
}

bool camera_reset_conditions ()
{
	float distance_traveled = vector2D_distance(&this->champ->v, &this->last_champpos);

	return (distance_traveled > 1000.0
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

	// User hovered the minimap, don't move the camera
	if (camera_interface_is_hovered())
		return NoMove;

	// If our champion is dead, set free mode
	if (this->dead_mode)
		return Free;

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
		return FollowEntity;

	// The champion has been teleported far, focus on the champion
	if (camera_reset_conditions())
		return FocusSelf;

    return Normal;
}

static bool camera_follow_champion_requested ()
{
	bool fx_pressed = false;
	int keys[] = {VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10};

	for (int i = 1; i < 10; i++)
	{
		if (
            (camera_getkey(keys[i]) < 0)
        &&  (this->playersCount > i)
        &&  (entity_ally_with(this->self, this->champions[i]))
        )
		{
			this->followed_entity = this->champions[i];
			this->fxstate = (this->fxstate) ? this->fxstate : 1;
			fx_pressed = true;
		}
	}

	if (!fx_pressed)
	{
		if (this->fxstate == 2)
		{
			camera_save_state(&this->champ->v);
			this->restore_tmpcam = true;
		}

		this->fxstate = 0;
	}

	if (this->fxstate == 1)
		this->fxstate = 2;

	return fx_pressed;
}

void camera_init_light (MemProc *mp)
{
	this = calloc(sizeof(Camera), 1);
	this->mp = mp;
}

void camera_run_light ()
{
	this->cam    = mempos_new (this->mp, this->camx_val,      this->camy_val);
	this->camPos = mempos_new (this->mp, this->camx_addr,     this->camy_addr);
	this->champ  = mempos_new (this->mp, this->champx_addr,   this->champy_addr);
	this->mouse  = mempos_new (this->mp, this->mousex_addr,   this->mousey_addr);
	this->dest   = mempos_new (this->mp, this->destx_addr,    this->desty_addr);
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
	this->enabled = true;
	this->global_weight_activated = true;
	this->active = false;

	// TODO : get .text section offset + size properly
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = EXECUTABLE_TEXT_SIZE;

	// Zeroing stuff
	memset(this->champions, 0, sizeof(Entity *));
	memset(this->nearby, 0, sizeof(this->nearby));

	// Read static vars from .ini
	this->section_settings_name = "Default";
	camera_load_ini();

	// Get loading screen address
	info("Process detected");
	Sleep(1000);

	info("Dumping process...");
	memproc_dump(this->mp, text_section, text_section + text_size);

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

	// Init data from the client
	this->cam   	   = mempos_new (this->mp, this->camx_val,      this->camy_val);
	this->camPos   	   = mempos_new (this->mp, this->camx_addr,     this->camy_addr);
	this->champ 	   = mempos_new (this->mp, this->champx_addr,   this->champy_addr);
	this->mouse 	   = mempos_new (this->mp, this->mousex_addr,   this->mousey_addr);
	this->dest  	   = mempos_new (this->mp, this->destx_addr,    this->desty_addr);

	// Export to CE
	camera_export_to_cheatengine();

	// Checks before patching !
	if (this->self == NULL) {
		fatal_error("LoLCamera failed to read client variables.\n"
		            "Please wait for a fix, or send a mail at <lolcamera.contact@gmail.com>.");
	}

	// Signature scanning for patches
    camera_scan_patch();

    // Patch
    info("Patching ...");
	patch_list_set (this->patchlist, true);

	// Load settings associated with champ name
	this->section_settings_name = this->self->champ_name;

	if (strlen(this->section_settings_name) > 0)
		camera_load_settings(this->section_settings_name);

	this->active = true;
}

bool bypass_login_screen_request (int key)
{
	return (key == 'p' || key == 'P');
}

bool camera_ingame_conditions ()
{
	if (!camera_scan_game_info(false))
		return false;

	if (!camera_scan_champions(false))
		return false;

	DWORD cur = this->entity_ptr;
	DWORD end = this->entity_ptr_end;

	for (int i = 0; cur != end && i < 10; cur += 4, i++)
	{
		Entity *e = this->champions[i];

		if ((e == NULL) || (e->entity_data == 0))
        {
            debug("Entity data is NULL (%d:0x%x)", i, cur);
            return false;
        }
	}

    // Check from mouse IG position
	if (!this->mouse)
    {
        if (camera_scan_cursor_champ())
        {
            if (this->mouse = mempos_new (this->mp, this->mousex_addr, this->mousey_addr))
            {
                if (out_of_map(this->mouse->v.x, this->mouse->v.y))
                {
                    warning("Mouse = %.2f - %.2f", this->mouse->v.x, this->mouse->v.y);
                    warning("Mouse is out of map : If your LoL window is inactive, ignore this warning");
                    return true;
                }
            }
            else
            {
                debug("mouse alloc error");
                return false;
            }
        }

        else
        {
            debug("Cannot scan cursor_champ");
            return false;
        }
    }

	return true;
}

bool out_of_map (float x, float y)
{
	return (x < 0.0       || y < 0.0
	||		x > MAP_WIDTH || y > MAP_HEIGHT);
}

bool camera_wait_for_ingame ()
{
	bool waited = false;

	if (!this->wait_loading_screen)
		return true;

	// Wait here
	int already_displayed_message = false;
	while (!camera_ingame_conditions())
	{
		if (!memproc_refresh_handle(this->mp))
			fatal_error("Client not detected anymore. Please relaunch LoLCamera.");

		int key;

		if ((key = get_kb()) != -1)
		{
			// User input
			if (bypass_login_screen_request(key))
				break;

			if (exit_request(key))
				exit(0);
		}

		waited = true;

		if (!already_displayed_message)
		{
			infob("Loading screen detected.\nKeep pressing 'P' if you wish to bypass that detection (only if you are already in game).\n");
			already_displayed_message = true;
		}
		else
		{
			infobn(".");
		}

		Sleep(3000);
	}

	if (already_displayed_message)
		printf("\n");

    Sleep(1000);

	return waited;
}

inline void camera_set_active (bool active)
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
		if (camera_getkey(VK_LBUTTON) < 0)
			this->focused_entity = this->entity_hovered;
	}

	if (this->focused_entity != NULL)
	{
		// When we are in "dead" spectator mode, it's not important to change camera mode.
		if (entity_is_dead(this->focused_entity)
		|| !camera_entity_is_near(this->focused_entity, MEDIUM_BOX)
		|| !entity_is_visible(this->focused_entity)
		)
		{
			this->focused_entity = NULL;
		}
	}

	if (this->hint_entity != NULL)
	{
		if (entity_is_dead(this->hint_entity)
		|| !camera_entity_is_near(this->hint_entity, MEDIUM_BOX)
		|| !entity_is_visible(this->hint_entity)
		)
		{
			this->hint_entity = NULL;
		}
	}
}

static void camera_middle_click ()
{
	if (camera_getkey(this->drag_key) < 0)
	{
		// Drag
		switch (this->mbutton_state)
		{
			case 0:
				this->drag_pos = this->mouse->v;
				this->request_polling = true;
				this->mbutton_state = 1;
			break;

			case 1:
				this->drag_pos = this->mouse->v;
				this->mbutton_state = 2;
			break;

			case 2:
			break;
		}

		this->drag_request = true;
	}

	else
	{
		this->mbutton_state = 0;
		this->drag_request = false;
	}
}

bool camera_update ()
{
	static unsigned int frame_count = 0;

	// Manage camera states
	camera_entity_manager();
	camera_middle_click();
	camera_left_click();
	camera_debug_mode();

	struct refreshFunctions { bool (*func)(); void *arg; char *desc; } refresh_funcs [] =
	{
		{.func = mempos_refresh, 				.arg = this->cam,  			.desc = "this->cam MemPos"},
		{.func = mempos_refresh, 				.arg = this->champ,			.desc = "this->champ MemPos"},
		{.func = mempos_refresh,				.arg = this->dest, 			.desc = "this->dest MemPos"},
		{.func = mempos_refresh,				.arg = this->mouse, 		.desc = "this->mouse MemPos"},
		{.func = camera_refresh_champions,		.arg = NULL,				.desc = "Entities array"},
		{.func = camera_refresh_win_is_opened,	.arg = NULL,				.desc = "Window opened"},
		{.func = camera_refresh_entity_hovered,	.arg = NULL,				.desc = "Entity hovered"},
		{.func = camera_refresh_self,	        .arg = false,				.desc = "Self champion detection"},
		{.func = camera_refresh_entities_nearby,.arg = NULL,				.desc = "Nearby champions"},
		{.func = camera_refresh_hover_interface,.arg = NULL,				.desc = "Hover Interface"},
		{.func = camera_refresh_mouse_screen,   .arg = NULL,				.desc = "Mouse Screen"},
		{.func = camera_refresh_ping_state,     .arg = NULL,				.desc = "Ping State"},
	};

	if (frame_count++ % this->poll_data == 0 || this->request_polling)
	{
		for (int i = 0; i < sizeof_array(refresh_funcs); i++)
		{
			bool (*func)() = refresh_funcs[i].func;
			void *arg = refresh_funcs[i].arg;
			char *desc = refresh_funcs[i].desc;

			if (!(func(arg)))
			{
				warning("\"%s\" : Refresh failed", desc);

				//  Detect if client is disconnected
				if (!memproc_refresh_handle(this->mp))
				{
					info("Client not detected anymore.");
					this->active = false;
					return false;
				}

				// Resynchronize with the process
				if (!camera_scan_variables()
				||	!camera_scan_champions(true))
				{
					warning("Synchronization with the client isn't possible - Retrying in 0.5s.");
					Sleep(500);
				}
				else
				{
					info("LoLCamera re-sync done");
					return false;
				}

				return false;
			}
		}

		this->request_polling = false;
	}

	return true;
}

void camera_mode_dump (CameraTrackingMode mode)
{
    char * mode_str [] = {
    	"Normal",
		"CenterCam",
		"RestoreCam",
		"NoMove",
		"NoUpdate",
		"EndOfGame",
		"ForeGround",
		"Free",
		"FocusSelf",
		"FollowEntity",
		"Translate",
		"isTranlating"
    };

	printf("Mode = %s\n", mode_str[(int)mode]);
}

void camera_set_tracking_mode (CameraTrackingMode *out_mode)
{
	*out_mode = camera_get_mode();
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

bool exit_request (int c)
{
	return (c == 'X' || c == 'x');
}

bool update_request (int c)
{
    return (c == 'U' || c == 'u');
}

bool reload_ini_request (int c)
{
	return (c == 'R' || c == 'r');
}

void camera_save_last_campos (Vector2D *campos)
{
	memcpy(&this->last_campos, campos, sizeof(*campos));
}

void camera_save_last_champpos (Vector2D *champpos)
{
	memcpy(&this->last_champpos, champpos, sizeof(*champpos));
}

void camera_update_states ()
{
	this->dead_mode = entity_is_dead(this->self);
}

short int camera_getkey (int key)
{
	if ((this->interface_opened == LOLCAMERA_CHAT_OPENED_VALUE)
	&&  (key >= 0x33 && key <= 0x126))
	{
		// Chat opened, printable character pressed : nothing happens
		return 1;
	}

	int keys[] = {VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10};
	if ((this->disable_fx_keys)
	&& (is_in_array(key, keys, sizeof_array(keys))))
	{
		// Disable FX keys
		return 1;
	}

	return GetKeyState(key);
}

bool move_keys_pressed ()
{
    return false;
}

bool global_key_pressed ()
{
	static short int last_global_key = -1;
	short int global_key_state = camera_getkey(this->global_key);

	if (last_global_key == -1)
		last_global_key = global_key_state;

	// Listen for translate toggle key
	if (global_key_state != last_global_key && global_key_state < 0)
	{
		last_global_key = global_key_state;
		return true;
	}

	return false;
}

LoLCameraState camera_main ()
{
	Vector2D target;
	CameraTrackingMode camera_mode = NoMove;

	while (this->active)
	{
		int key;

		// User input

		// Console input
		if ((key = get_kb()) != -1)
		{
			if (reload_ini_request(key))
			{
				info("Reloading ini...");
				camera_load_ini();
			}

			if (exit_request(key))
				return END_OF_LOLCAMERA;
		}
		// In game keys
		if (camera_mode != ForeGround)
		{
			if (global_key_pressed())
			{
				this->global_weight_activated = ! (this->global_weight_activated);
				info("Global weight has been %s.", (this->global_weight_activated) ? "enabled" : "disabled");
			}

			if (move_keys_pressed())
            {

            }
		}

		Sleep(this->sleep_time);

		// Update states
		camera_update_states();

		// Check if enabled.
		camera_set_tracking_mode(&camera_mode);

		switch (camera_mode)
		{
			default:
			break;

			case EndOfGame:
				return WAIT_FOR_END_OF_GAME;
			break;

			case ForeGround:
				//  Detect if client is disconnected
				if (!memproc_refresh_handle(this->mp))
				{
					info("Client not detected anymore.");
					this->active = false;
				}
				continue;
			break;
		}

		// Save last champion position
		camera_save_last_champpos(&this->champ->v);

		if (camera_mode == NoUpdate || !camera_update())
			continue;

		// Compute target
		camera_compute_target(&target, camera_mode);

		// Compute Camera Scroll Speed
		float camera_scroll_speed            = camera_compute_camera_scroll_speed (camera_mode);
		float camera_scroll_speed_vertical   = this->champ_settings.camera_scroll_speed_vertical;
		float camera_scroll_speed_horizontal = this->champ_settings.camera_scroll_speed_horizontal;

		// Distance from ideal target and current camera
		float dist_target_cam = vector2D_distance(&target, &this->cam->v);
		float threshold       = this->champ_settings.threshold;

		if (dist_target_cam > threshold)
            camera_scroll_speed *= ((dist_target_cam - threshold) * 0.01);

		// Apply to target
        vector2D_sscalar(&target, 1.0 + camera_scroll_speed);

        // Smoothing
		if (abs(target.x - this->cam->v.x) > this->champ_settings.threshold)
			this->cam->v.x += (target.x - this->cam->v.x) * camera_scroll_speed * camera_scroll_speed_horizontal;

		if (abs(target.y - this->cam->v.y) > this->champ_settings.threshold)
			this->cam->v.y += (target.y - this->cam->v.y) * camera_scroll_speed * camera_scroll_speed_vertical;

		// Keep this just before camera_set_pos(this->cam->v.x, this->cam->v.y);
        if (camera_mode == NoMove)
            continue;

        // update the ingame gamera position
		camera_set_pos(this->cam->v.x, this->cam->v.y);

		// Save last positions
		camera_save_last_campos(&this->cam->v);
	}

	return WAIT_FOR_NEW_GAME;
}

void camera_set_pos (float x, float y)
{
    //if (!out_of_map(x, y))
    {
        mempos_set(this->camPos, x, y);
        mempos_set(this->cam, x, y);
    }
}

static float camera_compute_camera_scroll_speed (CameraTrackingMode camera_mode)
{
	float camera_scroll_speed = this->champ_settings.camera_scroll_speed;

    // Adjust camera smoothing rate
	switch (camera_mode)
	{
		case Translate:
			camera_scroll_speed *= 1;
		break;

		case isTranlating:
			camera_scroll_speed *= 0.2;
		break;

		case RestoreCam:
		break;

		case CenterCam:
			camera_scroll_speed *= 5;
		break;

		case FollowEntity:
		break;

		case Free:
			camera_scroll_speed *= 2;
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

    camera_scroll_speed *= (this->self->movement_speed / 400.0);

    return camera_scroll_speed;
}

bool camera_is_near (MemPos *pos, float limit)
{
	if (pos == NULL)
		return false;

	float distance_cam_pos = vector2D_distance(&pos->v, &this->cam->v);

	return (distance_cam_pos < limit);
}

bool camera_entity_is_near (Entity *e, float limit)
{
	return camera_is_near(&e->p, limit);
}

void camera_compute_target (Vector2D *target, CameraTrackingMode camera_mode)
{
    Vector2D focus = vector2D_zero(),
             drag = vector2D_zero(),
             hint = vector2D_zero(),
             allies = vector2D_zero(),
             ennemies = vector2D_zero(),
             lmb  = vector2D_zero(),
             mouse = this->mouse->v,
             champ = this->champ->v,
             cam = this->cam->v,
             tmpcam = this->tmpcam.v,
             entity = (this->followed_entity) ? this->followed_entity->p.v : (Vector2D) vector2D_zero(),
             dest = this->dest->v;

    // Weights
    float champ_weight = this->champ_weight;
    float mouse_weight = this->mouse_weight;
    float dest_weight  = this->dest_weight;
    float lmb_weight   = this->lmb_weight;
    float hint_weight  = 0.0;
    float focus_weight = 0.0;
    float global_weight_allies   = 0.0;
    float global_weight_ennemies = 0.0;

    // Fix the perspective
    if (mouse_weight)
    {
        // The camera goes farther when the camera is moving to the south
        float distance_mouse_cam_y = cam.y - mouse.y;
        if (distance_mouse_cam_y > 0.0 && distance_mouse_cam_y > this->champ_settings.threshold) {
            mouse.y -= (distance_mouse_cam_y * this->champ_settings.camera_scroll_speed_bottom); // <-- arbitrary value
        }
    }

	switch (camera_mode)
	{
		case Free:
			if (!camera_interface_is_hovered()) {
				vector2D_set_pos (target,
					(mouse.x + cam.x) / 2.0,
					(mouse.y + cam.y) / 2.0
				);
			}
		break;

		case RestoreCam :
			vector2D_set_pos(target, tmpcam.x, tmpcam.y);
			mempos_set(this->cam,    tmpcam.x, tmpcam.y);
			this->restore_tmpcam = false;
		break;

		case FollowEntity :
			vector2D_set_pos(target, entity.x, entity.y);
            camera_set_pos(entity.x, entity.y);
		break;

		case FocusSelf :
			vector2D_set_pos(target, champ.x, champ.y);
			camera_set_pos(champ.x, champ.y);
		break;

		case Normal:
		{
			// LMB is not set
			if (this->lmb.x == 0 || this->lmb.y == 0)
			{
				lmb_weight = 0.0;
			}
			else
			{
				lmb.x = champ.x - (this->translate_lmb.x);
				lmb.y = champ.y - (this->translate_lmb.y);
			}

			if (this->drag_request)
			{
				drag.x = (drag.x - mouse.x) * 10;
				drag.y = (drag.y - mouse.y) * 10;
			}

			if (this->global_weight_allies && this->global_weight_activated)
			{
				if (this->nb_allies_nearby == 0)
				{
					global_weight_allies = 0.0;
				}

				else
				{
					global_weight_allies = this->global_weight_allies;

					float sum_x = 0, sum_y = 0;

					for (int i = 0; i < this->nb_allies_nearby; i++)
					{
						sum_x += this->nearby_allies[i]->p.v.x;
						sum_y += this->nearby_allies[i]->p.v.y;
					}

					allies.x = sum_x / (float) this->nb_allies_nearby;
					allies.y = sum_y / (float) this->nb_allies_nearby;
				}
			}

			if (this->global_weight_ennemies && this->global_weight_activated)
			{
				if (this->nb_ennemies_nearby == 0)
				{
					global_weight_ennemies = 0.0;
				}

				else
				{
					global_weight_ennemies = this->global_weight_ennemies;

					float sum_x = 0, sum_y = 0;

					for (int i = 0; i < this->nb_ennemies_nearby; i++)
					{
						sum_x += this->nearby_ennemies[i]->p.v.x;
						sum_y += this->nearby_ennemies[i]->p.v.y;
					}

					ennemies.x = sum_x / (float) this->nb_ennemies_nearby;
					ennemies.y = sum_y / (float) this->nb_ennemies_nearby;
				}
			}

			if (this->focused_entity != NULL)
			{
				// ShareEntity is a Normal camera behavior + ally weight
				focus_weight = this->focus_weight;
				focus.x = this->focused_entity->p.v.x;
				focus.y = this->focused_entity->p.v.y;
			}

			if (this->hint_entity != NULL)
			{
				hint_weight = this->hint_weight;
				hint.x = this->hint_entity->p.v.x;
				hint.y = this->hint_entity->p.v.y;
			}

			float weight_sum;
			{
				// Distances
				float distance_mouse_champ    = vector2D_distance(&mouse, &champ);
				float distance_dest_champ     = vector2D_distance(&dest, &champ);
				float distance_allies_champ   = vector2D_distance(&allies, &champ);
				float distance_ennemies_champ = vector2D_distance(&ennemies, &champ);
				float distance_mouse_dest     = vector2D_distance(&dest, &mouse);

				// weighted averages
				// these values control how quickly the weights fall off the further you are
				// from the falloff distance
				float dest_falloff_rate  = 0.0001;
				float mouse_falloff_rate = 0.0001;
				float global_falloff_rate = 0.0001;

				float mouse_range_max = 1000.0;
				float dest_range_max  = 3000.0;
				float mouse_dest_range_max = 3000.0;

				// adjust weights based on distance
				if (distance_dest_champ > dest_range_max)
					dest_weight = dest_weight / (((distance_dest_champ - dest_range_max) * dest_falloff_rate) + 1.0);

				if (distance_mouse_champ > mouse_range_max)
					champ_weight = champ_weight * (((distance_mouse_champ - mouse_range_max) * mouse_falloff_rate) + 1.0);

				if (distance_allies_champ > this->distance_entity_nearby)
					global_weight_allies = global_weight_allies * (((distance_allies_champ - this->distance_entity_nearby) * global_falloff_rate) + 1.0);

				if (distance_ennemies_champ > this->distance_entity_nearby)
					global_weight_ennemies = global_weight_ennemies * (((distance_ennemies_champ - this->distance_entity_nearby) * global_falloff_rate) + 1.0);

				// if the mouse is far from dest, reduce dest weight (mouse is more important)
				if (distance_mouse_dest > mouse_dest_range_max)
					dest_weight = dest_weight / (((distance_mouse_dest - mouse_dest_range_max) / 1500.0) + 1.0);

				weight_sum = champ_weight + mouse_weight + dest_weight + focus_weight + hint_weight + global_weight_allies + global_weight_ennemies + lmb_weight;
			}

            // Compute the target (weighted averages)
			vector2D_set_pos(target,
            (drag.x + ((lmb.x   * lmb_weight) +
                    (dest.x     * dest_weight) +
					(hint.x     * hint_weight) +
                    (champ.x    * champ_weight) +
                    (mouse.x    * mouse_weight) +
                    (focus.x    * focus_weight) +
                    (allies.x   * global_weight_allies) +
                    (ennemies.x * global_weight_ennemies)))
                     / weight_sum,
            (drag.y + ((lmb.y   * lmb_weight) +
                    (dest.y     * dest_weight) +
					(hint.y     * hint_weight) +
                    (champ.y    * champ_weight) +
                    (mouse.y    * mouse_weight) +
                    (focus.y    * focus_weight) +
                    (allies.y   * global_weight_allies) +
                    (ennemies.y * global_weight_ennemies)))
                     / weight_sum
            );
		}
		break;

		case isTranlating:
			vector2D_set_pos(target,
				this->distance_translation.x + mouse.x,
				this->distance_translation.y + mouse.y
		);
		break;

		case Translate:
			vector2D_set_pos(target, champ.x + this->distance_translation.x, champ.y + this->distance_translation.y);
		break;

		case CenterCam:
			// Target the champion
            vector2D_set_pos(target, champ.x, champ.y);
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
		warning("Can't load \"%s\" settings in LoLCamera.ini (the section #%s doesn't exist)", section, section);
		return;
	}

	info("------------------------------------------------------------------");
	while (bb_queue_get_length(default_settings))
	{
		KeyVal *kv    = bb_queue_get_first(default_settings);

		char *setting = kv->key;
		char *value   = kv->res;

		char *possible_settings[] = {
			[CAMERA_SETTING_SCROLL_SPEED] 			 = "camera_scroll_speed",
			[CAMERA_SETTING_THRESHOLD] 				 = "threshold",
			[CAMERA_SETTING_SCROLL_SPEED_VERTICAL] 	 = "camera_scroll_speed_vertical",
			[CAMERA_SETTING_SCROLL_SPEED_HORIZONTAL] = "camera_scroll_speed_horizontal",
			[CAMERA_SETTING_SCROLL_SPEED_BOTTOM]     = "camera_scroll_speed_bottom",
		};

		for (int i = 0; i < sizeof_array(possible_settings); i++)
		{
			if (strcmp(setting, possible_settings[i]) == 0)
			{
				switch (i)
				{
					case CAMERA_SETTING_SCROLL_SPEED:
						this->champ_settings.camera_scroll_speed = atof (value) / 10000.0; // this controls smoothing, smaller values mean slower camera movement
						info("%s camera_scroll_speed = %f", section, this->champ_settings.camera_scroll_speed);
					break;

					case CAMERA_SETTING_THRESHOLD:
						this->champ_settings.threshold = atof (value); // minimum threshold before calculations halted because camera is "close enough"
						info("%s threshold = %f", section, this->champ_settings.threshold);
					break;

					case CAMERA_SETTING_SCROLL_SPEED_VERTICAL:
						this->champ_settings.camera_scroll_speed_vertical = atof(value);
						info("%s camera_scroll_speed_vertical = %f", section, this->champ_settings.camera_scroll_speed_vertical);
					break;

					case CAMERA_SETTING_SCROLL_SPEED_HORIZONTAL:
						this->champ_settings.camera_scroll_speed_horizontal = atof(value);
						info("%s camera_scroll_speed_horizontal = %f", section, this->champ_settings.camera_scroll_speed_horizontal);
					break;

					case CAMERA_SETTING_SCROLL_SPEED_BOTTOM:
						this->champ_settings.camera_scroll_speed_bottom = atof(value);
						info("%s camera_scroll_speed_bottom = %f", section, this->champ_settings.camera_scroll_speed_bottom);
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

	if ((this->drag_key = ini_parser_get_char(parser, "drag_key")) == 0)
		this->drag_key  = strtol(ini_parser_get_value(parser, "drag_key"), NULL, 16);

	// Settings
	this->champ_weight = atof(ini_parser_get_value(parser, "champ_weight"));
	this->mouse_weight = atof(ini_parser_get_value(parser, "mouse_weight"));
	this->dest_weight  = atof(ini_parser_get_value(parser, "dest_weight"));
	this->lmb_weight   = atof(ini_parser_get_value(parser, "lmb_weight"));
	this->focus_weight = atof(ini_parser_get_value(parser, "focus_weight"));
	this->hint_weight  = atof(ini_parser_get_value(parser, "hint_weight"));
	this->global_weight_allies = atof(ini_parser_get_value(parser, "global_weight_allies"));
	this->global_weight_ennemies = atof(ini_parser_get_value(parser, "global_weight_ennemies"));

	this->sleep_time  = strtol(ini_parser_get_value(parser, "sleep_time"), NULL, 10); // Time slept between two camera updates (in ms)
	this->poll_data	  = strtol(ini_parser_get_value(parser, "poll_data"), NULL, 10); // Retrieve data from client every X loops

	this->disable_fx_keys = strtol(ini_parser_get_value(parser, "disable_fx_keys"), NULL, 10);
	this->wait_loading_screen = strtol(ini_parser_get_value(parser, "wait_loading_screen"), NULL, 10);
	this->ms_after_minimap_click = strtol(ini_parser_get_value(parser, "ms_after_minimap_click"), NULL, 10);
	this->distance_entity_nearby = atof(ini_parser_get_value(parser, "distance_entity_nearby"));

	// Champion Settings
	camera_load_settings("Default");
	if (!str_equals(this->section_settings_name, "Default"))
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

	for (int i = 0; i < sizeof_array(tabSet); i++)
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
		if (this->patchlist != NULL)
			patch_list_set(this->patchlist, false);

		this->mp = NULL;
		this = NULL;
	}
}
