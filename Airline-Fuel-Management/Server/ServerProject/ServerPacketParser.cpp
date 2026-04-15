#include "ServerPacketParser.h"
#include <sstream>
#include <stdexcept>

void ServerPacketParser::feed(const char* data, int length) {
    m_buffer.append(data, length);
}

bool ServerPacketParser::tryParse(TelemetryPacket& out) {
    size_t pos = m_buffer.find('\n');
    if (pos == std::string::npos) {
        return false; // No complete packet yet
    }

    std::string line = m_buffer.substr(0, pos);
    m_buffer.erase(0, pos + 1);
    //remove carriage return
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    return parseLine(line, out);
}

bool ServerPacketParser::parseLine(const std::string& line, TelemetryPacket& out) const {
    //<aircraftId>,<timestamp>,<fuel>
    std::istringstream ss(line);
    std::string aircraftId, timestamp, fuelStr;

    if (!std::getline(ss, aircraftId,  ',')) return false;
    if (!std::getline(ss, timestamp,   ',')) return false;
    if (!std::getline(ss, fuelStr,     ',')) return false;

    try {
        out.fuel = std::stod(fuelStr);
    } catch (const std::exception&) {
        return false;
    }

    out.aircraftId  = std::move(aircraftId);
    out.timestamp   = std::move(timestamp);
    return true;
}
