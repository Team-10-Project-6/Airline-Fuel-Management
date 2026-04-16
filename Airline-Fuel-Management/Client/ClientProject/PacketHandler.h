/**
 * @file PacketHandler.h
 * @brief Handles formatting and sending telemetry packets.
 */
#pragma once

#include "ConnectionManager.h"
#include <cstring>
#include <ctime>
#include <string>

/**
 * @struct TelemetryWirePacket
 * @brief Lightweight structure representing the raw wire format of a telemetry packet.
 */
struct TelemetryWirePacket {
    unsigned int  timestamp;        // Unix epoch seconds
    float         fuel;             // Fuel quantity
    unsigned int  aircraftId : 16;  // Aircraft ID (0-65535)
};

/**
 * @class PacketHandler
 * @brief Parses raw string data lines and transmits them as binary TelemetryWirePackets.
 */
class PacketHandler {
public:
    /**
     * @brief Constructs a new PacketHandler.
     * @param connMgr Reference to the connection manager used for transmission.
     */
    PacketHandler(ConnectionManager& connMgr);
    /**
     * @brief Parses a data line and sends the resulting telemetry packet.
     * @param dataLine A string containing telemetry values separated by commas.
     * @param lineNumber The line number from the source data file.
     * @param timestamp The extracted unix timestamp.
     * @return true if successful, false otherwise.
     */
    bool sendLine(const std::string& dataLine, int lineNumber, time_t timestamp);

private:
    ConnectionManager& m_connMgr;
    time_t m_prevTimestamp;
    bool transmit(const TelemetryWirePacket& pkt) const;
};
