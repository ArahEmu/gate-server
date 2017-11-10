#include "gw2packet.h"

#include "../3rdParty/rapidxml-1.13/rapidxml_print.hpp"

GW2Packet::GW2Packet(const char* path, int sequence, GW2PacketType type) :m_PacketHeader(),  m_PacketData(),
    m_Sequence(sequence), m_Type(type), m_Document(), m_RequestNode(nullptr), m_Elements(), m_BufferData(nullptr)
{
    // Set the request path.
    memset(m_PacketHeader, 0, sizeof(m_PacketHeader));
}

void GW2Packet::Fill()
{
    switch(m_Type) {
        case PT_REQUSET:
            m_RequestNode = m_Document.allocate_node(rapidxml::node_element, "Request");
            break;
        case PT_REPLY:
            m_RequestNode = m_Document.allocate_node(rapidxml::node_element, "Reply");
            break;
        case PT_REPLY_FAIL:
            m_RequestNode = m_Document.allocate_node(rapidxml::node_element, "Error");
            break;
        case PT_MESSAGE:
            m_RequestNode = m_Document.allocate_node(rapidxml::node_element, "Message");
            break;
    }
}

void GW2Packet::AddElement(const char *key, const char *value)
{
    m_Elements.push_back(
                m_Document.allocate_node(rapidxml::node_element, key, value));
}

void GW2Packet::AddAttribute(const char *elementKey, const char *key, const char *attribute)
{
    for (rapidxml::xml_node<>* element : m_Elements) {
        const char* name = element->name();
        if (strcmp(name, elementKey) == 0) {
            element->append_attribute(
                        m_Document.allocate_attribute(key, attribute));
        }
    }
}

void GW2Packet::SetErrorStatue(const char *error_code)
{
    m_RequestNode->append_attribute(
                m_Document.allocate_attribute("code", error_code));
    m_RequestNode->append_attribute(
                m_Document.allocate_attribute("server", "1001"));
    m_RequestNode->append_attribute(
                m_Document.allocate_attribute("module", "8008"));
    m_RequestNode->append_attribute(
                m_Document.allocate_attribute("line", "186"));
}

const char *GW2Packet::Payload(const char *failCode)
{
	if (m_BufferData != nullptr) {
		delete[] m_BufferData;
		m_BufferData = nullptr;
	}

    std::string header = "";
    std::string reply = "";

    if (m_RequestNode == nullptr) {
        Fill();
    }

    switch(m_Type) {
        case PT_REPLY:
            header = "STS/1.0 200 OK\r\n";
            header += "s:" + std::to_string(m_Sequence) + "R\r\n";
            break;
        case PT_REPLY_FAIL:
            header = std::string("STS/1.0 400 ") + failCode + "\r\n";
            header += "s:" + std::to_string(m_Sequence) + "R\r\n";
            break;
    }

    for (rapidxml::xml_node<>* element : m_Elements) {
        m_RequestNode->append_node(element);
    }

    // Build the reply / request / message body
    m_Document.append_node(m_RequestNode);
    std::string bodyBuffer;
    rapidxml::print(std::back_inserter(bodyBuffer), m_Document, 0);

    // Append the header, rapidxml puts an extra newline at the end that GW2 doesnt like
    header += "l:"+ std::to_string(bodyBuffer.size()-1) + "\r\n";
    header += "\r\n";

    reply += header;
    reply += bodyBuffer;

	// Write the packet to disk.
    FILE* debug = fopen("sent.log", "wb");
    fwrite(reply.c_str(), sizeof(char), reply.size(), debug);
    fclose(debug);

	// Copy our packet data to the return buffer.
	m_BufferData = new char[reply.size()+1];
	memset(m_BufferData, 0, reply.size() + 1);
	memcpy(m_BufferData, reply.c_str(), reply.size());

    return m_BufferData;
}
