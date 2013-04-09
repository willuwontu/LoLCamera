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
	0xF3,0x0F,0x11,0x15,0x3C,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x11,0x0D,0x40,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03  // xxxx????
};

unsigned char set_camera_pos_click_minimap_sig [] = {
/*
	Address   Hex dump                 Command                                                  Comments
	00B89822  ║·  F30F111D 3C71DF03    movss [dword ds:League_of_Legends.CameraX], xmm3         ; float 450.0000 <-- CameraX
	00B8982A  ║·  F30F1125 4071DF03    movss [dword ds:League_of_Legends.3DF7140], xmm4         ; float 0.0
	00B89832  ║·  F30F112D 4471DF03    movss [dword ds:League_of_Legends.CameraY], xmm5         ; float 8467.621 <-- CameraY
*/
	0xF3,0x0F,0x11,0x1D,0x3C,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x11,0x25,0x40,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x11,0x2D,0x44,0x71,0xDF,0x03  // xxxx????
};

unsigned char set_camera_pos_click_minimap_sig2 [] = {
/*
	00B8972F  ║·  F30F1105 3C71DF03      movss [dword ds:League_of_Legends.CameraX], xmm0    ; float 547.5713
	00B89737  ║·  F30F1005 4071DF03      movss xmm0, [dword ds:League_of_Legends.3DF7140]    ; float 0.0
	00B8973F  ║·  F30F58C2               addss xmm0, xmm2
	00B89743  ║·  F30F1105 4071DF03      movss [dword ds:League_of_Legends.3DF7140], xmm0    ; float 0.0
	00B8974B  ║·  F30F1005 4471DF03      movss xmm0, [dword ds:League_of_Legends.CameraY]    ; float 4717.657
	00B89753  ║·  F30F58C3               addss xmm0, xmm3
	00B89757  ║·  F30F1105 4471DF03      movss [dword ds:League_of_Legends.CameraY], xmm0    ; float 4717.657
*/
	0xF3,0x0F,0x11,0x05,0x3C,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x10,0x05,0x40,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x58,0xC2,					 // xxxx
	0xF3,0x0F,0x11,0x05,0x40,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x10,0x05,0x44,0x71,0xDF,0x03, // xxxx????
	0xF3,0x0F,0x58,0xC3,					 // xxxx
	0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03  // xxxx????
};

unsigned char reset_camera_when_champ_respawns_sig [] = {
/*
	Address   Hex dump                Command                                               Comments
	00A09334  ║·  F30F1105 3C71DF03   movss [dword ds:League_of_Legends.CameraX], xmm0      ; float 918.4202
	00A0933C  ║·  F30F1043 04         movss xmm0, [dword ds:ebx+4]
	00A09341  ║·  F30F1105 4071DF03   movss [dword ds:League_of_Legends.3DF7140], xmm0      ; float 0.0
	00A09349  ║·  F30F1043 08         movss xmm0, [dword ds:ebx+8]
	00A0934E  ║·  F30F1105 4471DF03   movss [dword ds:League_of_Legends.CameraY], xmm0      ; float 1154.925
*/
	0xF3,0x0F,0x11,0x05,0x3C,0x71,0xDF,0x03,	// xxxx????
	0xF3,0x0F,0x10,0x43,0x04,				 	// xxxxx
	0xF3,0x0F,0x11,0x05,0x40,0x71,0xDF,0x03,	// xxxx????
	0xF3,0x0F,0x10,0x43,0x08,					// xxxxx
	0xF3,0x0F,0x11,0x05,0x44,0x71,0xDF,0x03		// xxxx????
};

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

		camera_default_set_patch(this->enabled);
		camera_reset_when_respawn_set_patch(this->enabled);
	}

	// Disable when space is pressed
	if (GetKeyState(VK_SPACE) < 0)
	{
		/*	BUGFIX:
		*	bug :	When space is kept pressed, LoLCamera saves the last camera position (before it gets centered on the champion)
		*		  	When space is released, the camera returns in the last position saved -> weird camera moves
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
		*	fix:	Save the camera state when the left mouse button is pressed, and restore it when it is released
		*/
		if (!left_button_pressed)
		{
			// save camera state
			// todo
			left_button_pressed = TRUE;
		}

		return 0;
	}
	else
	{
		if (left_button_pressed) {
			// restore it
			// todo
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

void camera_init_patch ()
{
	// TODO : get .text section offset + size properly (shouldn't be really necessarly though)
	DWORD text_section = this->mp->base_addr + 0x1000;
	unsigned int text_size = 0x008B7000;

	info("Scanning process...");
	memproc_dump(this->mp, text_section, text_section + text_size);

	// Search for camera positionning instructions
	info("\n------------------------------------------------------------------\nLooking for addresses ...");
	camera_search_signature(set_camera_pos_sig, &this->default_camera_addr,
							"xxxx????xxxx????xxxx????", "Default camera positionning");

	camera_search_signature(
		set_camera_pos_click_minimap_sig, &this->minimap_camera_addr, "xxxx????xxxx????xxxx????",
		"Minimap camera positionning"
	);

	camera_search_signature(
		set_camera_pos_click_minimap_sig2, &this->minimap_camera_addr2, "xxxx????xxxx????xxxxxxxx????xxxx????xxxxxxxx????",
		"Minimap camera positionning 2"
	);

	camera_search_signature(
		reset_camera_when_champ_respawns_sig, &this->reset_cam_respawn_addr,"xxxx????xxxxxxxxx????xxxxxxxxx????",
		"Reset when the champion respawns"
	);

	info("\n------------------------------------------------------------------\n");
}

void camera_search_signature (unsigned char *pattern, DWORD *addr, char *mask, char *name)
{
	memproc_search(this->mp, pattern, mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(this->mp);

	if (bb_queue_get_length(results) <= 0) {
		warning("%s address not found (already patched ?)\nUsing the current .ini value : 0x%.8x", name, *addr);
		return;
	}

	if (bb_queue_get_length(results) > 1) {
		warning("Multiple occurences of %s found", name);
	}

	MemBlock *memblock = bb_queue_pick_first(results);
	*addr = memblock->addr;
	info("%s found = 0x%.8x", name, *addr);

	bb_queue_free_all(results, memblock_free);
}

void camera_reset_when_respawn_set_patch (BOOL patch_active)
{
	if (!this->reset_cam_respawn_addr)
	{
		warning("Camera reset respawn : patching isn't possible");
		return;
	}

	if (patch_active)
	{
		// NOP those bytes :
		/*
			->	$ ==>     ║·  F30F1105 3C71DF03   movss [dword ds:League_of_Legends.CameraX], xmm0      ; float 918.4202
				$+8       ║·  F30F1043 04         movss xmm0, [dword ds:ebx+4]
				$+D       ║·  F30F1105 4071DF03   movss [dword ds:League_of_Legends.3DF7140], xmm0      ; float 0.0
				$+15      ║·  F30F1043 08         movss xmm0, [dword ds:ebx+8]
			->	$+1A      ║·  F30F1105 4471DF03   movss [dword ds:League_of_Legends.CameraY], xmm0      ; float 1154.925
		*/
		char buffer[] = "\x90\x90\x90\x90\x90\x90\x90\x90";

		if (write_to_memory(this->mp->proc, buffer, this->reset_cam_respawn_addr,		 sizeof(buffer)-1)
		&&  write_to_memory(this->mp->proc, buffer, this->reset_cam_respawn_addr + 0x1A, sizeof(buffer)-1))
		{
			info("Camera reset respawn : Patch successful");
		}

		else
			warning("Camera reset respawn : Patch unsuccessful (0x%.8x)", this->reset_cam_respawn_addr);
	}

	else
	{
		// Restore the initial bytes
		if (write_to_memory(this->mp->proc, reset_camera_when_champ_respawns_sig, this->reset_cam_respawn_addr, sizeof(reset_camera_when_champ_respawns_sig)))
			info("Camera reset respawn : Unpatch successful");
		else
			warning("Camera reset respawn : Unpatch unsuccessful (0x%.8x)", this->reset_cam_respawn_addr);
	}
}

void camera_default_set_patch (BOOL patch_active)
{
	if (!this->default_camera_addr)
	{
		warning("Camera default : patching isn't possible");
		return;
	}

	if (patch_active)
	{
		// We must NOP those bytes :
		// $ ==>	F30F1115 3C71DF03		 movss [dword ds:League_of_Legends.CameraX], xmm2			   ; float 450.0000  <-- This
		// $+8		F30F110D 4071DF03		 movss [dword ds:League_of_Legends.3DF7140], xmm1			   ; float 0.0
		// $+10		F30F1105 4471DF03		 movss [dword ds:League_of_Legends.CameraY], xmm0			   ; float 3846.329  <-- and this
		char buffer[] = "\x90\x90\x90\x90\x90\x90\x90\x90";

		if (write_to_memory(this->mp->proc, buffer, this->default_camera_addr,		  sizeof(buffer)-1)
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
			info("Camera default : Unpatch successful");
		else
			warning("Camera default : Unpatch unsuccessful (0x%.8x)", this->default_camera_addr);
	}
}

void camera_minimap_set_patch (BOOL patch_active)
{
	if (!this->minimap_camera_addr || !this->minimap_camera_addr2)
	{
		warning("Camera minimap : patching isn't possible");
		return;
	}

	if (patch_active)
	{
		/* We must NOP those bytes :
		$ ==>  ║·  F30F111D 3C71DF03    movss [dword ds:League_of_Legends.CameraX], xmm3   ; float 450.0000 <-- Here
		$+8    ║·  F30F1125 4071DF03    movss [dword ds:League_of_Legends.3DF7140], xmm4   ; float 0.0
		$+10   ║·  F30F112D 4471DF03    movss [dword ds:League_of_Legends.CameraY], xmm5   ; float 8467.621 <-- and here
		*/
		char buffer[] = "\x90\x90\x90\x90\x90\x90\x90\x90";

		if (write_to_memory(this->mp->proc, buffer, this->minimap_camera_addr,		  sizeof(buffer)-1)
		&&  write_to_memory(this->mp->proc, buffer, this->minimap_camera_addr + 0x10, sizeof(buffer)-1))
		{
			info("Camera minimap : Patch successful");
		}

		else
			warning("Camera minimap : Patch unsuccessful (0x%.8x)", this->minimap_camera_addr);

		/* And these :
		$ ==>     ║·  F30F1105 3C71DF03      movss [dword ds:League_of_Legends.CameraX], xmm0    ; float 535.3584 <--
		$+8       ║·  F30F1005 4071DF03      movss xmm0, [dword ds:League_of_Legends.3DF7140]    ; float 0.0
		$+10      ║·  F30F58C2               addss xmm0, xmm2
		$+14      ║·  F30F1105 4071DF03      movss [dword ds:League_of_Legends.3DF7140], xmm0    ; float 0.0
		$+1C      ║·  F30F1005 4471DF03      movss xmm0, [dword ds:League_of_Legends.CameraY]    ; float 4647.248
		$+24      ║·  F30F58C3               addss xmm0, xmm3
		$+28      ║·  F30F1105 4471DF03      movss [dword ds:League_of_Legends.CameraY], xmm0    ; float 4647.248 <--
		*/

		if (write_to_memory(this->mp->proc, buffer, this->minimap_camera_addr2,		   sizeof(buffer)-1)
		&&  write_to_memory(this->mp->proc, buffer, this->minimap_camera_addr2 + 0x28, sizeof(buffer)-1))
		{
			info("Camera minimap 2 : Patch successful");
		}

		else
			warning("Camera minimap 2 : Patch unsuccessful (0x%.8x)", this->minimap_camera_addr2);

	}

	else
	{
		// Restore the initial bytes
		if (write_to_memory(this->mp->proc, set_camera_pos_click_minimap_sig, this->minimap_camera_addr, sizeof(set_camera_pos_click_minimap_sig)))
			info("Camera minimap : Unpatch successful");
		else
			warning("Camera minimap : Unpatch unsuccessful (0x%.8x)", this->minimap_camera_addr);

		if (write_to_memory(this->mp->proc, set_camera_pos_click_minimap_sig2, this->minimap_camera_addr2, sizeof(set_camera_pos_click_minimap_sig2)))
			info("Camera minimap : Unpatch successful");
		else
			warning("Camera minimap : Unpatch unsuccessful (0x%.8x)", this->minimap_camera_addr2);
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
	this->mouse_range_max = atof(ini_parser_get_value(parser, "mouse_range_max"));
	this->dest_range_max  = atof(ini_parser_get_value(parser, "dest_range_max"));
	this->shop_is_opened_addr = strtol(ini_parser_get_value(parser, "shop_is_opened_addr"), NULL, 16);
	this->reset_cam_respawn_addr = strtol(ini_parser_get_value(parser, "reset_cam_respawn_addr"), NULL, 16);


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

	this->enabled = TRUE;
	this->mp = mp;

	camera_load_ini();

	this->active = TRUE;

	// We wait for the client to be fully ready (in game) before patching
	this->request_polling = TRUE;
	while (!camera_update())
	{
		warning("Loading screen detected");
		Sleep(1000);
	}

	camera_init_patch();
	camera_default_set_patch(TRUE);
	//camera_minimap_set_patch(TRUE);
	camera_reset_when_respawn_set_patch(TRUE);
}

inline void camera_set_active (BOOL active)
{
	this->active = active;
}

int camera_shop_is_opened ()
{
	// Shop is open is the address of the pointer to the "isShopOpened"
	DWORD addr = read_memory_as_int(this->mp->proc, this->shop_is_opened_addr);

	if (!addr)
		return 0;

	// isShopOpen = edi+7c
	addr = addr + 0x7c;

	unsigned char buffer[1];
	read_from_memory(this->mp->proc, buffer, addr, 1);
	return (int) buffer[0];
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
		||  !mempos_refresh(this->mouse))
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
		camera_default_set_patch(FALSE);
		camera_reset_when_respawn_set_patch(FALSE);
		camera_minimap_set_patch(FALSE);
	}
}
