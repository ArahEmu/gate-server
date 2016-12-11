#include "authserver.h"

AuthServer::AuthServer()
{

}

bool AuthServer::Startup(int port)
{
    return true;
}

void AuthServer::Update()
{
    ServerSocket runningSocket;
    runningSocket.Configure(6112);
    ClientConnection baseClient = runningSocket.Accept();

}
