#ifndef DATABASE_H
#define DATABASE_H

#include <mysql++.h>
#include <functional>

class Database
{
private:
    Database();

public:
    ~Database();

public:
    static Database* GetPtr();
    static Database& Get();

public:
    bool                        Initalize(const char* database, const char *serverAddress, const char *username, const char *password, unsigned int port);
    mysqlpp::Query              PrepareQuery(const char* query);
    mysqlpp::StoreQueryResult   RunQuery(const char* runQuery);
    void                        ItterateQuery(const char *runQuery, std::function<void(mysqlpp::StoreQueryResult::const_iterator)>& ittFunct);

private:
    static Database* g_Instance;

private:
    mysqlpp::Connection m_Connection;
};

#endif // DATABASE_H
