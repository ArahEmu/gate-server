#include "serversocket.h"

addrinfo g_HintsInstance = {
    AI_PASSIVE,
    PF_INET,
    SOCK_STREAM,
    IPPROTO_TCP,
    0, 0, 0, 0
};

ServerSocket::ServerSocket() : m_SocketHandle(-1), m_IntfConfig()
{
    memset(m_Port, 0, 6);
}

bool ServerSocket::Configure(uint16_t ServerPort)
{
    System::IgnoreSigPipe();

#ifdef _WIN32 // Initalize shit sock 2
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif

    addrinfo *serverInfo;
    sprintf(m_Port, "%d", ServerPort);

    int result = getaddrinfo(NULL, m_Port, &g_HintsInstance, &serverInfo);

    if (result != 0) {
        printf("!) getaddrinfo error: %s\n", gai_strerror(result));
        return false;
    }

    m_IntfConfig = *serverInfo;

    // Need to check what the fuck i was doing this here for
    //freeaddrinfo(serverInfo);

    m_SocketHandle = (int)socket(m_IntfConfig.ai_family, m_IntfConfig.ai_socktype, m_IntfConfig.ai_protocol);

    if (m_SocketHandle == -1) {
        printf("!) socket error: \n");
        return false;
    }

    int yes = 1;
#ifdef _WIN32
    if (setsockopt(m_SocketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
#else
    if (setsockopt(m_SocketHandle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
#endif

        printf("!) setsockopt error: \n");
        return false;
    }

    result = bind(m_SocketHandle, m_IntfConfig.ai_addr, m_IntfConfig.ai_addrlen);
	
    if (result == -1) {
#ifdef _WIN32
		int last_error = WSAGetLastError();
        printf("!) bind error: %d\n", last_error);
#endif
        return false;
    }

    result = listen(m_SocketHandle, 20);

    if (result == -1) {
        printf("!) listen error: \n");
        return false;
    }

    sockaddr_in socketInformation;
    memset(&socketInformation, 0, sizeof(socketInformation));
    socklen_t len = sizeof(sockaddr);
    if (getsockname(m_SocketHandle, (struct sockaddr *) &socketInformation, &len) != 0) {
        printf("!) getsockname error: \n");
    }

    if (ntohs(socketInformation.sin_port) != ServerPort) {
        printf("Application port does not match requested port, this is a bug. Restart the server.\n");
        return false;
    }

    return true;
}

void ServerSocket::Flush() {

}

ClientConnection ServerSocket::Accept() {
    ClientConnection    newClient;
    sockaddr_storage    incomming_address;
    socklen_t           addr_size = sizeof(incomming_address);

    newClient.m_ClientSocket = accept(m_SocketHandle, (struct sockaddr *)&incomming_address, &addr_size);

    // Do not use blocking calls
#ifdef _WIN32

	if (newClient.m_ClientSocket == INVALID_SOCKET) {
		int winsockError = WSAGetLastError();
		throw new std::exception();
	}

    u_long iMode = 1;
    ioctlsocket(newClient.m_ClientSocket, FIONBIO, &iMode);

	

#else
    fcntl(newClient.m_ClientSocket, F_SETFL, O_NONBLOCK);
#endif


    sockaddr connectingClient;
    socklen_t len = sizeof(connectingClient);
    getpeername(newClient.m_ClientSocket, &connectingClient, &len);

    PrintIP(&connectingClient, "Client Connected: %s\n");
    WriteIP(&connectingClient, newClient.m_ClientIP);

    return newClient;
}

void ServerSocket::PrintIP(const sockaddr *SocketAddress, const char* Message)
{
    char connectingIPString[INET_ADDRSTRLEN];
    switch(SocketAddress->sa_family) {
        case AF_INET:
#ifdef _WIN32
        InetNtopA(AF_INET, (PVOID*)&(((struct sockaddr_in *)SocketAddress)->sin_addr),
                connectingIPString, INET_ADDRSTRLEN);
#else
        inet_ntop(AF_INET, &(((struct sockaddr_in *)SocketAddress)->sin_addr),
                connectingIPString, INET_ADDRSTRLEN);
#endif
            break;

        case AF_INET6:
#ifdef _WIN32
		InetNtopA(AF_INET, (PVOID*)&(((struct sockaddr_in6 *)SocketAddress)->sin6_addr),
                connectingIPString, INET_ADDRSTRLEN);
#else
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)SocketAddress)->sin6_addr),
                    connectingIPString, INET_ADDRSTRLEN);
#endif
            break;

        default:
            strncpy(connectingIPString, "Unknown AF", INET_ADDRSTRLEN);
    }

    printf(Message, connectingIPString);
}

void ServerSocket::WriteIP(const sockaddr *SocketAddress, char *Dst)
{
	switch (SocketAddress->sa_family) {
	case AF_INET:
#ifdef _WIN32
		InetNtopA(AF_INET, (PVOID*)&(((struct sockaddr_in *)SocketAddress)->sin_addr),
			Dst, INET_ADDRSTRLEN);
#else
		inet_ntop(AF_INET, &(((struct sockaddr_in *)SocketAddress)->sin_addr),
			Dst, INET_ADDRSTRLEN);
#endif
		break;

	case AF_INET6:
#ifdef _WIN32
		InetNtopA(AF_INET, (PVOID*)&(((struct sockaddr_in6 *)SocketAddress)->sin6_addr),
			Dst, INET_ADDRSTRLEN);
#else
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)SocketAddress)->sin6_addr),
			Dst, INET_ADDRSTRLEN);
#endif
		break;

	default:
		strncpy(Dst, "Unknown AF", INET_ADDRSTRLEN);
	}
}

