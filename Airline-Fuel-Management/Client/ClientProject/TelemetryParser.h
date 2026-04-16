/**
 * @file TelemetryParser.h
 * @brief Parses the raw telemetry CSV data and feeds it line by line.
 */
#pragma once

#include <fstream>
#include <string>

/**
 * @class TelemetryParser
 * @brief Handles reading and parsing telemetry data files.
 */
class TelemetryParser {
public:
    /**
     * @brief Constructs a new TelemetryParser object.
     * @param filePath The path to the telemetry input file.
     */
    TelemetryParser(const std::string& filePath);
    /**
     * @brief Destructor. Closes the file.
     */
    ~TelemetryParser();

    /**
     * @brief Opens the file stream.
     * @return true if successful, false otherwise.
     */
    bool open();
    
    /**
     * @brief Checks if there are more lines available in the file.
     * @return true if more lines exist, false if EOF is reached.
     */
    bool hasMore() const;
    
    /**
     * @brief Retrieves the next line from the file.
     * @param line Reference to string to store the line contents.
     * @return true on success, false on failure or EOF.
     */
    bool nextLine(std::string& line);

    /**
     * @brief Gets the current line number being parsed.
     * @return int Current line number.
     */
    int lineNumber() const;

    /**
     * @brief Returns the parsed unix timestamp of the current line.
     * @return time_t The parsed timestamp.
     */
    time_t currentTimestamp() const;

private:
    std::string  m_filePath; //Path to the telemetry CSV file.
    std::ifstream m_file; //File stream.
    int m_lineNumber; //Count of total lines delivered.
    bool m_hasMore; //Keeps track of whether there are more lines to read after the current one.
    time_t m_currentTimestamp; //Parsed timestamp of current line
    time_t parseTimestamp(const std::string& line) const;

};