#ifndef AUTHCLIENT_HPP
#define AUTHCLIENT_HPP

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include "../Networking/clientconnection.h"
#include "../GW2/gw2authpacket.h"

class AuthClient : public ClientConnection {

public:
    AuthClient();
    AuthClient(ClientConnection& Base);
    ~AuthClient();

public:
    void Update();
};

#endif // AUTHCLIENT_HPP
