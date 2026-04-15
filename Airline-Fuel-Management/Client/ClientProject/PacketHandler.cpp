#include "PacketHandler.h"
#include <iostream>
#include <stdexcept>
using namespace std;

PacketHandler::PacketHandler(ConnectionManager& connMgr) : m_connMgr(connMgr), m_prevTimestamp(0) {}

bool PacketHandler::sendLine(const string& dataLine, int lineNumber, time_t timestamp) {

    // Sleep for the time delta between this line and the previous one
    if (m_prevTimestamp != 0 && timestamp != 0) {
        double delta = difftime(timestamp, m_prevTimestamp);
        if (delta > 0 && delta < 60) {
            Sleep(static_cast<int>(delta * 1000));
        }
    }
    m_prevTimestamp = timestamp;

    // Extract fuel value from dataLine: "<timestamp>,<fuel>,"
    size_t firstComma  = dataLine.find(',');
    size_t secondComma = dataLine.find(',', firstComma + 1);
    if (firstComma == string::npos || secondComma == string::npos) {
        cerr << "[WARN] Malformed data line " << lineNumber << ": " << dataLine << endl;
        return false;
    }
    float fuel;
    try {
        fuel = stof(dataLine.substr(firstComma + 1, secondComma - firstComma - 1));
    } catch (const exception&) {
        cerr << "[WARN] Could not parse fuel on line " << lineNumber << endl;
        return false;
    }

    TelemetryWirePacket pkt{};
    pkt.timestamp  = static_cast<uint32_t>(timestamp);
    pkt.fuel       = fuel;
    pkt.aircraftId = static_cast<uint16_t>(stoi(m_connMgr.getAircraftId()));

    if (!transmit(pkt)) {
        cerr << "[WARN] Send failed on line " << lineNumber << ". Error: " << WSAGetLastError() << endl;

        // Reconnect and retry once
        if (!m_connMgr.reconnect()) {
            cerr << "[ERROR] Reconnect failed. Aborting." << endl;
            return false;
        }

        cout << "[INFO] Resuming from line " << lineNumber << "." << endl;
        pkt.aircraftId = static_cast<uint16_t>(stoi(m_connMgr.getAircraftId()));

        if (!transmit(pkt)) {
            cerr << "[ERROR] Send failed again after reconnect. Aborting." << endl;
            return false;
        }
    }

    return true;
}

bool PacketHandler::transmit(const TelemetryWirePacket& pkt) const {
    char buf[sizeof(TelemetryWirePacket)];
    memcpy(buf, &pkt, sizeof(pkt));
    return send(m_connMgr.getSocket(), buf, sizeof(buf), 0) != SOCKET_ERROR;
}
