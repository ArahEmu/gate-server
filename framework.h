#ifndef FRAMEWORK_H
#define FRAMEWORK_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <time.h>

//#include "mongo/client/dbclient.h"

#include "Util/serverconfig.h"
#include "Util/serverclock.h"
#include "Util/system.h"
#include "Util/database.h"
#include "Login/loginserver.h"
#include "Auth/authserver.h"
#include "GW2/gw2blacklist.h"
#include "Mitm/proxyserver.h"

#define REQ_ARGUMENTS_C 1 // Require just the filename to the config file

/*
 * Store any options that are frequently accessed
 * */
struct FrameworkOptions {
    char            m_ServerName[512];
    char            m_ServerMode[512];
    bool            m_MITMMode;
    unsigned int    m_ServerRate;
};

/*
 * Holds startup logic and the main application loop
 * */
class Framework : public ServerConfig
{
public:
    Framework();
public:
    int                 Run(int argc, char** argv);
    int                 RunGateway();
    int                 RunAuth();
    int                 RunProxy();
public:
    bool                Configure(int argc, char** argv);
public:
    void                ShowUsage();
private:
    bool                m_Running;
    FrameworkOptions    m_Options;
    LoginServer         m_LoginServer;
    AuthServer          m_AuthServer;
    ProxyServer         m_ProxyServer;

private:
    ServerClock         m_Clock;
};

#endif // FRAMEWORK_H
