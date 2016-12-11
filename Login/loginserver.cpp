#include "loginserver.h"

LoginServer::LoginServer() : m_Running(false), m_MitmMode(false), m_ServerPort(0),
    m_loginThread(), m_ClientsLock(), m_Clients(), m_SSL(), m_Gateways()
{

}

bool LoginServer::Startup(uint16_t ServerPort, const char* Certificate, const char* PrivateKey, bool MitmMode)
{
    m_Running = true;
    m_loginThread = std::thread(Run, this);
    m_ServerPort = ServerPort;
    m_MitmMode = MitmMode;

    if (!m_SSL.Init(Certificate, PrivateKey)) {
        printf("Unable to init SSL, most likely an issue with reading the Certificate or Private Key\n.");
        return false;
    }

    std::function<void(mysqlpp::StoreQueryResult::const_iterator)> funct = [this] (mysqlpp::StoreQueryResult::const_iterator itt) {
            this->RegisterGateway(std::string((*itt)["gateRegion"]), std::string((*itt)["gateAddress"]));
        };

    Database::Get().ItterateQuery("SELECT * FROM cligate.Gates;", funct);
    printf("Login server discovered %d gateway/s\n", m_Gateways.size());

    return true;
}

void LoginServer::Update()
{
    m_ClientsLock.lock();

    /*
     * This is our heavy weight champion
     * we itterate over any connected players and perform network io with
     * the clients here.
     */

    for (unsigned int i = 0; i < m_Clients.size(); i++) {
        LoginClient* client = m_Clients.at(i);

        assert(client);

        // TODO: Move this to a better place
        if (GW2BlackList::CheckBlacklist(client->m_ClientIP)) {
            printf("Client forcefully disconnected as they were on the ban list.\n");
            client->Close();
        }

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

void LoginServer::Shutdown()
{
    m_Running = false;
    m_loginThread.join();
    for (unsigned int i = 0; i < m_Clients.size(); i++) {
        delete m_Clients.at(i);
    }
}

void LoginServer::Run(LoginServer *Instance)
{
    ServerSocket runningSocket;
    runningSocket.Configure(Instance->m_ServerPort);

    while (Instance->m_Running) {
        /*
         * Listen For New Connections.
         */
        ClientConnection baseClient = runningSocket.Accept();
        LoginClient* client = new LoginClient(baseClient);

        /*
         * Lock client array and place the client in the queue
         */
        Instance->m_ClientsLock.lock();
        Instance->m_Clients.push_back(client);
        Instance->m_ClientsLock.unlock();

    }

    runningSocket.Flush();
}

void LoginServer::RegisterGateway(std::string region, std::string address)
{
    m_Gateways[region] = address;
}

ServerSSL *LoginServer::SSL()
{
    return &m_SSL;
}

bool LoginServer::Mitm()
{
    return m_MitmMode;
}

void LoginServer::DisableMitm()
{
    m_MitmMode = false;
}
