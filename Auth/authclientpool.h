#ifndef AUTHCLIENTPOOL_H
#define AUTHCLIENTPOOL_H

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>

#include "authclient.h"
#include "Util/serverclock.h"

class AuthClientPool
{
public:
    AuthClientPool(int threadId);
    AuthClientPool(AuthClientPool& other);
    ~AuthClientPool();
public:
    void                                        Start();
    void                                        Stop();
    void                                        AddClient(std::unique_ptr<AuthClient> client);
    int                                         Count();
private:
    static void                                 ThreadProcess(AuthClientPool* instance);
private:
    bool                                        m_Running;
    int                                         m_ThreadId;
    std::mutex                                  m_ClientsLock;
    std::thread                                 m_PoolThread;
    std::vector<std::unique_ptr<AuthClient>>    m_Clients;
    ServerClock                                 m_Clock;
};

#endif // AUTHCLIENTPOOL_H
