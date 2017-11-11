#include "loginsession.h"

const char* g_STS_Ping              = "/Sts/Ping STS/1.0";
const char* g_STS_Connect           = "/Sts/Connect STS/1.0";
const char* g_AUTH_StartTLS         = "/Auth/StartTls STS/1.0";
const char* g_AUTH_GetHost          = "/Auth/GetHostname STS/1.0";
const char* g_AUTH_StartSsoLogin    = "/Auth/StartSsoLogin STS/1.0";
const char* g_AUTH_ListGameAcc      = "/Account/ListMyGameAccounts STS/1.0";
const char* g_AUTH_Logout           = "/Auth/LogoutMyClient STS/1.0";
const char* g_AUTH_Resume           = "/Auth/Resume STS/1.0";
const char* g_AUTH_RequestGameToken = "/Auth/RequestGameToken STS/1.0";

LoginSession::LoginSession(ClientConnection *Client) : m_Client(Client), m_ConnectionType(0), m_Program(0),
    m_Build(0), m_Process(0), m_SendPackets(), m_TSLReady(false), m_LogoutRequested(false), m_ClientEmail()
{

}

bool LoginSession::Recieve(XMLPacket *Packet)
{
    if (!Packet) {
        return false;
    }

    if (strncmp(Packet->m_Path, g_STS_Ping, sizeof(Packet->m_Path)) == 0) {
        printf("%s -> Ping!\n", m_Client->m_ClientIP);
    }

    else if (strncmp(Packet->m_Path, g_STS_Connect, sizeof(Packet->m_Path)) == 0) {
        Init(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_StartTLS, sizeof(Packet->m_Path)) == 0) {
        StartTLS(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_GetHost, sizeof(Packet->m_Path)) == 0) {
        GetHostname(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_StartSsoLogin, sizeof(Packet->m_Path)) == 0) {
        StartSsoLogin(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_ListGameAcc, sizeof(Packet->m_Path)) == 0) {
        ListGameAccounts(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_Resume, sizeof(Packet->m_Path)) == 0) {
        ResumeAuthentication(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_RequestGameToken, sizeof(Packet->m_Path)) == 0) {
        RequestGameToken(Packet);
    }

    else if (strncmp(Packet->m_Path, g_AUTH_Logout, sizeof(Packet->m_Path)) == 0) {
        Logout(Packet);
    }

    else {
        printf("Unknown command: %s\n", Packet->m_Path);
    }

    return true;
}

void LoginSession::Send(SecureLogin *tlsClient)
{
    for (auto& message : m_SendPackets) {
        if (message.m_TLSSendNeeded) {
            tlsClient->Send(message.m_TLSSendBuffer, message.m_TLSSendBufferLength);
            message.m_TLSSendNeeded = false;
            memset(message.m_TLSSendBuffer, 0, 4096);
        }
    }

    m_SendPackets.clear();
}

void LoginSession::Init(XMLPacket *Packet)
{
    rapidxml::xml_node<>* connect_node = Packet->m_XMLDocument.first_node("Connect");

    m_ConnectionType = atoi(connect_node->first_node("ConnType")->value());
    m_Program = atoi(connect_node->first_node("Program")->value());
    m_Build = atoi(connect_node->first_node("Build")->value());
    m_Process = atoi(connect_node->first_node("Process")->value());

    /*printf("%s -> Client Query { Type: %d , Program: %d, Build: %d, Process: %d }\n", m_Client->m_ClientIP, m_ConnectionType,
           m_Program, m_Build, m_Process);*/

    m_LogoutRequested = true;
}

void LoginSession::StartTLS(XMLPacket *Packet)
{
    char response[512];
    printf("%s -> TLS Session Requested\n", m_Client->m_ClientIP);
    int sequence = Packet->m_Meta[2] - '0';

    // Do not bother with a formal reply, there does not seem to be any variation in this call
    sprintf(response, "STS/1.0 400 Success\r\ns:%dR\r\nl:%d\r\n\r\n<Error server=\"1001\" module=\"4\" line=\"262\"/>\n", sequence, 45);
	
	auto responseSize = strlen(response);
	if (responseSize < 0) {
		responseSize = 0;
	}
    m_Client->Send(response, (unsigned int)strlen(response));
    m_TSLReady = true;
}

void LoginSession::GetHostname(XMLPacket *Packet)
{
    rapidxml::xml_node<>* requestNode = Packet->m_XMLDocument.first_node("Request");
    bool validUser  = false;
    int userId      = -1;
    std::string userRegion;
    std::string userGateway;

    // TODO: hook the below code up to the database, we may
    // want to send them to a diffrent server if say portal = bot?
    auto loginName  = requestNode->first_node("LoginName")->value();
    auto provider   = requestNode->first_node("Provider")->value();

    try {
        auto userQuery = Database::Get().PrepareQuery("SELECT * FROM cligate.Users where userEmail = %0q;");
        auto gatewayQuery = Database::Get().PrepareQuery("SELECT * FROM cligate.Gates WHERE gateRegion = %0q;");

        auto result = userQuery.store(loginName);
        if (result.size() != 0) {
            validUser   = true;
            if (result.size() > 1) {
                printf("Data consistency error, an email address appear's more than once in the users table.");
            }
            userId      = atoi(std::string(result.front()["userId"]).c_str());
            userRegion  = std::string(result.front()["userRegion"]).c_str();
        }

        result = gatewayQuery.store(userRegion);
        if (result.size() != 0) {
            validUser   = true;
            if (result.size() > 1) {
                printf("Data consistency error, more than one gateway exists for a region.");
            }
            userGateway  = std::string(result.front()["gateAddress"]).c_str();
        }

    }
    catch (const mysqlpp::BadQuery& er) {
        std::cerr << "Query error: " << er.what() << std::endl;
        m_LogoutRequested = true;
        return;
    }
    catch (const mysqlpp::BadConversion& er) {
        std::cerr << "Conversion error: " << er.what() << std::endl <<
                "\tretrieved data size: " << er.retrieved <<
                ", actual size: " << er.actual_size << std::endl;
        m_LogoutRequested = true;
        return;
    }
    catch (const mysqlpp::Exception& er) {
        std::cerr << "Error: " << er.what() << std::endl;
        m_LogoutRequested = true;
        return;
    }

    // Debug message.
    printf("User %s is logging in using %s\n", loginName, provider);
    printf("Forwarding %s to %s\n", loginName, userGateway.c_str());
    int sequence = Packet->m_Meta[2] - '0';

    // Form a packet, the only element is the detination host name
    // GW2 will then try to connect to the specified server.
    GW2Packet replyPacket("", sequence, PT_REPLY);
    replyPacket.AddElement("Hostname", userGateway.c_str());
    //replyPacket.AddElement("Hostname", "cligate-fra.101.ncplatform.net.");

    // Signal that there is TLS data to be sent next time round.
    SessionSendPacket packet;
    memset(packet.m_TLSSendBuffer, 0, 4096);
    sprintf(packet.m_TLSSendBuffer, "%s", replyPacket.Payload());
    packet.m_TLSSendBufferLength = (int)strlen(packet.m_TLSSendBuffer);
    packet.m_TLSSendNeeded = true;
    m_SendPackets.push_back(packet);

    m_LogoutRequested = true;
}

void LoginSession::StartSsoLogin(XMLPacket *Packet)
{
    rapidxml::xml_node<>* requestNode = Packet->m_XMLDocument.first_node("Request");

    char password[1024];
    char passwordToken[1024];
    memset(password, 0, 1024);
    memset(passwordToken, 0, 1024);
    int passwordLength = -1;
    int passwordTokenLength = -1;

    char* emailaddress = nullptr;
    char* passwordBase64 = nullptr;
    char* passwordTokenBase64 = nullptr;

    std::string sha256Password;
    std::string guid;
    std::string username;
    std::string decodedToken;

    try {
        emailaddress = requestNode->first_node("LoginName")->value();

        if (requestNode->first_node("PasswordToken") != nullptr) {
            passwordTokenBase64 = requestNode->first_node("PasswordToken")->value();
            auto    bio = BIO_new_mem_buf(passwordTokenBase64, -1);
            auto    b64 = BIO_new(BIO_f_base64());
                    bio = BIO_push(b64, bio);

            BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
            passwordTokenLength = BIO_read(bio, passwordToken, (int)strlen(passwordTokenBase64));
            BIO_free_all(bio);
        }

        else if (requestNode->first_node("Password") != nullptr) {
            passwordBase64 = requestNode->first_node("Password")->value();
            auto    bio = BIO_new_mem_buf(passwordBase64, -1);
            auto    b64 = BIO_new(BIO_f_base64());
                    bio = BIO_push(b64, bio);

            BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
            passwordLength = BIO_read(bio, password, (int)strlen(passwordBase64));
            BIO_free_all(bio);
        }

        else {
            int sequence = Packet->m_Meta[2] - '0';
            QueueLoginErrorMessage(sequence);
            m_LogoutRequested = true;
            printf("Unsupported authentication method.\n");
            return;
        }

        auto userQuery = Database::Get().PrepareQuery("SELECT * FROM cligate.Users where userEmail = %0q;");
        auto result = userQuery.store(emailaddress);
        if (result.size() != 0) {
            if (result.size() > 1) {
                printf("Data consistency error, an email address appear's more than once in the users table.");
            }
            sha256Password  = std::string(result.front()["userPassword"]).c_str();
            guid            = std::string(result.front()["userGuid"]).c_str();
            username        = std::string(result.front()["userName"]).c_str();
        }
    }
    catch(std::exception ex)
    {
        printf("Unexpected authentication error: %s.\n", ex.what());
        int sequence = Packet->m_Meta[2] - '0';
        QueueLoginErrorMessage(sequence);
        m_LogoutRequested = true;
        return;
    }
    catch (const mysqlpp::BadQuery& er) {
        std::cerr << "Query error: " << er.what() << std::endl;
        m_LogoutRequested = true;
        return;
    }
    catch (const mysqlpp::BadConversion& er) {
        std::cerr << "Conversion error: " << er.what() << std::endl <<
                "\tretrieved data size: " << er.retrieved <<
                ", actual size: " << er.actual_size << std::endl;
        m_LogoutRequested = true;
        return;
    }
    catch (const mysqlpp::Exception& er) {
        std::cerr << "Error: " << er.what() << std::endl;
        m_LogoutRequested = true;
        return;
    }

    // Check if there is no password present, if so try using the password token, then try
    // using a password
    if (passwordTokenBase64 != nullptr) {
        if (CheckResumeToken(guid.c_str(), passwordToken)) {
            printf("Client %s using resume token %s\n", guid.c_str(), passwordToken);
        } else {
            printf("Resume token %s failed\n", passwordToken);
            int sequence = Packet->m_Meta[2] - '0';
            QueueLoginErrorMessage(sequence);
            m_LogoutRequested = true;
            return;
        }
    } else if (passwordBase64 != nullptr) {
        eSHA256::SHA256 sha256;
        if (sha256(password).compare(sha256Password) != 0) {
            int sequence = Packet->m_Meta[2] - '0';
            QueueLoginErrorMessage(sequence);
            m_LogoutRequested = true;
            return;
        }
    } else {
        printf("Unexpected authentication error\n");
        int sequence = Packet->m_Meta[2] - '0';
        QueueLoginErrorMessage(sequence);
        m_LogoutRequested = true;
    }

    // Ugly hack to get the sequence number
    int sequence = Packet->m_Meta[2] - '0';

    // Persist the ResumeToken to the DB
    std::string resumetoken = CreateGuid();
    SaveResumeToken(guid.c_str(), resumetoken.c_str());

    GW2Packet replyPacket("", sequence, PT_REPLY);
    replyPacket.AddElement("UserId", guid.c_str());
    replyPacket.AddElement("UserCenter", "5");
    replyPacket.AddElement("UserName", username.c_str());
    replyPacket.AddElement("Parts", "");
    replyPacket.AddElement("ResumeToken", resumetoken.c_str());
    replyPacket.AddElement("EmailVerified", "1");

    SessionSendPacket packet;
    memset(packet.m_TLSSendBuffer, 0, 4096);
    sprintf(packet.m_TLSSendBuffer, replyPacket.Payload());
    packet.m_TLSSendBufferLength = (int)strlen(packet.m_TLSSendBuffer);
    packet.m_TLSSendNeeded = true;
    m_SendPackets.push_back(packet);

    // Assume that the login information was correct, persist the email address for this client.
    m_ClientEmail = std::string(emailaddress);
}

void LoginSession::ListGameAccounts(XMLPacket *Packet)
{
    rapidxml::xml_node<>* requestNode = Packet->m_XMLDocument.first_node("Request");
    auto gameCode = requestNode->first_node("GameCode")->value();
    int sequence = Packet->m_Meta[2] - '0';
    std::string username;
    std::string userGuid;

    auto userQuery = Database::Get().PrepareQuery("SELECT * FROM cligate.Users where userEmail = %0q;");
    auto result = userQuery.store(m_ClientEmail.c_str());

    printf("Looking up game accounts...\n");

    if (result.size() != 0) {
        if (result.size() > 1) {
            printf("Data consistency error, an email address appear's more than once in the users table.");
        }
        username =  std::string(result.front()["userName"]).c_str();
        userGuid =  std::string(result.front()["userGuid"]).c_str();
    }

    printf("Fetching game accounts for %s.\n", m_ClientEmail.c_str());
    printf("User GUID %s.\n", userGuid.c_str());

    GW2Packet messagePacket("/Presence/UserInfo", sequence, PT_MESSAGE);
    messagePacket.AddElement("Status", "online");
    messagePacket.AddElement("Aliases", "");
    messagePacket.AddElement("OnlineTimes", "");
    messagePacket.AddElement("AppData", "");
    messagePacket.AddElement("Channels", "");
    messagePacket.AddElement("Groups", "");
    messagePacket.AddElement("Contacts", "");
    messagePacket.AddElement("UserId", userGuid.c_str());
    messagePacket.AddElement("UserCenter", "5");
    messagePacket.AddElement("UserName", username.c_str());
    messagePacket.AddElement("ChangeId", "20");
    messagePacket.AddElement("NewBeginning", "");

    std::string compiledReply = "";
    std::string temporaryGameList[] = {
        "STS/1.0 200 OK\r\n",
        "s:"+std::to_string(sequence)+"R\r\n",
        "l:136\r\n",
        "\r\n",
        "<Reply type=\"array\">\n",
        "<Row>\n"
        "<GameCode>gw2</GameCode>\n"
        "<Alias>Guild Wars 2</Alias>\n"
        "<Created>2016-06-13T01:07:20Z</Created>\n"
        "</Row>\n"
        "</Reply>\n"
    };
    for (auto line : temporaryGameList) {
        compiledReply += line;
    }

    SessionSendPacket message;
    memset(message.m_TLSSendBuffer, 0, 4096);
    sprintf(message.m_TLSSendBuffer, messagePacket.Payload());
    message.m_TLSSendBufferLength = (int)strlen(message.m_TLSSendBuffer);
    message.m_TLSSendNeeded = true;
    m_SendPackets.push_back(message);

    SessionSendPacket gameArray;
    memset(gameArray.m_TLSSendBuffer, 0, 4096);
    sprintf(gameArray.m_TLSSendBuffer, compiledReply.c_str());
    gameArray.m_TLSSendBufferLength = (int)strlen(gameArray.m_TLSSendBuffer);
    gameArray.m_TLSSendNeeded = true;
    m_SendPackets.push_back(gameArray);
}

void LoginSession::ResumeAuthentication(XMLPacket *Packet)
{
    char response[512];
    int sequence = Packet->m_Meta[2] - '0';

    // Do not bother with a formal reply, there does not seem to be any variation in this call
    sprintf(response, "STS/1.0 400 Success\r\ns:%dR\r\nl:%d\r\n\r\n<Error server=\"1001\" module=\"4\" line=\"262\"/>\n", sequence, 45);

    auto responseSize = strlen(response);
    if (responseSize < 0) {
        responseSize = 0;
    }
    m_Client->Send(response, (unsigned int)strlen(response));
}

void LoginSession::RequestGameToken(XMLPacket *Packet)
{
    rapidxml::xml_node<>* requestNode = Packet->m_XMLDocument.first_node("Request");
    auto gameCode = requestNode->first_node("GameCode")->value();
    auto accountAlias = requestNode->first_node("AccountAlias")->value();

    printf("Generating Session Token.\n");
    int sequence = Packet->m_Meta[2] - '0';

    // Form a packet, the only element is the detination host name
    // GW2 will then try to connect to the specified server.
    GW2Packet replyPacket("", sequence, PT_REPLY);
    replyPacket.AddElement("Token", "fe9484c3-d53a-4225-a2d6-096be358af73");

    // Signal that there is TLS data to be sent next time round.
    SessionSendPacket packet;
    memset(packet.m_TLSSendBuffer, 0, 4096);
    sprintf(packet.m_TLSSendBuffer, replyPacket.Payload());
    packet.m_TLSSendBufferLength = (int)strlen(packet.m_TLSSendBuffer);
    packet.m_TLSSendNeeded = true;
    m_SendPackets.push_back(packet);

    printf("Handing client to Auth2.101.ArenaNetworks.com\n");

    m_LogoutRequested = true;
}

void LoginSession::Logout(XMLPacket *Packet)
{
    char response[512];
    printf("%s -> Logout Requested\n", m_Client->m_ClientIP);
    int sequence = Packet->m_Meta[2] - '0';

    // Do not bother with a formal reply, there does not seem to be any variation in this call
    sprintf(response, "STS/1.0 400 Success\r\ns:%dR\r\nl:%d\r\n\r\n<Error server=\"1001\" module=\"4\" line=\"262\"/>\n", sequence, 45);
    m_Client->Send(response, (int)strlen(response));

    m_LogoutRequested = true;
}

std::string LoginSession::CreateGuid()
{
#ifdef _WIN32
    GUID output;
    CoCreateGuid(&output);
    OLECHAR* guidString;
    StringFromCLSID(output, &guidString);

    std::wstring ws( guidString );
    std::string test( ws.begin(), ws.end() );

    return test;
#else
    uuid_t id;
    uuid_generate(id);
    char guidString[256];
    uuid_unparse(id, guidString);
    return std::string(guidString);
#endif
}

void LoginSession::SaveResumeToken(const char *userGuid, const char *resumeToken)
{
    auto userQuery = Database::Get().PrepareQuery("UPDATE cligate.Tokens SET tokenCode = %0q WHERE userGuid = %1q;");
    auto result = userQuery.store(resumeToken, userGuid);
    // TODO: check result.
}

bool LoginSession::CheckResumeToken(const char *userGuid, const char *resumeToken)
{
    auto userQuery = Database::Get().PrepareQuery("SELECT * FROM cligate.Tokens WHERE tokenCode = %0q AND userGuid = %1q;");
    auto result = userQuery.store(resumeToken, userGuid);
    if (result.size() != 0) {
        if (result.size() > 1) {
            printf("Data consistency error, an email address appear's more than once in the users table.");
        }
        return true;
    }
    return false;
}

void LoginSession::QueueLoginErrorMessage(int sequence)
{
    // Create the error packet
    GW2Packet replyPacket("", sequence, PT_REPLY_FAIL);
    replyPacket.Fill();
    replyPacket.SetErrorStatue("11");

    // Send it off
    SessionSendPacket packet;
    memset(packet.m_TLSSendBuffer, 0, 4096);
    sprintf(packet.m_TLSSendBuffer, replyPacket.Payload("ErrBadPasswd"));
    packet.m_TLSSendBufferLength = (int)strlen(packet.m_TLSSendBuffer);
    packet.m_TLSSendNeeded = true;
    m_SendPackets.push_back(packet);
}
