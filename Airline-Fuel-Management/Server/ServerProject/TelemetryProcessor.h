#pragma once

#include <unordered_map>
#include <mutex>
#include "ServerPacketParser.h"

struct FuelConsumptionRecord {
    std::string aircraftId;
    std::string timestamp;
    double fuelConsumed;      // fuel delta since last packet
    double consumptionRate;   // average fuel burned per second since first packet
};

class TelemetryProcessor {
public:
    // Process a telemetry packet and compute fuel consumption.
    // Returns false on the first packet for an aircraft (no prior state to diff against)
    // or if the timestamp is zero or non-increasing.
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
