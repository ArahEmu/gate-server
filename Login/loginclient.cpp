#include "loginclient.h"
#include "loginserver.h"

/**************/

struct UNK_PACKET_01 {
    uint16_t unk01;
    uint16_t size;
    char* unkdata;
};

struct ClientVersionPacket {
    uint16_t packetCode;
    uint16_t packetLength;
    uint32_t ClientVersion;
    uint32_t ClientProgram;
    uint32_t UNK_01;
};

struct ClientEncryptionSeedPacket {
    uint16_t packetCode;
    char CypherSeed[64];
};

struct ClientIdentity {
    uint16_t packetCode;
    wchar_t ClientUsername[1024];
    wchar_t ClientHostName[1024];
};

/**************/


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
                System::DebugWriteMessage("Plain_Trace.dat", "--Send--\n\n", strlen("--Send--\n\n"));
                System::DebugWriteMessage("Plain_Trace.dat", incommingBuffer, readLength);
                System::DebugWriteMessage("Plain_Trace.dat", "\n\n--Send END--\n\n", strlen("\n\n--Send END--\n\n"));
            }

            // Then clear out.
            m_Packet.Clear();
        }

        //
        else if (readLength > 0) {
            uint16_t packetCode = ((uint16_t*)incommingBuffer)[0];

            switch(packetCode) {
            case 1024: { // Client Hello / Version Information
                ClientVersionPacket incommingVersionPacket;
                memset(&incommingVersionPacket, 0, sizeof(ClientVersionPacket));
                memcpy(&incommingVersionPacket, incommingBuffer, sizeof(ClientVersionPacket));
                printf("Incomming Game Connection, Client Version %d\n", incommingVersionPacket.ClientVersion);
                break;
            }
            case 16896: { // Cypher Seed packet
                ClientEncryptionSeedPacket incommingCypherPacket;
                memcpy(&incommingCypherPacket, incommingBuffer, sizeof(ClientEncryptionSeedPacket));
                printf("Client Cypher Seed Recieved.\n");

                char response[22]= { /* Packet 32 */
                                     0x01, 0x16, 0xae, 0x93, 0xc5, 0x3e, 0x74, 0x64,
                                     0x05, 0x3c, 0x1d, 0xbc, 0xc6, 0xdb, 0x3a, 0xe2,
                                     0x01, 0x0a, 0xa9, 0xf7, 0xda, 0xeb };

                Send(response, sizeof(response));
                printf("Sent Response.\n");
                break;
            }
            case 2: { // Client Host name (computer name)
                ClientIdentity incommingClientIdentity;
                memset(&incommingClientIdentity, 0, sizeof(ClientIdentity));
                incommingClientIdentity.packetCode = packetCode;
                auto usernameLength = wcslen((wchar_t*)(incommingBuffer+2));
                auto computerNameAddress = (wchar_t*)(incommingBuffer+2+(usernameLength*2)+2);
                auto computerNameLength = wcslen((wchar_t*)(incommingBuffer+2+(usernameLength*2)+2));
#ifdef _WIN32
                wcsncpy(incommingClientIdentity.ClientUsername, (wchar_t*)(incommingBuffer+2), usernameLength );
                wcsncpy_s(incommingClientIdentity.ClientHostName, (wchar_t*)(incommingBuffer+2+(usernameLength*2)+2), computerNameLength );
#endif
                setlocale(LC_ALL, "");
                printf("Computer Username: %ls , Computer Name: %ls\n", incommingClientIdentity.ClientUsername, incommingClientIdentity.ClientHostName);
                char poisionResponse[22]= { /* Packet 32 */
                                     0x69, 0x63, 0x69, 0x63, 0x69, 0x63, 0x69, 0x63,
                                     0x69, 0x63, 0x69, 0x63, 0x69, 0x63, 0x69, 0x63,
                                     0x69, 0x63, 0x69, 0x63, 0x69, 0x63 };

                Send(poisionResponse, sizeof(poisionResponse));
                break;
            }
            default: { // Unknown Packet
                 printf("Incomming Unknown Packet Code %d\n", packetCode);
                 char padding[] = {0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69};
                 System::DebugWriteMessage("UNK_PACKETS.dat", incommingBuffer, readLength);
                 System::DebugWriteMessage("UNK_PACKETS.dat", padding, sizeof(padding));
            }
            }



            /*int validPacketIndex = -1;
            for (auto i = 0; i < readLength; i++) {
                if (incommingBuffer[i] != 0) {
                    validPacketIndex = i;
                    break;
                }
            }

            if (validPacketIndex > 0) {
                char* packetBuffer = new char[(readLength-validPacketIndex)+1];
                memset(packetBuffer, 0, (readLength-validPacketIndex)+1);
                memcpy(packetBuffer, incommingBuffer+validPacketIndex, readLength-validPacketIndex);

                if (m_Packet.Parse(packetBuffer, readLength) && m_Packet.Validate()) {
                    // Read in and spit out.
                    if (m_Session.Recieve(&m_Packet)) {
                    }

                    // Then clear out.
                    m_Packet.Clear();
                }
            }*/
        } else {

        }
    }

    // Otherwise its probably a game / ssl packet
    else {

        auto loginState = m_Login.Recieve(ServerInstance->SSL());

        // MITM Stuff...
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
        } else {

        }

    } // m_TSLReady

    CheckIdle();
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
