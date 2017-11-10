#ifndef LOGINSESSION_H
#define LOGINSESSION_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include "defines.h"

#include "securelogin.h"
#include "GW2/gw2packet.h"
#include "Networking/xmlpacket.h"
#include "Networking/clientconnection.h"
#include "Util/database.h"
#include "3rdParty/sha256.h"

#include <openssl/bio.h>
#include <openssl/evp.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <uuid/uuid.h>
#endif
#include <string>

struct SessionSendPacket {
    char m_TLSSendBuffer[4096];
    int m_TLSSendBufferLength;
    bool m_TLSSendNeeded;
};

class LoginSession
{
public:
    LoginSession(ClientConnection* Client);
public:
    bool LogoutRequested() {return m_LogoutRequested;}
public:
    bool Recieve(XMLPacket* Packet);
    void Send(SecureLogin* tlsClient);
private:
    void Init(XMLPacket* Packet);
    void StartTLS(XMLPacket* Packet);
    void GetHostname(XMLPacket* Packet);
    void StartSsoLogin(XMLPacket* Packet);
    void ListGameAccounts(XMLPacket* Packet);
    void RequestGameToken(XMLPacket* Packet);
    void Logout(XMLPacket *Packet);
private:
    std::string CreateGuid();
    void SaveResumeToken(const char* userGuid, const char* resumeToken);
    bool CheckResumeToken(const char *userGuid, const char *resumeToken);
    void QueueLoginErrorMessage(int sequence);
private:
    ClientConnection* m_Client;
private:
    int m_ConnectionType;
    int m_Program;
    int m_Build;
    int m_Process;
    bool m_LogoutRequested;
    std::string m_ClientEmail;

private:
    /*char m_TLSSendBuffer[4096];
    int m_TLSSendBufferLength;
    bool m_TLSSendNeeded;*/
    std::vector<SessionSendPacket> m_SendPackets;
public:
    bool m_TSLReady;
};

#endif // LOGINSESSION_H
