#pragma once

#include <vector>

struct TelemetryPacket {
    unsigned short aircraftId;  // Aircraft ID
    unsigned int   timestamp;   // Unix epoch seconds
    float          fuel;        // Fuel quantity
};

class ServerPacketParser {
public:
    void feed(const char* data, int length);
    bool tryParse(TelemetryPacket& out);

private:
    std::vector<char> m_buffer;
};
