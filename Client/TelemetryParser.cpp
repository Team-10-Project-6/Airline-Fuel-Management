#include "TelemetryParser.h"
#include <iostream>
#include <sstream>

using namespace std;

TelemetryParser::TelemetryParser(const string& filePath) : m_filePath(filePath) , m_lineNumber(0) , m_hasMore(false) {}

TelemetryParser::~TelemetryParser() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

bool TelemetryParser::open() {
    m_file.open(m_filePath);
    if (!m_file.is_open()) {
        cerr << "[ERROR] Failed to open telemetry file: " << m_filePath << endl;
        return false;
    }
    //Skip the "FUEL TOTAL QUANTITY," section of the first line
    string prefix = "FUEL TOTAL QUANTITY,";
    m_file.ignore(prefix.length());
    m_hasMore = !m_file.eof();
    return true;
}

bool TelemetryParser::hasMore() const {
    return m_hasMore;
}

bool TelemetryParser::nextLine(string& line) {
    string raw;
    while (getline(m_file, raw)) {
        if (raw.empty()) continue; //Skip blank lines.

        m_lineNumber++;
        line = raw;
        m_currentTimestamp = parseTimestamp(raw);
        m_hasMore = !m_file.eof();
        return true;
    }
    m_hasMore = false;
    return false;
}

int TelemetryParser::lineNumber() const {
    return m_lineNumber;
}

time_t TelemetryParser::currentTimestamp() const {
    return m_currentTimestamp;
}

// Parses "3_3_2023 14:53:21" from the start of a data line
// Format is: " D_M_YYYY HH:MM:SS,fuel,"
time_t TelemetryParser::parseTimestamp(const string& line) const {
    // Find the comma to isolate the timestamp field
    size_t commaPos = line.find(',');
    if (commaPos == string::npos) return 0;

    string ts = line.substr(0, commaPos);

    // Trim leading whitespace
    size_t start = ts.find_first_not_of(" \t");
    if (start == string::npos) return 0;
    ts = ts.substr(start);

    // Parse "D_M_YYYY HH:MM:SS"
    int day, month, year, hour, minute, second;
    char sep;
    istringstream ss(ts);
    // D_M_YYYY
    ss >> day >> sep >> month >> sep >> year;
    // HH:MM:SS
    ss >> hour >> sep >> minute >> sep >> second;

    if (ss.fail()) return 0;

    tm t{};
    t.tm_mday  = day;
    t.tm_mon   = month - 1; // tm_mon is 0-based
    t.tm_year  = year - 1900;
    t.tm_hour  = hour;
    t.tm_min   = minute;
    t.tm_sec   = second;
    t.tm_isdst = -1; // Let the system determine DST

    return mktime(&t);
}