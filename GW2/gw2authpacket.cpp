#include "gw2authpacket.h"

Gw2AuthPacket::Gw2AuthPacket(unsigned char* buffer, unsigned int size) : m_Buffer(), m_Sig(0)
{
    // Copy the data to an internal buffer
    m_Buffer.reserve(size);
    std::copy(buffer, buffer + sizeof(unsigned char)*size, std::back_inserter(m_Buffer));

    // Parse the packet
    //m_Sig = *((uint16_t*)m_Buffer.data());
    m_Sig = INT16t(m_Buffer, 0);
}

void Gw2AuthPacket::Print()
{
    printf("Incomming Packet - Length %d\n-\n", m_Buffer.size());
    printf("Packet ID: %d\n", m_Sig);
    printf("Packet Bytes...\n");
    text::PrintHex((unsigned char*)m_Buffer.data(), m_Buffer.size());
    printf("\n-\n");
}
