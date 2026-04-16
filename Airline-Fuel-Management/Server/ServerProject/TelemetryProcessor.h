/**
 * @file TelemetryProcessor.h
 * @brief Processes mathematical metrics calculation for telemetry packets.
 */
#pragma once

#include <unordered_map>
#include <mutex>
#include "ServerPacketParser.h"

/**
 * @struct FuelConsumptionRecord
 * @brief A calculated data record mapping an aircraft to specific consumption analytics.
 */
struct FuelConsumptionRecord {
    std::string aircraftId;   // Unique aircraft string identifier
    std::string timestamp;    // ISO-8601 formatted timestamp
    double fuelConsumed;      // fuel delta since last packet
    double consumptionRate;   // average fuel burned per second since first packet
};

/**
 * @class TelemetryProcessor
 * @brief Thread-safe processor that caches aircraft previous states to compute fuel consumption deltas.
 */
class TelemetryProcessor {
public:
    /**
     * @brief Processes a telemetry packet to compute incremental fuel metrics.
     * @param packet The generic decoded telemetry data representation.
     * @param[out] out Output structured parameter holding calculated rates and deltas.
     * @return true if variables were computed properly, false if this is the first packet (no previous delta reference) or an invalid timestamp occurred.
     */
    bool process(const TelemetryPacket& packet, FuelConsumptionRecord& out);

private:
    struct AircraftState {
        double prevFuel;
        double prevTimestamp;
        double initialFuel;
        double initialTimestamp;
    };

    std::unordered_map<unsigned short, AircraftState> m_state;
    std::mutex m_mutex;
};
