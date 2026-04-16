/**
 * @file DataStorage.h
 * @brief Manages persistence for fuel consumption metrics inside a SQLite database.
 */
#pragma once

#include <string>
#include <mutex>
#include "TelemetryProcessor.h"

// Forward-declare the SQLite handle so callers don't need to include sqlite3.h
struct sqlite3;

/**
 * @class DataStorage
 * @brief Interfaces with a SQLite backend for thread-safe database interactions.
 */
class DataStorage {
public:
    /**
     * @brief Opens (or creates) the SQLite database and ensures the schema exists.
     * @param dbPath The file path to the SQLite database.
     */
    explicit DataStorage(const std::string& dbPath);

    /**
     * @brief Destructor. Cleans up handles and closes the database connection.
     */
    ~DataStorage();

    /**
     * @brief Inserts one fuel consumption record into the database. Thread-safe.
     * @param record The FuelConsumptionRecord structure to insert.
     * @return true on success, false if the query fails.
     */
    bool insert(const FuelConsumptionRecord& record);

    /**
     * @brief Returns the total amount of fuel consumed for a specified aircraft across all records.
     * @param aircraftId The unique identifier of the aircraft.
     * @return double The total fuel consumed, or -1.0 on error or uninitialized Database.
     */
    double sumConsumed(const std::string& aircraftId);

private:
    sqlite3*   m_db;
    std::mutex m_mutex;

    bool createSchema();
};
