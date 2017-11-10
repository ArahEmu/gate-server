#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include "networking.h"

#include <stdio.h>
#include <cerrno>
#include <chrono>
#include <ctime>

enum RecieveError {
    RE_UNK = -1,
    RE_NOTHING_THERE = -2,
    RE_CLIENT_CLOSED = -3
};

class ClientConnection
{
public:
    ClientConnection();
    virtual ~ClientConnection();
public:
    void Send(const char* Data, unsigned int DataSize);
    int Poll(char* Buffer, unsigned int BufferSize);
    void CheckIdle();
public:
    virtual void Close();
    virtual bool IsConnected();
public:
    char m_ClientIP[INET_ADDRSTRLEN];
    int m_ClientSocket;
    int m_ClientPriority;
public:
    bool m_ClientDisconnectRequest;
    std::clock_t m_LastResponseTime;
};

#endif // CLIENTCONNECTION_H
