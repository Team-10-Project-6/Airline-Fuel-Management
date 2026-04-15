#pragma once

#include "ConnectionManager.h"
#include <cstring>
#include <ctime>
#include <string>

struct TelemetryWirePacket {
    unsigned int  timestamp;        // Unix epoch seconds
    float         fuel;             // Fuel quantity
    unsigned int  aircraftId : 16;  // Aircraft ID (0-65535)
};

class PacketHandler {
public:
    PacketHandler(ConnectionManager& connMgr);
    bool sendLine(const std::string& dataLine, int lineNumber, time_t timestamp);

private:
    ConnectionManager& m_connMgr;
    time_t m_prevTimestamp;
    bool transmit(const TelemetryWirePacket& pkt) const;
};
