/**
 * @file ServerPacketParser.h
 * @brief Parses incoming binary streams into distinct TelemetryPacket structures.
 */
#pragma once

#include <vector>

/**
 * @struct TelemetryPacket
 * @brief Represents the decoded telemetry data received from an aircraft.
 */
struct TelemetryPacket {
    unsigned short aircraftId;  // Aircraft ID
    unsigned int   timestamp;   // Unix epoch seconds
    float          fuel;        // Fuel quantity
};

/**
 * @class ServerPacketParser
 * @brief Buffers raw incoming bytes and extracts complete TelemetryPacket structures.
 */
class ServerPacketParser {
public:
    /**
     * @brief Appends raw bytes from the network stream into the internal buffer.
     * @param data The byte array to append.
     * @param length The number of bytes to read from the data array.
     */
    void feed(const char* data, int length);

    /**
     * @brief Attempts to decode one complete packet from the buffer.
     * @param[out] out The TelemetryPacket to populate if enough data exists.
     * @return true if a packet was successfully extracted, false if more bytes are needed.
     */
    bool tryParse(TelemetryPacket& out);

private:
    std::vector<char> m_buffer;
    size_t            m_offset = 0;
};
