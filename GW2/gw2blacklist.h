#ifndef GW2BLACKLIST_H
#define GW2BLACKLIST_H

#include <fstream>
#include <unordered_map>
#include <string>

class GW2BlackList
{
public:
    static bool Initalize(const char* blacklistFile);
    static bool CheckBlacklist(const char* clientIp);

private:
    static std::unordered_map<std::size_t, bool> m_BanedIpAddresses;
};

#endif // GW2BLACKLIST_H
