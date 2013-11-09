/**
 *  @author		:	Spl3en (Moreau Cyril) <spl3en.contact@gmail.com>
 *  @file		:	EasySocket.h
 *  @version	:	1.0
 *  @date		:	2011-04-12-16.51
 *
 *
 *  EasySocket propose un ensemble de méthodes permettant l'utilisation agréable de socket en C.
 *  La librairie permet de faire abstraction de l'utilisation des threads par callback.
 *
 *  Exemple :
 *  --------------------------------------------------------------------------------------
 *  void callback_client (EasySocketListened *esl)
 *  {
 *      printf("Hi ! You said : %s\n", esl->buffer);
 *  }
 *
 *  void finish_callback (EasySocketListened *esl)
 *  {
 *      es_listener_free(esl, free);
 *      printf("Client is freed!");
 *  }
 *
 *  int main()
 *  {
 *      es_init();
 *
 *      EasySocket *sock;
 *      EasySocketListened *client;
 *      char buffer[100];
 *
 *      // Example de serveur
 *      sock = es_server_new(1337, 100);
 *
 *      while (1)
 *      {
 *          client = es_accept(sock, 1024);
 *          es_listener(client, callback_client, finish_callback);
 *      }
 *
 *      // Exemple de client
 *      sock = es_client_new_from_host("localhost", 1337);
 *
 *      if (sock != ES_ERROR_CONNECT
 *      && sock != ES_ERROR_MALLOC)
 *      {
 *          es_send(sock, "Hello", -1);
 *          es_recv(sock, buffer, 99);
 *
 *          printf("Recu : %s\n", buffer);
 *      }
 *
 *      return 0;
 *  }
 *
 *
*/

#ifndef EasySocket_H_INCLUDED
#define EasySocket_H_INCLUDED

/* Includes */
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <assert.h>

#include "../Ztring/Ztring.h"

#define ES_ERROR_BIND       ((void*)-1)
#define ES_ERROR_LISTEN     ((void*)-2)
#define ES_ERROR_CONNECT    ((void*)-3)
#define ES_ERROR_MALLOC     ((void*)-4)

// Working Macros
#define _es_get_data(esl) \
    (esl->_data)

#define _es_set_data(esl, data) \
    (esl->_data = data)

#define EASY_SOCKET(object) \
    ((EasySocket *)object)

#define es_is_connected(es) \
    (es->is_connected)

typedef
struct _EasySocket
{
    SOCKET sock;
    int is_connected;

}	EasySocket;

typedef
struct _EasySocketListened
{
    SOCKET sock;
    HANDLE thread;

    int is_connected;
    char *buffer;
    int bsize;

    void *_data;

}   EasySocketListened;

typedef
struct _EasySocketListenerArgs
{
    EasySocketListened *esl;
    void (*callback)();
    void (*finish_callback)();

}   EasySocketListenerArgs;


	/**=================
		@Constructors
	===================*/


EasySocket *
es_client_new_from_ip (char *ip, int port);

EasySocket *
es_client_new_from_host (char *hostname, int port);

EasySocket *
es_server_new (int port, int max_connection);

	/**=================
		  @Methods
	===================*/

int
es_init(void);

EasySocketListened *
es_accept(EasySocket *server, int buffer_size_allocated);

void
es_listener (EasySocketListened *esl, void (*recv_callback)(EasySocketListened *sock), void (*finish_callback)(EasySocketListened *sock));

char *
es_get_ip_from_hostname (char *addr);

void
es_send(EasySocket *es, char *msg, int len);

int
es_recv(EasySocket *es, char *buffer, int len);

int
es_close(EasySocket *es);

void
es_end();

void *
_es_func_get_data (EasySocketListened *esl);

void
_es_func_set_data (EasySocketListened *esl, void *data);

void
es_set_connected(EasySocket *es, int is_connected);

void
es_send_http_request(EasySocket *es, char *msg);

char *
es_get_http_file (EasySocket *es, char *path, char *host);

	/**=================
		@Destructors
	===================*/

void
es_free (EasySocket *p);

void
es_listener_free (EasySocketListened *esl, void (*free_data_func)());

#endif
