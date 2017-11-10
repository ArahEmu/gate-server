TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

CFLAGS += -m32
QMAKE_CXXFLAGS += -m32

#Linux include paths
INCLUDEPATH += /usr/include/mysql++/ /usr/include/mysql/

#Windows include paths
INCLUDEPATH += ./Libs/libconfig
INCLUDEPATH += ./Libs/mysql++
INCLUDEPATH += 'C:/Program Files/MySQL/MySQL Connector C 6.1/include'
INCLUDEPATH += 'C:/OpenSSL-Win64/include'



win32 {
    LIBS += -lws2_32 -lole32 -lwsock32 -luuid
    LIBS += $$absolute_path("libconfig.lib", "G:/ArahEmu/gate-server/Libs/")
    LIBS += $$absolute_path("libconfig++.lib", "G:/ArahEmu/gate-server/Libs/")

    LIBS += $$absolute_path("mysqlpp_d.lib", "G:/ArahEmu/gate-server/Libs/")
    LIBS += $$absolute_path("mysqlpp_excommon.lib", "G:/ArahEmu/gate-server/Libs/")
    LIBS += $$absolute_path("mysqlpp_ssqls2parse.lib", "G:/ArahEmu/gate-server/Libs/")

    LIBS += $$absolute_path("ssleay32.lib", "C:/OpenSSL-Win64/lib/")
    LIBS += $$absolute_path("libeay32.lib", "C:/OpenSSL-Win64/lib/")
}

unix {
    #-lmongoclient -lboost_system
    LIBS += -lconfig++ -pthread -luuid
    LIBS += -L"/usr/local/lib64/" -lcrypto
    LIBS += -L"/usr/local/lib64/" -lssl
    LIBS += -L"/usr/lib64/" -lmysqlpp
}

SOURCES += main.cpp \
    framework.cpp \
    Networking/xmlpacket.cpp \
    Util/system.cpp \
    Util/serverconfig.cpp \
    Util/serverclock.cpp \
    Networking/serversocket.cpp \
    Networking/clientconnection.cpp \
    Login/loginserver.cpp \
    Login/loginclient.cpp \
    Util/messagetokenizer.cpp \
    GW2/gw2common.cpp \
    Login/securelogin.cpp \
    Util/text.cpp \
    Login/loginsession.cpp \
    3rdParty/sha256.cpp \
    Login/serverssl.cpp \
    Login/mitmsocket.cpp \
    GW2/gw2packet.cpp \
    GW2/gw2blacklist.cpp \
    Util/database.cpp \
    Auth/authserver.cpp \
    GW2/gw2authpacket.cpp \
    Mitm/gw2forwardingclient.cpp \
    Mitm/proxyserver.cpp \
    port.cpp

QMAKE_CXXFLAGS +=

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    framework.h \
    Networking/xmlpacket.h \
    Util/system.h \
    Util/serverconfig.h \
    Util/serverclock.h \
    Networking/serversocket.h \
    Networking/networking.h \
    Networking/clientconnection.h \
    Login/loginserver.h \
    Login/loginclient.h \
    3rdParty/rapidxml-1.13/rapidxml_utils.hpp \
    3rdParty/rapidxml-1.13/rapidxml_print.hpp \
    3rdParty/rapidxml-1.13/rapidxml_iterators.hpp \
    3rdParty/rapidxml-1.13/rapidxml.hpp \
    Util/messagetokenizer.h \
    GW2/gw2common.h \
    Login/securelogin.h \
    Util/text.h \
    Login/loginsession.h \
    3rdParty/sha256.h \
    Login/serverssl.h \
    Login/mitmsocket.h \
    GW2/gw2packet.h \
    GW2/gw2blacklist.h \
    Util/database.h \
    Auth/authserver.h \
    GW2/gw2authpacket.h \
    Mitm/gw2forwardingclient.h \
    Mitm/proxyserver.h \
    defines.h \
    port.h
