#ifndef NETWORKING
#define NETWORKING

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#define _WINSOCKAPI_

#include "defines.h"

#ifdef SOCK_VER_WIN
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <io.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include <sys/types.h>

#ifndef _WIN32
#   include <unistd.h>
#endif

#include <signal.h>
#include <assert.h>
#include <fcntl.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#endif // NETWORKING
