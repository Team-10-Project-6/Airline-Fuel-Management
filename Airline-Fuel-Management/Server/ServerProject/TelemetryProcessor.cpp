#include "TelemetryProcessor.h"
#include <cstdio>
#include <string>

bool TelemetryProcessor::process(const TelemetryPacket& packet, FuelConsumptionRecord& out) {
    double currTimestamp = static_cast<double>(packet.timestamp);
    if (currTimestamp == 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_state.find(packet.aircraftId);
    if (it == m_state.end()) {
        // First packet for this aircraft — store baseline, cannot calculate consumption yet
        m_state[packet.aircraftId] = { packet.fuel, currTimestamp, packet.fuel, currTimestamp };
        return false;
    }

    AircraftState& prev = it->second;
    double deltaTime = currTimestamp - prev.prevTimestamp;

    if (deltaTime <= 0.0) {
        // Out-of-order or duplicate timestamp — update state and skip
        prev.prevFuel      = packet.fuel;
        prev.prevTimestamp = currTimestamp;
        return false;
    }

    double fuelConsumed  = prev.prevFuel - packet.fuel;
    double totalConsumed = prev.initialFuel - packet.fuel;
    double totalElapsed  = currTimestamp - prev.initialTimestamp;
    double rate          = (totalElapsed > 0.0) ? (totalConsumed / totalElapsed) : 0.0;

    char buf[32];
    sprintf_s(buf, sizeof(buf), "%u", (unsigned int)packet.aircraftId);
    out.aircraftId = buf;
    sprintf_s(buf, sizeof(buf), "%u", packet.timestamp);
    out.timestamp  = buf;
    out.fuelConsumed    = fuelConsumed;
    out.consumptionRate = rate;

    prev.prevFuel      = packet.fuel;
    prev.prevTimestamp = currTimestamp;

    return true;
}
