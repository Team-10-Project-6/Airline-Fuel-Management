#include "TelemetryParser.h"
#include <iostream>
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
        m_hasMore = !m_file.eof();
        return true;
    }
    m_hasMore = false;
    return false;
}

int TelemetryParser::lineNumber() const {
    return m_lineNumber;
}