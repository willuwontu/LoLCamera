/**
*	Discussion thread	: http://www.reddit.com/r/leagueoflegends/comments/1qcjj6/lolcamera_is_working_again/
*	Source				: https://github.com/Spl3en/LoLCamera
*/

#include "./LoLCamera/LoLCamera.h"
#include "./Webserver/Webserver.h"
#include "./Crypto/md5.h"
#include <signal.h>

#define LOLCAMERA_VERSION 0.233

char download_link[] = "https://sourceforge.net/projects/lolcamera/files";
char download_host[] = "cznic.dl.sourceforge.net";
char download_path[] = "/project/lolcamera/LoLCamera%20exe%2Bini.zip";

void download_lolcamera (char *link)
{
    EasySocket *sourceforge = es_client_new_from_host(download_host, 80);

    (void) sourceforge;
    (void) link;
}

char * get_own_patchnotes ()
{
    return file_get_contents("./patchnotes.txt");
}

char * get_own_md5 (char *filename)
{
	FILE *file = file_open(filename, "rb");

	if (!file)
        return NULL;

	char *md5 = MD5_file(file);
	return md5;
}

int main_light ()
{
	MemProc *mp = NULL;

	mp = memproc_new("League of Legends.exe", "League of Legends (TM) Client");
	memproc_set_default_baseaddr(mp, 0x00400000);
	memproc_refresh_handle(mp);

	info("Dumping process...");
	DWORD text_section = mp->base_addr + 0x1000;
	unsigned int text_size = EXECUTABLE_TEXT_SIZE;
	memproc_dump(mp, text_section, text_section + text_size);

	#ifdef REPAIR
		camera_init_light(mp);
		camera_scan_campos();
		camera_scan_camval();
		camera_scan_dest();
		camera_scan_cursor_champ();
		camera_scan_game_info();
		camera_scan_win_is_opened();
		camera_scan_hover_interface();
		camera_scan_victory();
		camera_refresh_entity_hovered();
		camera_scan_champions(true);
		camera_run_light();
		camera_scan_patch();
		camera_export_to_cheatengine();
	#endif

	return 0;
}

int main (int argc, char **argv)
{
    (void) argc;
    (void) argv;

	#ifdef REPAIR
		return main_light();
	#endif

	LoLCameraState state = PLAY;
	MemProc *mp = NULL;

	important("Sources & Hotkeys : https://github.com/Spl3en/LoLCamera");
	important("Last .exe version : https://sourceforge.net/projects/lolcamera/files");
	important("------------------------------------------------------------------");
	important("Keep pressing X in this console to exit safely (strongly recommended)");
	important("------------------------------------------------------------------");

    IniParser *parser = ini_parser_new("LoLCamera.ini");
    ini_parser_reg_and_read(parser);
    char * offline = (char *) ini_parser_get_value(parser, "offline");
    bool isOffline = (offline) ? atoi(offline) : 0;
    bool update_available = false;
    ini_parser_free(parser);

    #ifdef DEBUG
    isOffline = false;
    #endif // DEBUG

	// Check online version
	if (!isOffline)
    {
        info("Checking for updates ... (current version = %.3f)", LOLCAMERA_VERSION);
        char *str_last_version = webserver_do(GET_VERSION);
        float last_version = 0.0;

        if (str_last_version)
        {
            last_version = atof(str_last_version);
            free(str_last_version);
        }

        char *patchnotes;

        if ((int)(last_version * 1000) > (int)(LOLCAMERA_VERSION * 1000))
        {
            warning ("\n"
                      "    +-------------------------------------------------------------------+\n"
                      "    |                   A NEW UPDATE IS AVAILABLE                       |\n"
                      "    |             New version = %.3f, current version = %.3f          |\n"
                      "    |    Download : %s    |\n"
                      "    +-------------------------------------------------------------------+",
                        last_version, LOLCAMERA_VERSION, download_link);

            update_available = true;
        }
        else
            info("No updates.");

        info("Getting patchnotes ...");
        patchnotes = (update_available) ? webserver_do(GET_PATCHNOTES) : get_own_patchnotes();

        if (patchnotes != NULL)
        {
            char *ptr = patchnotes;
            char loops[] = {0, 0, 1};
            int pos;

            for (int i = 0; i < sizeof_array(loops); i++)
            {
                pos = str_pos_after(ptr, "===");

                if (loops[i])
                    ptr[pos-strlen("===") - 1] = '\0';
                else
                    ptr = &ptr[pos];
            }

            readable("LoLCamera Patch %.3f Notes :\n%s", last_version, patchnotes);
            free(patchnotes);
        }

        // Check integrity
        #ifndef DEBUG
        info("Checking executable integrity...");
        if (!str_equals(get_own_md5(argv[0]), webserver_do(GET_MD5, LOLCAMERA_VERSION)))
        {
            error("Integrity error : Please download the last version here : \n\t%s", download_link);
            system("pause");
            exit(EXIT_FAILURE);
        }
        #endif
    }

	// Debug privileges
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
			int already_displayed_message = false;

			while (mp->proc)
			{
				memproc_refresh_handle(mp);
				Sleep(2000);

				if (!already_displayed_message)
				{
					info("Waiting for the end of the game...");
					already_displayed_message = true;
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

            // if (update_available)
            //    readable("Keep pressing [U] if you want to download the new version.\n");

			while (!memproc_refresh_handle(mp))
			{
				int key;
				if ((key = get_kb()) != -1)
				{
					// User input
					if (exit_request(key))
						return 0;

                    if (update_available && update_request(key))
                        download_lolcamera(download_link);
				}

				Sleep(1000);
			}
		}

		info("Game detected, main loop started (press 'x' to quit)");

		camera_init(mp);
        camera_export_to_cheatengine();

		info("LoLCamera is ready and running !");

		state = camera_main();
		camera_unload();

		if (state == END_OF_LOLCAMERA)
        {
            memproc_free(mp);
            mp = NULL;
        }
	}

	// Force atexit event
	exit(0);
}
