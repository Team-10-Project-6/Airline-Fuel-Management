#include "TelemetryProcessor.h"
#include <iostream>
#include <sstream>
#include <ctime>

//cuz of broken stod() due to our timestamp format, we need to parse manually
static double parseTimestamp(const std::string& ts) {
    std::string s = ts;
    // trim leading whitespace
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return 0;
    s = s.substr(start);

    int day, month, year, hour, minute, second;
    char sep;
    std::istringstream ss(s);
    ss >> day >> sep >> month >> sep >> year >> hour >> sep >> minute >> sep >> second;
    if (ss.fail()) return 0;

    std::tm t{};
    t.tm_mday  = day;
    t.tm_mon   = month - 1;
    t.tm_year  = year - 1900;
    t.tm_hour  = hour;
    t.tm_min   = minute;
    t.tm_sec   = second;
    t.tm_isdst = -1;

    std::time_t result = std::mktime(&t);
    return (result == -1) ? 0 : static_cast<double>(result);
}

bool TelemetryProcessor::process(const TelemetryPacket& packet, FuelConsumptionRecord& out) {
    double currTimestamp = parseTimestamp(packet.timestamp);
    if (currTimestamp == 0) {
        std::cerr << ("[TelemetryProcessor] Could not parse timestamp: " + packet.timestamp + "\n");
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

    out.aircraftId      = packet.aircraftId;
    out.timestamp       = packet.timestamp;
    out.fuelConsumed    = fuelConsumed;
    out.consumptionRate = rate;

    prev.prevFuel      = packet.fuel;
    prev.prevTimestamp = currTimestamp;

    return true;
}
