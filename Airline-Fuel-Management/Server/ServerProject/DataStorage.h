#pragma once

#include <string>
#include <mutex>
#include "TelemetryProcessor.h"

// Forward-declare the SQLite handle so callers don't need to include sqlite3.h
struct sqlite3;

class DataStorage {
public:
    // Opens (or creates) the SQLite database at dbPath and ensures the schema exists.
    explicit DataStorage(const std::string& dbPath);
    ~DataStorage();

    // Inserts one fuel consumption record.  Thread-safe.
    // Returns true on success.
    bool insert(const FuelConsumptionRecord& record);

    // Returns the sum of fuel_consumed for the given aircraft across all stored records.
    // Returns -1.0 on error or if the database is not open.
    double sumConsumed(const std::string& aircraftId);

private:
    sqlite3*   m_db;
    std::mutex m_mutex;

    bool createSchema();
};
