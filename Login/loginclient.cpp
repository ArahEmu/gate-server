#include "loginclient.h"
#include "loginserver.h"

LoginClient::LoginClient(ClientConnection &Base) : ClientConnection(), m_Packet(), m_Login(this), m_Session(this), m_MitmSocket()
{
    this->m_ClientSocket = Base.m_ClientSocket;
    this->m_ClientPriority = Base.m_ClientPriority;
    memcpy(this->m_ClientIP, Base.m_ClientIP, INET_ADDRSTRLEN);
}

LoginClient::~LoginClient()
{

}

void LoginClient::Tick(LoginServer *ServerInstance)
{
    // Check if the client is on the blacklist


    // Create out MITM socket if needed, if that failes then make sure we do not try to send
    // or recieve on the mitm socket. This could cause havok, need a way to fail gracefully.
    if (ServerInstance->Mitm() && !m_MitmSocket.IsCreated()) {
        if (!m_MitmSocket.Create()) {
            ServerInstance->DisableMitm();
        }
    }

    // Check if there was an XML request.
    if (!m_Session.m_TSLReady) {

        // Read any incomming packets from client.
        char incommingBuffer[2048];
        memset(incommingBuffer, 0x69, 2048);
        int readLength = this->Poll(incommingBuffer, 2048);

        // See if we can read the data as an xml request.
        if (readLength > 0 && m_Packet.Parse(incommingBuffer, readLength) && m_Packet.Validate()) {

            if (ServerInstance->Mitm()) {
                m_MitmSocket.OnServerRecieve(&m_Packet, incommingBuffer, readLength);
            }

            // Read in and spit out.
            if (m_Session.Recieve(&m_Packet)) {
            }

            // Then clear out.
            m_Packet.Clear();
        }
    }

    // Otherwise its probably a game / ssl packet
    else {

        auto loginState = m_Login.Recieve(ServerInstance->SSL());

        if (ServerInstance->Mitm()) {
            if (m_MitmSocket.TLSReady() && m_MitmSocket.TLSEstablished() == false) {
                if (!m_MitmSocket.EstablishTLSSession()) {
                    printf("<MITM> Error estabilshing TLS session with ANet.\n");
                } else {
                    printf("<MITM> TLS uplink to ANet established.\n");
                }
            }
        }


        if (loginState == RT_TLS_LOGIN_PACKET) {

            const char* clientLoginBuffer = m_Login.GetBuffer();
            if (clientLoginBuffer[0] != 0) {
                if (m_Packet.Parse(clientLoginBuffer, 4096) && m_Packet.Validate()) {

                    if (m_Session.Recieve(&m_Packet)) {
                        m_Session.Send(&m_Login);
                    }
                    m_Packet.Clear();
                }
                if (m_MitmSocket.TLSEstablished()) {
                    m_MitmSocket.SendTLSToANet(clientLoginBuffer, strlen(clientLoginBuffer));
                }
            }
        }


    } // m_TSLReady
}

void LoginClient::Close()
{
    //m_Session.Clear();
    ClientConnection::Close();
}

bool LoginClient::IsConnected()
{
    if (!ClientConnection::IsConnected() ||
            m_Login.GetState() == LS_TLS_FAIL_HANDSHAKE ||
            m_Session.LogoutRequested()) {
        Close();
        return false;
    }
    return true;
}
