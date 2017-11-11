#include "gw2forwardingclient.h"

#include <Mitm/proxyserver.h>

GW2ForwardingClient::GW2ForwardingClient()
{

}

GW2ForwardingClient::GW2ForwardingClient(ClientConnection &Base)
{
    this->m_ClientSocket = Base.m_ClientSocket;
    this->m_ClientPriority = Base.m_ClientPriority;
    memcpy(this->m_ClientIP, Base.m_ClientIP, INET_ADDRSTRLEN);
}

GW2ForwardingClient::~GW2ForwardingClient()
{

}

void GW2ForwardingClient::Tick(ProxyServer *ServerInstance)
{
    //UNREFERENCED_PARAMETER(ServerInstance);

    // Read any incomming packets from client.
    char incommingBuffer[2048];
    memset(incommingBuffer, 0x69, 2048);
    int readLength = this->Poll(incommingBuffer, 2048);

    if (readLength > 0) {
        printf("%.*s", readLength, incommingBuffer);
    }
}

void GW2ForwardingClient::Close()
{
    ClientConnection::Close();
}

bool GW2ForwardingClient::IsConnected()
{
    return true;
}
