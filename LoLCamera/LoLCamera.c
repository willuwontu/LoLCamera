#include "LoLCamera.h"

// Todo : Find a userfriendly way to put TOGGLE_KEY in the .ini
#define TOGGLE_KEY	 VK_F11   // which key toggles the camera adjustments

// Singleton
static Camera *this = NULL;

typedef enum {
    Normal,
    CenterCam,
    NoMove,
    NoUpdate
} CameraTrackingMode;

static CameraTrackingMode camera_is_enabled ()
{
	static short last_toggle_state = 0;

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
		patch_set_active(this->locked_camera, this->enabled);
	}

	// Disable when space is pressed
	if (GetKeyState(VK_SPACE) < 0 || (GetKeyState(VK_F1) < 0))
    {
    	// Polling data is requested because we want to center the camera exactly where the champion is
        this->request_polling = TRUE;
        return CenterCam;
    }

	// to allow minimap navigation, also disabled if LMB is down
	if (GetKeyState(VK_LBUTTON) < 0)
		return NoMove;

	//
	if (GetKeyState(VK_MBUTTON) < 0)
        return NoMove;

	// Disable camera when shop opened
	if (this->shop_opened)
        return NoMove;

	// skip the next loop if not enabled
	if (!this->enabled)
        return NoUpdate;

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
	this->shop_opened = FALSE;

	camera_load_ini();

	// Dumping process
	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	info("Dumping process...");
	memproc_dump(this->mp, text_section, text_section + text_size);

	// Zeroing
	memset(this->champions, 0, sizeof(Entity *));

	// Scanning for variables address
	camera_scan_variables();

	// Signature scanning for patches
	camera_scan_patch();
	camera_scan_champions();
	camera_scan_mouse_screen();
	camera_scan_shop_is_opened();

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

	// We wait for the client to be fully ready (in game) before patching
	this->request_polling = TRUE;

	while (!camera_update()) {
		warning("Loading screen detected");
	}

	patch_set_active(this->border_screen, TRUE);
	patch_set_active(this->F2345_pressed[0], TRUE);
	patch_set_active(this->F2345_pressed[1], TRUE);
	patch_set_active(this->respawn_reset, TRUE);
	patch_set_active(this->locked_camera, TRUE);

	this->active = TRUE;
}

BOOL camera_refresh_champions ()
{
	// We can't rely on this information to decide if LoLCam is synchronized;
	// MemPos already does that job correctly
	// returns TRUE so it's ignored in camera_update()
	if (!this->active)
		return TRUE;

	BOOL already_scanned = FALSE;
	DWORD entity_ptr     = read_memory_as_int(this->mp->proc, this->entities_addr);
	DWORD entity_ptr_end = read_memory_as_int(this->mp->proc, this->entities_addr_end);

	for (int i = 0; entity_ptr != entity_ptr_end; entity_ptr+=4, i++)
	{
		if (!entity_refresh(this->champions[i]))
		{
			if (this->champions[i])
				warning("Entity 0x%.8x cannot be refreshed", this->champions[i]->addr);

			// We can't rely on this information to decide if LoLCam is synchronized
			// However, we can try to fix this issue by loading again the entities array adresses
			if (!already_scanned)
				camera_scan_champions ();

			already_scanned = TRUE;
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
		||  !mempos_int_refresh(this->mouse_screen)
		||  !camera_refresh_champions()
		||  !camera_refresh_shop_is_opened())
		{
			// Synchronization seems not possible
			if (!memproc_refresh_handle(this->mp))
			{
				info("Client not detected anymore.");
				this->active = FALSE;
				return FALSE;
			}

			if (trying_sync == TRUE) // One try before pausing
			{
				warning("Synchronization with the client isn't possible - Retrying in 3s.");
				Sleep(3000);
			}

			trying_sync = TRUE;

			// Resynchronize with the process
			camera_scan_shop_is_opened();
			camera_scan_mouse_screen();
			camera_scan_champions();

			return FALSE;
		}

		else if (trying_sync) {
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
		CameraTrackingMode camera_mode = camera_is_enabled();

		if (camera_mode == NoUpdate || !camera_update())
			continue;

        float distance_mouse_champ = vector2D_distance(&this->mouse->v, &this->champ->v);
        float distance_dest_champ = vector2D_distance(&this->dest->v, &this->champ->v);
        float distance_mouse_dest = vector2D_distance(&this->dest->v, &this->mouse->v);

        {
            // weighted averages
            float champ_weight = 1.0;
            float mouse_weight = 1.0;
            float dest_weight = 1.0;

            // these values control how quickly the weights fall off the further you are
            // from the falloff distance
            float dest_falloff_rate = 0.001;
            float mouse_falloff_rate = 0.002;

            // adjust weights based on distance
            if (distance_dest_champ > this->dest_range_max)
                dest_weight = 1 / (((distance_dest_champ - this->dest_range_max) * dest_falloff_rate) + 1.0);

            if (distance_mouse_champ > this->mouse_range_max)
                mouse_weight = 1 / (((distance_mouse_champ - this->mouse_range_max) * mouse_falloff_rate) + 1.0);
            else if (distance_mouse_dest > this->mouse_dest_range_max)
                // increase mouse weight if far from dest
                mouse_weight += (distance_mouse_dest - this->mouse_dest_range_max) / 1000.0;

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
                target.y -= distance_mouse_champ / 8.0; // <-- arbitrary value
        }

        // Smoothing
		if (abs(target.x - this->cam->v.x) > this->threshold)
			this->cam->v.x += (target.x - this->cam->v.x) * this->lerp_rate;

		if (abs(target.y - this->cam->v.y) > this->threshold)
			this->cam->v.y += (target.y - this->cam->v.y) * this->lerp_rate;

        if (camera_mode == NoMove)
            continue;

        // update the ingame gamera position
        if (camera_mode == CenterCam)
        {
            mempos_set(this->cam, this->champ->v.x, this->champ->v.y);
        }
        else
		{
			mempos_set(this->cam, this->cam->v.x, this->cam->v.y);
		}
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
	this->locked_camera_addr = strtol(ini_parser_get_value(parser, "locked_camera_addr"), NULL, 16);

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
        { .addr = this->shop_is_opened_ptr,.str = "shop_is_opened_ptr" },//	That kirby has been watching for me since the beggining of the project, it deserves its place in the source code :)
        { .addr = this->respawn_reset_addr, .str = "respawn_reset_addr" }, //
        { .addr = this->border_screen_addr, .str = "border_screen_addr" }, //                  ██████████            ,---------------------.
        { .addr = this->champx_addr,        .str = "champion_posx_addr" }, //              ████░░      ░░████        |  imgur.com/74bnbGP  |
        { .addr = this->champy_addr,        .str = "champion_posy_addr" }, //            ██░░              ░░██      \____________  _______/
        { .addr = this->locked_camera_addr, .str = "locked_camera_addr" }, //          ██                    ░░██                 |/
        { .addr = this->mouse_screen_ptr,   .str = "mouse_screen_ptr" },   //        ██              ██  ██  ░░██                 `
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
	};

	// Todo : corresponding scanning function corresponding to each data to get directly in the client
	for (int i = 0; i < sizeof(tabAddr) / sizeof(struct AddrStr); i++)
		if (!tabAddr[i].addr)
			warning("\"%*s\" cannot be read in .ini file", 30 - strlen(tabAddr[i].str), tabAddr[i].str);

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
		patch_set_active(this->border_screen, FALSE);
		patch_set_active(this->F2345_pressed[0], FALSE);
		patch_set_active(this->F2345_pressed[1], FALSE);
		patch_set_active(this->respawn_reset, FALSE);
		patch_set_active(this->locked_camera, FALSE);
	}
}
