/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1bo5eg/new_camera_idea_my_solution/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"

int main()
{
	info("Sources : https://github.com/Spl3en/LoLCamera");

	enable_debug_privileges();

	// Force unpatch at exit
	atexit(camera_unload);

	while (1)
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

		info("Game detected, main loop started");

		camera_init(mp);
		camera_main();
		camera_unload();

		memproc_free(mp);
	}

	return 0;
}
