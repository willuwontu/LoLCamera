/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1bo5eg/new_camera_idea_my_solution/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"
#include <signal.h>

int main()
{
	LoLCameraState state = PLAY;
	MemProc *mp = NULL;

	console_set_size(1200, 600);
	important("Sources      : https://github.com/Spl3en/LoLCamera");
	important("Last version : https://sourceforge.net/projects/lolcamera/files");
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
			while (mp->proc)
			{
				memproc_refresh_handle(mp);
				Sleep(5000);
				info("Waiting for the end of the game... 5s pause");
			}
		}

		mp = memproc_new("League of Legends.exe", "League of Legends (TM) Client");
		memproc_set_default_baseaddr(mp, 0x00400000);

		if (!mp->proc)
		{
			// Client is not detected
			info("Waiting for a new game ...");

			while (!memproc_refresh_handle(mp))
			{
				if (exit_request())
					return 0;

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
