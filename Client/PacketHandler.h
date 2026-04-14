#pragma once

#include "ConnectionManager.h"
#include <string>
#include <ctime>

class PacketHandler {
public:
    PacketHandler(ConnectionManager& connMgr);
    bool sendLine(const std::string& dataLine, int lineNumber, time_t timestamp);

private:
    ConnectionManager& m_connMgr;  
    time_t m_prevTimestamp; // Timestamp of the previous line
    bool transmit(const std::string& packet) const;
};