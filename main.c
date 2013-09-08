/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1jgfn8/lolcamera_first_release_tool/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"
#include <signal.h>

int main()
{
	LoLCameraState state = PLAY;
	MemProc *mp = NULL;

	console_set_size(1200, 600);
	important("Sources & Hotkeys : https://github.com/Spl3en/LoLCamera");
	important("Last .exe version : https://sourceforge.net/projects/lolcamera/files");
	important("------------------------------------------------------------------");
	important("Keep pressing X in this console to exit safely (strongly recommanded)");
	important("------------------------------------------------------------------");

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

		state = camera_main();
		camera_unload();

		if (state == END_OF_LOLCAMERA)
			memproc_free(mp);
	}

	exit(0);
}
