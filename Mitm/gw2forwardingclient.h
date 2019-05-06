#ifndef GW2FORWARDINGCLIENT_H
#define GW2FORWARDINGCLIENT_H

#include <memory.h>

#include <Networking/clientconnection.h>

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

class ProxyServer;

class GW2ForwardingClient : public ClientConnection
{
public:
    GW2ForwardingClient();
    GW2ForwardingClient(ClientConnection& Base);
    ~GW2ForwardingClient();

public:
    void Tick(ProxyServer* ServerInstance);

    // ClientConnection interface
public:
    void Close() override;
    bool IsConnected() override;
};

#endif // GW2FORWARDINGCLIENT_H
