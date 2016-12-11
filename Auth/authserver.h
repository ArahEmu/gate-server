#ifndef AUTHSERVER_H
#define AUTHSERVER_H

#include <stdio.h>

#include "Networking/serversocket.h"

class AuthServer
{
public:
    AuthServer();
public:
    bool Startup(int port);
public:
    void Update();
};

#endif // AUTHSERVER_H
