#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include <vector>
#include <map>
#include <stdint.h>
#include <thread>
#include <mutex>

#include "Networking/serversocket.h"

#include "../Login/serverssl.h"

class ProxyServer
{
public:
    ProxyServer();

public:
    bool                        Startup(uint16_t ServerPort, const char* Certificate, const char* PrivateKey);
    void                        Update();
    void                        Shutdown();

private:
    static void                 Run(ProxyServer* Instance);

private:
    bool                        m_Running;
    uint16_t                    m_Port;
    std::thread                 m_loginThread;

private:
    std::mutex                  m_ClientsLock;
    //std::vector<LoginClient*>   m_Clients;

private:
    ServerSSL   m_SSL;
};

#endif // PROXYSERVER_H
