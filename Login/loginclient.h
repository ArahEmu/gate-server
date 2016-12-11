#ifndef LOGINCLIENT_H
#define LOGINCLIENT_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include <string.h>
#include <stdio.h>

#include "../Networking/clientconnection.h"
#include "../Networking/xmlpacket.h"
#include "../GW2/gw2common.h"
#include "../Util/system.h"
#include "../Util/database.h"

#include "mitmsocket.h"
#include "securelogin.h"
#include "loginsession.h"

class LoginServer;

class LoginClient : public ClientConnection
{
public:
    LoginClient();
    LoginClient(ClientConnection& Base);
    ~LoginClient();

public:
    void Tick(LoginServer* ServerInstance);

private:
    XMLPacket m_Packet;
    SecureLogin m_Login;
    LoginSession m_Session;

    // ClientConnection interface
public:
    void Close() override;
    bool IsConnected() override;

private:
    MitmSocket m_MitmSocket;
};

#endif // LOGINCLIENT_H
