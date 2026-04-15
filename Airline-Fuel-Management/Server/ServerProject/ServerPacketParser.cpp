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
    if (m_buffer.size() < sizeof(TelemetryWirePacket)) {
        return false;
    }

    TelemetryWirePacket wire;
    memcpy(&wire, m_buffer.data(), sizeof(wire));
    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + sizeof(wire));

    out.aircraftId = static_cast<unsigned short>(wire.aircraftId);
    out.timestamp  = wire.timestamp;
    out.fuel       = wire.fuel;
    return true;
}
