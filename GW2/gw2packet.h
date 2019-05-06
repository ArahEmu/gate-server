#ifndef GW2PACKET_H
#define GW2PACKET_H

#include "../3rdParty/rapidxml-1.13/rapidxml.hpp"

#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>

enum GW2PacketType {
    PT_REQUSET,
    PT_REPLY,
    PT_REPLY_FAIL,
    PT_MESSAGE
};

class GW2Packet
{
public:
    GW2Packet(const char* path, int sequence, GW2PacketType type);
public:
    void                                Fill();
    void                                AddElement(const char* key, const char* value);
    void                                AddAttribute(const char* elementKey, const char* key, const char* attribute);
    void                                SetErrorStatue(const char* error_code);
    const char*                         Payload(const char* failCode = 0);
private:
    char                                m_PacketHeader[2048];
    char                                m_PacketData[4096];
    int                                 m_Sequence;
    GW2PacketType                       m_Type;
    rapidxml::xml_document<>            m_Document;
    rapidxml::xml_node<>*               m_RequestNode;
    std::vector<rapidxml::xml_node<>*>  m_Elements;
private:
	char*								m_BufferData;
};

#endif // GW2PACKET_H
