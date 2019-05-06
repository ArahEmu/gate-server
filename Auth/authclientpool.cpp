#include "authclientpool.h"

AuthClientPool::AuthClientPool(int threadId) : m_Running(false), m_ThreadId(threadId), m_ClientsLock(), m_PoolThread(), m_Clients(), m_Clock(5000)
{
    printf("AuthClientPool()\n");
}

AuthClientPool::AuthClientPool(AuthClientPool &other) : m_Clock(5000)
{
    printf("AuthClientPool(AuthClientPool &other)\n");
}

AuthClientPool::~AuthClientPool()
{
    printf("~AuthClientPool()\n");
}

void AuthClientPool::Start()
{
    m_Running = true;
    m_PoolThread = std::thread(ThreadProcess, this);
}

void AuthClientPool::Stop()
{
    m_Running = false;
}

void AuthClientPool::AddClient(std::unique_ptr<AuthClient> client)
{
    m_ClientsLock.lock();
    m_Clients.push_back(std::move(client));
    m_ClientsLock.unlock();
}

int AuthClientPool::Count()
{
    return m_Clients.size();
}

void AuthClientPool::ThreadProcess(AuthClientPool *instance)
{
    printf("Starting client pool (id: %d)\n", instance->m_ThreadId);

    instance->m_Clock.Start();

    while(instance->m_Running) {
        if (instance->m_Clock.InFrame()) {
            instance->m_ClientsLock.lock();
            for(auto& client : instance->m_Clients) {
                client->Update();
            }
            instance->m_ClientsLock.unlock();

            instance->m_Clock.Frame();

        } else {
#ifdef _WIN32
            ::Sleep(200);
#else
            ::usleep(200);
#endif
        }
    }
    printf("Exiting client pool (id: %d)\n", instance->m_ThreadId);
}
