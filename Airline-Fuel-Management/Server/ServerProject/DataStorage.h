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

private:
    sqlite3*   m_db;
    std::mutex m_mutex;

    bool createSchema();
};
