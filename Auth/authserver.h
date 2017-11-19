#ifndef AUTHSERVER_H
#define AUTHSERVER_H

#include <vector>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <memory>

#include "Networking/serversocket.h"
#include "authclient.h"
#include "authclientpool.h"

typedef std::unique_ptr<AuthClient> Client;
typedef std::unique_ptr<AuthClientPool> ClientPool;

class AuthServer
{
public:
    AuthServer();
public:
    bool                        Startup(uint16_t ServerPort, int ThreadCount);
public:
    void                        Update();
private:
    static void                 AcceptThread(AuthServer* Instance);
private:
    std::mutex                  m_ClientsLock;
    uint16_t                    m_ServerPort;
    std::thread                 m_loginThread;
    std::vector<ClientPool>     m_ThreadPool;
    std::vector<Client>         m_Clients;
    bool                        m_Running;
};

#endif // AUTHSERVER_H
