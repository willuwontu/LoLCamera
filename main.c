/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1jgfn8/lolcamera_first_release_tool/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"
#include <signal.h>

#define LOLCAMERA_VERSION 0.171

float check_version (void)
{
	es_init();
	info("Checking for updates (current version : %.3f) ...", LOLCAMERA_VERSION);

	EasySocket *socket = es_client_new_from_host("lolcamera.alwaysdata.net", 80);
	char *version = es_get_http_file(socket, "/version.txt", "lolcamera.alwaysdata.net");
	es_free(socket);

	if (version != NULL)
	{
		if (atof(version) != LOLCAMERA_VERSION)
			return atof(version);
	}

	return 0.0;
}

int main_light ()
{
	MemProc *mp = NULL;

	mp = memproc_new("League of Legends.exe", "League of Legends (TM) Client");
	memproc_set_default_baseaddr(mp, 0x00400000);
	memproc_refresh_handle(mp);

	info("Dumping process...");
	DWORD text_section = mp->base_addr + 0x1000;
	unsigned int text_size = 0x00B1A000;
	memproc_dump(mp, text_section, text_section + text_size);

	// These calls produce warnings, keep it commented
	/*
	//
	camera_init_light(mp);

	//
	camera_scan_campos();
	camera_scan_camval();
	camera_scan_loading();
	camera_scan_dest();
	camera_scan_cursor_champ();
	camera_scan_game_info();
	camera_scan_win_is_opened();
	camera_scan_hover_interface();
	camera_scan_hovered_champ();
	camera_scan_victory();
	camera_refresh_entity_hovered();
	camera_scan_champions();

	//
	camera_run_light();
	camera_scan_patch();

	camera_export_to_cheatengine();
	*/

	return 0;
}

int main()
{
	//return main_light();
	LoLCameraState state = PLAY;
	MemProc *mp = NULL;

	important("Sources & Hotkeys : https://github.com/Spl3en/LoLCamera");
	important("Last .exe version : https://sourceforge.net/projects/lolcamera/files");
	important("------------------------------------------------------------------");
	important("Keep pressing X in this console to exit safely (strongly recommanded)");
	important("------------------------------------------------------------------");

	// Check online version
	float new_version = 0.0;
	if ((new_version = check_version()) != 0.0)
	{
		important("\n"
				  "    +-------------------------------------------------------------------+\n"
				  "    |                   A NEW UPDATE IS AVAILABLE                       |\n"
				  "    |             New version = %.3f, current version = %.3f          |\n"
				  "    |    Download : https://sourceforge.net/projects/lolcamera/files    |\n"
				  "    +-------------------------------------------------------------------+",
					new_version, LOLCAMERA_VERSION);
		system("pause");
	}

	if (!enable_debug_privileges())
	{
		warning("Debug privileges aren't active");
	}

	// Force unpatch at exit
	signal(SIGINT, camera_unload);
	atexit(camera_unload);

	while (state != END_OF_LOLCAMERA)
	{
		if (state == WAIT_FOR_END_OF_GAME)
		{
			int already_displayed_message = FALSE;

			while (mp->proc)
			{
				memproc_refresh_handle(mp);
				Sleep(2000);

				if (!already_displayed_message)
				{
					info("Waiting for the end of the game...");
					already_displayed_message = TRUE;
				}
			}
		}

		mp = memproc_new("League of Legends.exe", "League of Legends (TM) Client");
		memproc_set_default_baseaddr(mp, 0x00400000);
		memproc_refresh_handle(mp);

		if (!mp->proc)
		{
			// Client is not detected
			camera_reset();
			info("Waiting for a new game ...");

			while (!memproc_refresh_handle(mp))
			{
				int key;
				if ((key = get_kb()) != -1)
				{
					// User input
					if (exit_request(key))
						return 0;
				}

				Sleep(1000);
			}
		}

		info("Game detected, main loop started (press 'x' to quit)");

		camera_init(mp);

		camera_export_to_cheatengine();

		state = camera_main();
		camera_unload();

		if (state == END_OF_LOLCAMERA)
			memproc_free(mp);
	}

	// Force atexit event
	exit(0);
}
