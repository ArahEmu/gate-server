#include "clientconnection.h"

ClientConnection::ClientConnection() : m_ClientIP(), m_ClientSocket(0), m_ClientPriority(0),
    m_ClientDisconnectRequest(false), m_LastResponseTime(std::clock()) {

}

ClientConnection::~ClientConnection()
{

}

void ClientConnection::Send(const char *Data, unsigned int DataSize) {
    if (m_ClientSocket < 0) {
        return;
    }

    unsigned int totalSent = 0;
    unsigned int toSend = DataSize;

#ifdef _WIN32
    u_long iMode = 1;
    ioctlsocket(m_ClientSocket, FIONBIO, &iMode);
#else
    fcntl(m_ClientSocket, F_SETFL, O_NONBLOCK);
#endif

    while (totalSent != DataSize) {
        //setsockopt(m_ClientSocket, SOL_SOCKET, SO_SNDBUF,  &DataSize, sizeof(unsigned int));
        int sent = send(m_ClientSocket, Data, DataSize, 0);
        totalSent += sent;
        toSend -= toSend;
        if (sent < 0) {
            printf("Close Called: ClientConnection::Send\n");
            Close();
            break;
        }
    }

}

int ClientConnection::Poll(char *Buffer, unsigned int BufferSize) {
    int pollResult = recv(m_ClientSocket, Buffer, BufferSize-1, 0);
    if (pollResult == -1 && errno == EWOULDBLOCK) {
        return RE_NOTHING_THERE;
    } else if (pollResult == -1) {
        return RE_UNK;
    } else if (pollResult == 0) {
        return RE_CLIENT_CLOSED;
    }
    m_LastResponseTime = std::clock();
    return pollResult;
}

void ClientConnection::CheckIdle()
{
    auto checkTime = std::clock();
    auto lastResponseDuration = 1000.0 * (checkTime-m_LastResponseTime) / CLOCKS_PER_SEC;
    auto seconds = lastResponseDuration / 1000;
    if (seconds>25) {
        printf("Client Timout Reached Of %d Seconds For %s\n", 15, this->m_ClientIP);
        this->Close();
    }
}

void ClientConnection::Close() {
    m_ClientSocket = shutdown(m_ClientSocket, SD_BOTH);
    m_ClientSocket = -1;
}

bool ClientConnection::IsConnected() {
    if (m_ClientSocket < 0) {
        return false;
    }
    return true;
}
