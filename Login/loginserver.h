#ifndef LOGINSERVER_H
#define LOGINSERVER_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include <vector>
#include <map>
#include <stdint.h>
#include <thread>
#include <mutex>

#include "GW2/gw2blacklist.h"
#include "Util/database.h"
#include "Networking/serversocket.h"

#include "loginclient.h"
#include "serverssl.h"

class LoginServer
{
public:
    LoginServer();

public:
    bool                                Startup(uint16_t ServerPort, const char *Certificate, const char *PrivateKey, bool MitmMode);
    void                                Update();
    void                                Shutdown();

private:
    // Function used to spawn the login socket server, do not call directly from main thread.
    static void                         Run(LoginServer* Instance);

public:
    void                                RegisterGateway(std::string region, std::string address);
    ServerSSL*                          SSL();
    bool                                Mitm();
    void                                DisableMitm();

private:
    bool                                m_Running;
    bool                                m_MitmMode;
    uint16_t                            m_ServerPort;
    std::thread                         m_loginThread;

private:
    std::mutex                          m_ClientsLock;
    std::vector<LoginClient*>           m_Clients;
    std::map<std::string, std::string>  m_Gateways;

private:
    ServerSSL                           m_SSL;
};

#endif // LOGINSERVER_H
