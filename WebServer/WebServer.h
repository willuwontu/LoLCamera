#pragma once

// ---------- Includes ------------
#include "../EasySocket/EasySocket.h"
#include "../Win32Tools/Win32Tools.h"
#include "../Crypto/md5.h"

// ---------- Defines -------------


// ------ Struct declaration -------

typedef
struct _WebServer
{
    EasySocket *socket;

    char *version;
    char *patchnotes;
    char *md5;

} WebServer;


typedef
enum _WebServerAction
{
    GET_MD5,
    GET_PATCHNOTES,
    GET_VERSION

} WebServerAction;

// --------- Constructors ---------

void webserver_connect ();

// ----------- Functions ------------

char * webserver_do (WebServerAction action, ...);

// --------- Destructors ----------

void webserver_disconnect ();


