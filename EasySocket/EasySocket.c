/**
 *	@author		:	Spl3en (Moreau Cyril) <spl3en.contact@gmail.com>
 *	@file		:	EasySocket.c
 *
 *	Furthermore informations in EasySocket.h
*/
#include "EasySocket.h"

#ifdef ES_WITHOUT_LINKED_LIBS
	int (*_WSAStartup) (WORD,LPWSADATA);
	int (*_WSACleanup) (void);
	int (*_listen)(SOCKET,int);
	int (*_bind) (SOCKET,const struct sockaddr*,int);
	int (*_recv)(SOCKET,char*,int,int);
	int (*_send)(SOCKET,const char*,int,int);
	int (*_connect) (SOCKET,const struct sockaddr*,int);
	int (*_closesocket) (SOCKET);
	SOCKET (*_accept) (SOCKET,struct sockaddr*,int*);
	SOCKET (*_socket)(int,int,int);
	u_long (*_htonl) (u_long);
	u_short (*_htons) (u_short);
	struct hostent * (*_gethostbyname) (const char*);
	unsigned long (*_inet_addr) (const char*);
	char * (*_inet_ntoa) (struct in_addr);

	#define recv _recv
	#define accept _accept
	#define send _send
	#define bind _bind
	#define listen _listen
	#define inet_ntoa _inet_ntoa
	#define gethostbyname _gethostbyname
	#define socket _socket
	#define htonl _htonl
	#define htons _htons
	#define inet_addr _inet_addr
	#define connect _connect
	#define WSAStartup _WSAStartup
	#define WSACleanup _WSACleanup
	#define closesocket _closesocket
#endif

/* Private Methods */

static void
_ex_es_listener (EasySocketListenerArgs *esla)
{
    EasySocketListened *esl = esla->esl;
    void (*_callback)() = esla->callback;
    void (*_finish_callback)() = esla->finish_callback;
    int b_read;

    while (esl->is_connected)
    {
		if ((b_read = recv(esl->sock, esl->buffer, esl->bsize, 0)) <= 0)
        {
            esl->is_connected = 0;
            continue;
        }

        esl->buffer[b_read-1] = '\0';
		_callback(esl);
    }

    _finish_callback(esl);
    CloseHandle(esl->thread);
}

/* Public Methods */

/**
 *    @Constructors
 */
EasySocketListenerArgs *
esla_new(EasySocketListened *esl, void (*callback)(), void (*finish_callback)())
{
    EasySocketListenerArgs *esla = NULL;
    if ((esla = malloc(sizeof(EasySocketListenerArgs))) == NULL)
        return ES_ERROR_MALLOC;

    esla->esl = esl;
    esla->callback = callback;
    esla->finish_callback = finish_callback;

    return esla;
}

EasySocket *
es_server_new (int port, int max_connection)
{
    EasySocket *p = NULL;

    SOCKET sock = 0;
    SOCKADDR_IN server_context;
    SOCKADDR_IN csin;
    int csin_size = sizeof(csin);

	if ((p = (EasySocket *) malloc (sizeof(EasySocket))) == NULL)
		return ES_ERROR_MALLOC;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    assert (sock != INVALID_SOCKET);

    server_context.sin_family      = AF_INET;
    server_context.sin_addr.s_addr = htonl(INADDR_ANY);
    server_context.sin_port        = htons(port);

    if (bind(sock, (SOCKADDR*)&server_context, csin_size) == SOCKET_ERROR)
        return ES_ERROR_BIND;

    if (listen(sock, max_connection) == SOCKET_ERROR)
        return ES_ERROR_LISTEN;

    p->sock = sock;
    p->is_connected = 1;

    return p;
}

EasySocket *
es_client_new_from_ip (char *ip, int port)
{
    EasySocket *p = NULL;

    SOCKET sock = 0;
    SOCKADDR_IN server_context;
    SOCKADDR_IN csin;
    int csin_size = sizeof(csin);

	if ((p = (EasySocket *) malloc (sizeof(EasySocket))) == NULL)
		return ES_ERROR_MALLOC;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    assert (sock != INVALID_SOCKET);

    server_context.sin_family      = AF_INET;
    server_context.sin_addr.s_addr = inet_addr(ip);
    server_context.sin_port        = htons (port);

    if (connect(sock, (SOCKADDR*)&server_context, csin_size) == SOCKET_ERROR)
        return ES_ERROR_CONNECT;

    p->sock = sock;
    p->is_connected = 1;

    return p;
}

EasySocket *
es_client_new_from_host (char *hostname, int port)
{
    EasySocket *p = NULL;
    char *ip = NULL;

    if ((ip = es_get_ip_from_hostname(hostname)) == NULL)
        return ES_ERROR_MALLOC;

    p = es_client_new_from_ip(ip, port);

	return p;
}

/**
 *  @Accessors
 */
// Existe en macro
void
_es_func_set_data (EasySocketListened *esl, void *data)
{
    esl->_data = data;
}

void *
_es_func_get_data (EasySocketListened *esl)
{
    return esl->_data;
}



/**
 *  @Methods
 */

int
es_init()
{
    WSADATA wsaData;

	#ifdef ES_WITHOUT_LINKED_LIBS
	HINSTANCE ws2_32 = LoadLibrary("ws2_32.dll");
	_recv = (void *) GetProcAddress (ws2_32, "recv");
	_send = (void *) GetProcAddress (ws2_32, "send");
	_bind = (void *) GetProcAddress (ws2_32, "bind");
	_accept = (void *) GetProcAddress (ws2_32, "accept");
	_gethostbyname = (void *) GetProcAddress (ws2_32, "gethostbyname");
	_socket = (void *) GetProcAddress (ws2_32, "socket");
	_listen = (void *) GetProcAddress (ws2_32, "listen");
	_htons = (void *) GetProcAddress (ws2_32, "htons");
	_htonl = (void *) GetProcAddress (ws2_32, "htonl");
	_connect = (void *) GetProcAddress (ws2_32, "connect");
	_WSAStartup = (void *) GetProcAddress (ws2_32, "WSAStartup");
	_WSACleanup = (void *) GetProcAddress (ws2_32, "WSACleanup");
	_closesocket = (void *) GetProcAddress (ws2_32, "closesocket");
	_inet_ntoa = (void *) GetProcAddress (ws2_32, "inet_ntoa");
	_inet_addr = (void *) GetProcAddress (ws2_32, "inet_addr");
	#endif

    return (WSAStartup (MAKEWORD(2, 0), &wsaData) == 0);
}


EasySocketListened *
es_accept(EasySocket *server, int buffer_size_allocated)
{
    SOCKET sock;
    SOCKADDR_IN csin;
    int csin_size = sizeof(csin);
    EasySocketListened *esl = NULL;

    sock = accept(server->sock, (SOCKADDR *) &csin, &csin_size);

    if (sock == INVALID_SOCKET)
        return NULL;

    if ((esl = malloc(sizeof(EasySocketListened))) == NULL)
        return NULL;

    esl->sock         = sock;
    esl->is_connected = 1;
    esl->buffer       = str_malloc_clear(buffer_size_allocated);
    esl->bsize        = buffer_size_allocated;
    esl->_data        = NULL;

    return esl;
}

char *
es_get_ip_from_hostname (char *addr)
{
    struct hostent *h;

	if ((h = gethostbyname(addr)) == NULL)
        return NULL;

	return inet_ntoa(*((struct in_addr *)h->h_addr));
}

void
es_set_connected(EasySocket *es, int is_connected)
{
    es->is_connected = is_connected;
}

void
es_listener (EasySocketListened *esl, void (*recv_callback)(EasySocketListened *sock), void (*finish_callback)(EasySocketListened *sock))
{
    EasySocketListenerArgs *esla = esla_new(esl, recv_callback, finish_callback);

    esl->thread = CreateThread(NULL, 0, (void*)_ex_es_listener, esla, 0, NULL);
}

void
es_send(EasySocket *es, char *msg, int len)
{
    if (len == -1)
        len = strlen(msg);

    send(es->sock, msg, len, 0);
}

char *
es_get_http_file (EasySocket *es, char *path)
{
	char *full_msg = str_dup_printf(
		"GET %s HTTP/1.1\r\n"
		"Host: lolcamera.alwaysdata.net\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:24.0) Gecko/20100101 Firefox/24.0\r\n\r\n\r\n",
		path
    );

    es_send(es, full_msg, -1);
    free(full_msg);

	char buffer[1024 * 100];
    es_recv(es, buffer, sizeof(buffer));
    int bytes = es_recv(es, buffer, sizeof(buffer));

	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		return strdup(buffer);
	}

	return NULL;
}

void
es_send_http_request(EasySocket *es, char *msg)
{
    char *full_msg = str_dup_printf(
        "HTTP/1.1 200 OK\r\n"
        "Date: Mon, 23 May 2005 22:38:34 GMT\r\n"
        "Server: Apache/1.3.3.7 (Unix) (Red-Hat/Linux)\r\n"
        "Last-Modified: Wed, 31 Nov 3373 31:33:73 GMT\r\n"
        "Etag: \"3f80f-1b6-3e1cb03b\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n\r\n%s",
        strlen(msg) + 1,
		msg
    );

    es_send(es, full_msg, -1);

    free(full_msg);
}

int
es_recv(EasySocket *es, char *buffer, int len)
{
    return recv(es->sock, buffer, len, 0);
}

int
es_close(EasySocket *es)
{
    return (closesocket(es->sock) != SOCKET_ERROR);
}

void
es_end()
{
    WSACleanup();
}

/**
 *  @Destructors
 */

void
es_free (EasySocket *p)
{
	if (p != NULL)
	{
		free(p);
	}
}

void
es_listener_free (EasySocketListened *esl, void (*free_data_func)())
{
    if (esl != NULL)
    {
        free(esl->buffer);

        if (esl->_data != NULL && free_data_func != NULL)
            free_data_func(esl->_data);

        closesocket(esl->sock);

        free(esl);
    }
}
