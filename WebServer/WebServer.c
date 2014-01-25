#include "WebServer.h"
#include <stdarg.h>

WebServer webserver =
{
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
    es_close (webserver.socket);
    es_free  (webserver.socket);
}

// ----- Public -----

char * webserver_do (WebServerAction action, ...)
{
    char *contents = NULL;

    webserver_connect();

    switch (action)
    {
        case GET_VERSION:
            contents = es_http_get_contents(webserver.socket, "/version.txt");
            webserver.version = strdup(contents);
        break;

        case GET_PATCHNOTES:
            contents = es_http_get_contents(webserver.socket, "/patchnotes.txt");
            webserver.patchnotes = strdup(contents);
        break;

        case GET_MD5:
        {
            va_list args;
            va_start (args, action);
            double version = va_arg (args, double);
            va_end (args);

            contents = es_http_get_contents(webserver.socket, str_dup_printf("/md5/%.3f.txt", version));
            webserver.md5 = strdup(contents);
        }
        break;

        default :
        break;
    }

    webserver_disconnect();

    return contents;
}
