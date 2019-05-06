#include "authserver.h"

AuthServer::AuthServer() : m_ClientsLock(), m_ServerPort(0), m_loginThread(), m_ThreadPool(), m_Clients(), m_Running(false)
{

}

bool AuthServer::Startup(uint16_t ServerPort, int ThreadCount)
{
    m_Running = true;
    m_ServerPort = ServerPort;
    m_loginThread = std::thread(AcceptThread, this);
    m_ThreadPool.reserve(ThreadCount);

    for (auto i = 0; i < ThreadCount; i++) {
        //m_ThreadPool.at(i) = std::thread(ClientWorkerThread, this);
        m_ThreadPool.push_back(std::make_unique<AuthClientPool>(i));
    }

    for (auto i = 0; i < m_ThreadPool.size(); i++) {
        m_ThreadPool.at(i)->Start();
    }

    return true;
}

void AuthServer::Update()
{
    m_ClientsLock.lock();

    /*
     * The main thread simply looks for new clients placed in the array by the accept thread
     * Then moves them into the appropriate thread pool
     */

    for (unsigned int i = 0; i < m_Clients.size(); i++) {
        auto client = std::move(m_Clients.at(i));
        m_Clients.erase(m_Clients.begin()+i);

        int clientCount = 999999, threadId = -1;

        for (auto i = 0; i < m_ThreadPool.size(); i++) {
            auto currentThreadClientCount = m_ThreadPool.at(i)->Count();
            if (currentThreadClientCount < clientCount) {
                clientCount = currentThreadClientCount;
                threadId = i;
            }
        }

        if (threadId != -1) {
            m_ThreadPool.at(threadId)->AddClient(std::move(client));
            printf("Sending Client To Thread Pool (id: %d)\n", threadId);
        }
    }

    m_ClientsLock.unlock();
}

void AuthServer::AcceptThread(AuthServer* Instance)
{
    ServerSocket runningSocket;
    if (runningSocket.Configure(Instance->m_ServerPort) == false) {
        printf("Unable to establish server socket, there is an issue with the server.\n");
        return;
    }

    while (Instance->m_Running) {
        /*
         * Listen For New Connections.
         */
        ClientConnection baseClient = runningSocket.Accept();
        //AuthClient* client = new AuthClient(baseClient);

        /*
         * Lock client array and place the client in the queue
         */
        Instance->m_ClientsLock.lock();
        Instance->m_Clients.push_back(std::make_unique<AuthClient>(AuthClient(baseClient)));
        Instance->m_ClientsLock.unlock();
    }

    runningSocket.Flush();
}
