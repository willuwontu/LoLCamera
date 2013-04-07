#include "LoLCamera.h"

// Todo : Find a userfriendly way to put TOGGLE_KEY in the .ini
#define TOGGLE_KEY	 VK_F11   // which key toggles the camera adjustments

unsigned char set_camera_pos_sig [] = {
/*
	Address   Hex dump					Command														Comments
	00A37E2A	F30F1115 3C71DF03		 movss [dword ds:League_of_Legends.CameraX], xmm2			   ; float 450.0000  <-- CameraX
	00A37E32	F30F110D 4071DF03		 movss [dword ds:League_of_Legends.3DF7140], xmm1			   ; float 0.0
	00A37E3A	F30F1105 4471DF03		 movss [dword ds:League_of_Legends.CameraY], xmm0			   ; float 3897.000  <-- CameraY
*/
	0xF3,0x0F,0x11,0x15,0x3C,0x71,0xDF,0x03,0xF3,0x0F,0x11,0x0D,0x40,0x71,0xDF,0x03,0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03
};

unsigned char set_camera_pos_click_minimap_sig [] = {
/*
	Address   Hex dump                 Command                                                  Comments
	00B89822  ║·  F30F111D 3C71DF03    movss [dword ds:League_of_Legends.CameraX], xmm3         ; float 450.0000 <-- CameraX
	00B8982A  ║·  F30F1125 4071DF03    movss [dword ds:League_of_Legends.3DF7140], xmm4         ; float 0.0
	00B89832  ║·  F30F112D 4471DF03    movss [dword ds:League_of_Legends.CameraY], xmm5         ; float 8467.621 <-- CameraY
*/
	0xF3,0x0F,0x11,0x1D,0x3C,0x71,0xDF,0x03,0xF3,0x0F,0x11,0x25,0x40,0x71,0xDF,0x03,0xF3,0x0F,0x11,0x2D,0x44,0x71,0xDF,0x03
};

// Singleton
static Camera *this = NULL;

static BOOL camera_is_enabled ()
{
	static BOOL enabled = TRUE;
	static short last_toggle_state = 0;
	static BOOL space_pressed = FALSE;

	// listen for toggle key
	short new_toggle_state = GetKeyState(TOGGLE_KEY);
	if (new_toggle_state != last_toggle_state && new_toggle_state >= 0)
	{
		enabled = !enabled;
		last_toggle_state = new_toggle_state;

		camera_default_set_patch(enabled);
	}

	// Disable when space is pressed
	if (GetKeyState(VK_SPACE) < 0)
	{
		/*	BUGFIX:
		*	When space is kept pressed, LoLCamera saves the last camera position (before it gets centered on the champion)
		*	When space is released, the camera returns in the last position saved -> weird camera moves
		*	fix : request polling data for the next loop, so the data saved are synchronized with the new camera
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
		return 0;

	// skip if not enabled
	return enabled;
}

void camera_init_patch ()
{
	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	info("Scanning process...");
	memproc_dump(this->mp, text_section, text_section + text_size);

	// Search for camera positionning instructions
	this->default_camera_addr    = camera_search_signature(set_camera_pos_sig, 			  "xxxxxxxxxxxxxxxx", "default camera positionning");
	this->minimap_camera_addr = camera_search_signature(set_camera_pos_click_minimap_sig, "xxxxxxxxxxxxxxxx", "minimap camera positionning");
}

DWORD camera_search_signature (unsigned char *pattern, char *mask, char *name)
{
	memproc_search(this->mp, pattern, "xxxxxxxxxxxxxxxx", NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);
	DWORD addr = 0;

	if (bb_queue_get_length(results) <= 0) {
		warning("%s address not found (already patched ?)", name);
		return 0;
	}

	if (bb_queue_get_length(results) > 1) {
		warning("Multiple occurences of %s found", name);
	}

	MemBlock *memblock = bb_queue_pick_first(results);
	addr = memblock->addr;
	info("%s found = 0x%.8x", name, addr);

	bb_queue_free_all(results, memblock_free);

	return addr;
}

void camera_default_set_patch (BOOL patch_active)
{
	if (!this->default_camera_addr)
	{
		warning("Camera movement patching isn't possible");
		return;
	}

	if (patch_active)
	{
		// We must NOP those bytes :
		// $ ==>	F30F1115 3C71DF03		 movss [dword ds:League_of_Legends.CameraX], xmm2			   ; float 450.0000  <-- This
		// $+8		F30F110D 4071DF03		 movss [dword ds:League_of_Legends.3DF7140], xmm1			   ; float 0.0
		// $+10		F30F1105 4471DF03		 movss [dword ds:League_of_Legends.CameraY], xmm0			   ; float 3846.329  <-- and this
		char buffer[] = "\x90\x90\x90\x90\x90\x90\x90\x90";

		if (write_to_memory(this->mp->proc, buffer, this->default_camera_addr,		   sizeof(buffer)-1)
		&&  write_to_memory(this->mp->proc, buffer, this->default_camera_addr + 0x10, sizeof(buffer)-1))
		{
			info("Camera default : Patch successful");
		}

		else
			warning("Patch unsuccessful (0x%.8x)", this->default_camera_addr);
	}

	else
	{
		// Restore the initial bytes
		if (write_to_memory(this->mp->proc, set_camera_pos_sig, this->default_camera_addr, sizeof(set_camera_pos_sig)))
			info("Unpatch successful");
		else
			warning("Unpatch unsuccessful (0x%.8x)", this->default_camera_addr);
	}
}

void camera_load_ini ()
{
	// Loading parameters from .ini file :
	IniParser *parser = ini_parser_new("LoLCamera.ini");
	ini_parser_reg_and_read(parser);

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
	// this->close_limit = atof(ini_parser_get_value(parser, "close_limit")); // don't move the camera when the cursor is near the champion
	// this->disable_if_too_far = atof(ini_parser_get_value(parser, "disable_if_too_far")); // condition "disable the camera if you go too far"
	//this->camera_far_limit = atof(ini_parser_get_value(parser, "camera_far_limit"));   // disable the camera if you go too far
	this->default_camera_addr = strtol(ini_parser_get_value(parser, "default_camera_addr"), NULL, 16);

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
		this = malloc(sizeof(Camera));

	this->mp = mp;

	camera_load_ini();

	this->active = TRUE;

	camera_init_patch();
	camera_default_set_patch(TRUE);
}

inline void camera_set_active (BOOL active)
{
	this->active = active;
}

BOOL camera_update ()
{
	static unsigned int frame_count = 0;

	if (frame_count++ % this->poll_data == 0 || this->request_polling)
	{
		if (!mempos_refresh(this->cam)
		||  !mempos_refresh(this->champ)
		||  !mempos_refresh(this->dest)
		||  !mempos_refresh(this->mouse))
		{
			// Synchronization seems not possible
			if (!memproc_refresh_handle(this->mp))
			{
				info("Client not detected anymore.");
				this->active = FALSE;
				return FALSE;
			}

			warning("Synchronization with the client isn't possible - Retrying in 5s.");
			Sleep(5000);
			return FALSE;
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

		// only calculate target position if mouse is within some distance of the champ
		// this is to keep the camera from jumping around when you hover the minimap
		// TODO: we probably want 2000 to be an adjustable value maybe
		// (although it's in world scale so it wont matter between resolution)
		if (distance_mouse_champ < 2000)
        {
            // Compute the target
            vector2D_set_pos(&target,
                (this->champ->v.x + this->mouse->v.x) / 2.0,
                (this->champ->v.y + this->mouse->v.y) / 2.0
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
		camera_default_set_patch(FALSE);
}
