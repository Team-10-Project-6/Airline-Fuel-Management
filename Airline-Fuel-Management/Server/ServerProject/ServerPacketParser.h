#pragma once

#include <string>

// <aircraftId>,<telemetryId>,<timestamp>,<fuel>\n
struct TelemetryPacket {
    std::string aircraftId;
    std::string timestamp;
    double      fuel;
};

class ServerPacketParser {
public:
    void feed(const char* data, int length);
    bool tryParse(TelemetryPacket& out);

private:
    std::string m_buffer;
    bool parseLine(const std::string& line, TelemetryPacket& out) const;
};
