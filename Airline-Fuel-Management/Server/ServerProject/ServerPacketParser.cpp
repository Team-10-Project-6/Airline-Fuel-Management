#include "ServerPacketParser.h"
#include <cstring>

// Must match TelemetryWirePacket in the client's PacketHandler.h
struct TelemetryWirePacket {
    unsigned int timestamp;
    float        fuel;
    unsigned int aircraftId : 16;
};

void ServerPacketParser::feed(const char* data, int length) {
    m_buffer.insert(m_buffer.end(), data, data + length);
}

bool ServerPacketParser::tryParse(TelemetryPacket& out) {
    if (m_buffer.size() - m_offset < sizeof(TelemetryWirePacket)) {
        return false;
    }

    TelemetryWirePacket wire;
    memcpy(&wire, m_buffer.data() + m_offset, sizeof(wire));
    m_offset += sizeof(wire);

    // Flush consumed bytes once the offset grows large enough
    if (m_offset >= 4096) {
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + m_offset);
        m_offset = 0;
    }

    out.aircraftId = static_cast<unsigned short>(wire.aircraftId);
    out.timestamp  = wire.timestamp;
    out.fuel       = wire.fuel;
    return true;
}
