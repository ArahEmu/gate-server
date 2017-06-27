#include "proxyserver.h"

ProxyServer::ProxyServer() : m_Running(false), m_Port(0), m_SSL(), m_loginThread(), m_ClientsLock()//, m_Clients()
{

}

bool ProxyServer::Startup(uint16_t ServerPort, const char* Certificate, const char* PrivateKey)
{
    this->m_Port = ServerPort;
    this->m_loginThread = std::thread(Run, this);
    this->m_Running = true;

    if (!m_SSL.Init(Certificate, PrivateKey)) {
        printf("Unable to init SSL, most likely an issue with reading the Certificate or Private Key\n.");
        return false;
    }

    return true;
}

void ProxyServer::Update()
{
    m_ClientsLock.lock();

    /*
     * This is our heavy weight champion
     * we itterate over any connected players and perform network io with
     * the clients here.
     */

    for (unsigned int i = 0; i < m_Clients.size(); i++) {
        GW2ForwardingClient* client = m_Clients.at(i);

        assert(client);

        // Only interact with the client if its connected duh
        if (client->IsConnected()) {
            client->Tick(this);
        } else {
            m_Clients.erase(m_Clients.begin()+i);
            printf("Client Disconnected: %s\n", client->m_ClientIP);
            printf("Connected Clients: %lu\n", m_Clients.size());
        }

    }

    m_ClientsLock.unlock();
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
        GW2ForwardingClient* client = new GW2ForwardingClient(baseClient);

        /*
         * Lock client array and place the client in the queue
         */
        Instance->m_ClientsLock.lock();
        Instance->m_Clients.push_back(client);
        Instance->m_ClientsLock.unlock();

    }

    runningSocket.Flush();
}
