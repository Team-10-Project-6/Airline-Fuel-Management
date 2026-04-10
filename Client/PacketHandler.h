#pragma once

#include "ConnectionManager.h"
#include <string>

class PacketHandler {
public:
    PacketHandler(ConnectionManager& connMgr);
    bool sendLine(const std::string& dataLine, int lineNumber);

private:
    ConnectionManager& m_connMgr;  
    bool transmit(const std::string& packet) const;
};