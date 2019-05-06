#ifndef GW2AUTHPACKET_H
#define GW2AUTHPACKET_H

#define INT16t(X, Y) *((uint16_t*)X.data()+Y);

#include <vector>
#include <iterator>

#include "../Util/text.h"

class Gw2AuthPacket
{
public:
    Gw2AuthPacket(unsigned char* buffer, unsigned int size);
public:
    void Print();
private:
    std::vector<unsigned char> m_Buffer;
private:
    uint16_t m_Sig;
};

#endif // GW2AUTHPACKET_H
