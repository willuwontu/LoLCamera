/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1bo5eg/new_camera_idea_my_solution/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"

int main()
{
	BOOL exit_request = FALSE;

	console_set_size(150, 30);
	info("Sources : https://github.com/Spl3en/LoLCamera");

	if (!enable_debug_privileges())
	{
		warning("Debug privileges aren't active");
	}

	// Force unpatch at exit
	atexit(camera_unload);

	while (!exit_request)
	{
		MemProc *mp = memproc_new("League of Legends.exe", "League of Legends (TM) Client");
		memproc_set_default_baseaddr(mp, 0x00400000);

		if (!mp->proc)
		{
			// Client is not detected
			info("Waiting for a new game ...");

			while (!memproc_refresh_handle(mp))
				Sleep(1000);
		}

		info("Game detected, main loop started (press 'x' to quit)");

		camera_init(mp);

		exit_request = camera_main();
		camera_unload();

		memproc_free(mp);
	}

	return 0;
}
