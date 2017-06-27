﻿#include "framework.h"

Framework::Framework() : m_Running(true), m_Options(), m_LoginServer(), m_AuthServer(), m_ProxyServer(), m_Clock(5000)
{

}

int Framework::Run(int argc, char **argv)
{
    printf(">> GW2SEX - Guild Wars 2 Server Emulator for XNIX\n");
    printf(">> Some ideas taken from http://poke152.blogspot.com.au/2012_12_01_archive.html\n");
    printf(">> Developed by Nomelx\n\n");

    System::IgnoreSigPipe();    // For debugging with GDB
    System::CheckRoot();        // Servers should have a unique user to run the application

    if (!Configure(argc, argv)) {
        printf("!) Server attempting to start with incorrect configuration.\n");
        return 1;
    }

    if (strcmp(m_Options.m_ServerMode, "gateway") == 0) {
        return RunGateway();
    }

    if (strcmp(m_Options.m_ServerMode, "auth") == 0) {
        return RunAuth();
    }

    if (strcmp(m_Options.m_ServerMode, "proxy") == 0) {
        return RunProxy();
    }

    return 0;
}

int Framework::RunGateway()
{
    int gatewayPort = GetSettingInteger("networking.login_server.port");
    const char* gatewayCertificate = GetSettingString("networking.login_server.certFile");
    const char* gatewayPrivateKey = GetSettingString("networking.login_server.keyFile");

    printf("Starting %s\n", m_Options.m_ServerName);
    printf("Starting login gateway server on port %d\n", gatewayPort);

    // Establish a connection to mysql db
    if (!Database::Get().Initalize(
                GetSettingString("networking.database.db_name"),
                GetSettingString("networking.database.db_address"),
                GetSettingString("networking.database.db_username"),
                GetSettingString("networking.database.db_password"),
                GetSettingInteger("networking.database.db_port")
                )) {
        printf("!) Unable to connect to database server (listner).\n");
        return -1;
    } else {
        printf("Connected to database %s on %s\n", GetSettingString("networking.database.db_name"), GetSettingString("networking.database.db_address"));
    }

    // This will spawn off the login / server socket thread
    if (!m_LoginServer.Startup(gatewayPort,
                               gatewayCertificate,
                               gatewayPrivateKey,
                               m_Options.m_MITMMode)) {
        m_LoginServer.Shutdown();
        printf("!) Unable to create login server (listner).\n");
        return 1;
    }

    // Populates the blacklist file
    if (!GW2BlackList::Initalize(GetSettingString("networking.login_server.blacklist"))) {
        m_LoginServer.Shutdown();
        printf("!) Unable to read ban list (listner).\n");
        return 1;
    }

    // Start the server clock for regulation, this does not regulate the listning socket.
    // We would idealy use this to make sure we are not spending too much time doing expensive operations.
    m_Clock.SetRate(m_Options.m_ServerRate);
    m_Clock.Start();

    // Main thread loop, do any non blocking functions here.
    while (m_Running) {

        // Poll the users, this is where all player managment is done.
        if (m_Clock.InFrame()) {
            m_LoginServer.Update();

            // Update the clock.
            m_Clock.Frame();

        } else {
            ::usleep(1000);
        }

    }

    m_LoginServer.Shutdown();

    return 0;
}

int Framework::RunAuth()
{
    int authPort = GetSettingInteger("networking.auth_server.port");

    printf("Starting %s\n", m_Options.m_ServerName);
    printf("Starting auth server on port %d\n", authPort);

    m_AuthServer.Startup(authPort);
    m_Clock.Start();

    // Main thread loop, do any non blocking functions here.
    while (m_Running) {

        // Update
        m_AuthServer.Update();

        // Update the clock.
        m_Clock.Frame();

    }

    m_LoginServer.Shutdown();
}

int Framework::RunProxy()
{
    int authPort = GetSettingInteger("networking.proxy_server.port");
    const char* gatewayCertificate = GetSettingString("networking.proxy_server.certFile");
    const char* gatewayPrivateKey = GetSettingString("networking.proxy_server.keyFile");

    printf("Starting %s\n", m_Options.m_ServerName);
    printf("Starting proxy server on port %d\n", authPort);

    m_ProxyServer.Startup(authPort, gatewayCertificate, gatewayPrivateKey);
    m_Clock.Start();

    // Main thread loop, do any non blocking functions here.
    while (m_Running) {

        // Update
        m_ProxyServer.Update();

        // Update the clock.
        m_Clock.Frame();

    }

    m_ProxyServer.Shutdown();
}

bool Framework::Configure(int argc, char **argv)
{
    // Check argc agains the required argument count, we subtract one because the OS includes the running path.
    if ((argc-1)!=REQ_ARGUMENTS_C) {
        ShowUsage();
        return false;
    }

    // Assume arg 1 is the config file.
    if (!Import(argv[1])) {
        return false;
    }

    // Load any missions critical options here.
    sprintf(m_Options.m_ServerName, "%s", GetSettingString("networking.server.name"));
    sprintf(m_Options.m_ServerMode, "%s", GetSettingString("networking.server.mode"));

    m_Options.m_MITMMode = GetSettingBool("networking.server.mitmMode");
    m_Options.m_ServerRate = GetSettingInteger("networking.server.rate");

    return true;
}

void Framework::ShowUsage() {
    printf("Usage: gw2sex config-file[.cofg]\n");
    printf("\n");
}
