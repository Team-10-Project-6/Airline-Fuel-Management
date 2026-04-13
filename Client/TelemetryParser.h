#pragma once

#include <fstream>
#include <string>

class TelemetryParser {
public:
    TelemetryParser(const std::string& filePath);
    ~TelemetryParser();

    bool open();
    
    bool hasMore() const;
    
    bool nextLine(std::string& line);

    int lineNumber() const;

private:
    std::string  m_filePath; //Path to the telemetry CSV file.
    std::ifstream m_file; //File stream.
    int m_lineNumber; //Count of total lines delivered.
    bool m_hasMore; //Keeps track of whether there are more lines to read after the current one.
};