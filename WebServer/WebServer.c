#include "WebServer.h"
#include <stdarg.h>

WebServer webserver = {
    .socket  = NULL,
    .version = NULL,
    .patchnotes = NULL,
    .md5 = NULL
};

void webserver_connect ()
{
    webserver.socket = es_client_new_from_host("lolcamera.alwaysdata.net", 80);
}

void webserver_disconnect ()
{
    es_free(webserver.socket);
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


// ----- Public

char * webserver_do (WebServerAction action, ...)
{
    char *contents = NULL;

    webserver_connect();

    switch (action)
    {
        case GET_VERSION:
            contents = es_http_get_contents(webserver.socket, "/version.txt");
        break;

        case GET_PATCHNOTES:
            contents = es_http_get_contents(webserver.socket, "/patchnotes.txt");
        break;

        case GET_MD5:
        {
            va_list args;
            va_start (args, action);
            double version = va_arg (args, double);
            va_end (args);

            contents = es_http_get_contents(webserver.socket, str_dup_printf("/md5/%.3f.txt", version));
        }
        break;

        default :
        break;
    }

    webserver_disconnect();

    return contents;
}
