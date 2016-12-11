#include "gw2blacklist.h"

std::unordered_map<std::size_t, bool> GW2BlackList::m_BanedIpAddresses = std::unordered_map<std::size_t, bool>();

bool GW2BlackList::Initalize(const char *blacklistFile)
{
    std::ifstream   blackListFileStream(blacklistFile);
    std::string     currentIP;

    while(std::getline(blackListFileStream, currentIP)) {
        m_BanedIpAddresses.insert(std::pair<std::size_t, bool>(
                                      std::hash<std::string>{}(currentIP), true));
    }

    return true;
}

bool GW2BlackList::CheckBlacklist(const char *clientIp)
{
    auto findItterator = m_BanedIpAddresses.find(std::hash<std::string>{}(std::string(clientIp)));
    if (findItterator == m_BanedIpAddresses.end())
        return false;
    return true;
}
