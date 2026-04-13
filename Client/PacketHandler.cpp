#include "PacketHandler.h"
#include <iostream>
using namespace std;

PacketHandler::PacketHandler(ConnectionManager& connMgr) : m_connMgr(connMgr) {}

bool PacketHandler::sendLine(const string& dataLine, int lineNumber) {

    string packet = m_connMgr.getAircraftId() + "," + dataLine + "\n";

    if (!transmit(packet)) {
        cerr << "[WARN] Send failed on line " << lineNumber << ". Error: " << WSAGetLastError() << endl;

        // Reconnect and retry once
        if (!m_connMgr.reconnect()) {
            cerr << "[ERROR] Reconnect failed. Aborting." << endl;
            return false;
        }

        cout << "[INFO] Resuming from line " << lineNumber << "." << endl;

        //Rebuild packet with aircraft ID
        packet = m_connMgr.getAircraftId() + "," + dataLine + "\n";

        if (!transmit(packet)) {
            cerr << "[ERROR] Send failed again after reconnect. Aborting." << endl;
            return false;
        }
    }

    return true;
}

bool PacketHandler::transmit(const string& packet) const {
    return send(m_connMgr.getSocket(), packet.c_str(), static_cast<int>(packet.size()), 0) != SOCKET_ERROR;
}