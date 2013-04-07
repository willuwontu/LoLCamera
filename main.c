/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1bo5eg/new_camera_idea_my_solution/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"

int main()
{
	info("Sources : https://github.com/Spl3en/LoLCamera");

	MemProc *mp = memproc_new("League of Legends.exe", "League of Legends (TM) Client");

	if (!mp->proc)
	{
		error("Please launch a game");
		return 0;
	}

	info("Game detected, main loop started");

	// Force unpatch at exit
	atexit(camera_unload);

	camera_init(mp);
	camera_main();
	camera_unload();

	return 0;
}
