#include "proxyserver.h"

ProxyServer::ProxyServer() : m_Running(false), m_Port(0), m_SSL(), m_loginThread(), m_ClientsLock()//, m_Clients()
{

}

bool ProxyServer::Startup(uint16_t ServerPort, const char* Certificate, const char* PrivateKey)
{
    this->m_Port = ServerPort;
    this->m_loginThread = std::thread(Run, this);

    if (!m_SSL.Init(Certificate, PrivateKey)) {
        printf("Unable to init SSL, most likely an issue with reading the Certificate or Private Key\n.");
        return false;
    }

    return true;
}

void ProxyServer::Update()
{

}

void ProxyServer::Shutdown()
{

}

void ProxyServer::Run(ProxyServer *Instance)
{
    ServerSocket runningSocket;
    runningSocket.Configure(Instance->m_Port);

    while (Instance->m_Running) {
        /*
         * Listen For New Connections.
         */
        ClientConnection baseClient = runningSocket.Accept();
        //LoginClient* client = new LoginClient(baseClient);

        /*
         * Lock client array and place the client in the queue
         */
        Instance->m_ClientsLock.lock();
        //Instance->m_Clients.push_back(client);
        Instance->m_ClientsLock.unlock();

    }

    runningSocket.Flush();
}
