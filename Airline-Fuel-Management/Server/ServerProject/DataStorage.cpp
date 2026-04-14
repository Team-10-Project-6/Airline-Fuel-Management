#include "DataStorage.h"
#include "sqlite3.h"
#include <iostream>

DataStorage::DataStorage(const std::string& dbPath) : m_db(nullptr) {
    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
        std::cerr << "[DataStorage] Failed to open database '"
                  << dbPath << "': " << sqlite3_errmsg(m_db) << std::endl;
        sqlite3_close(m_db);
        m_db = nullptr;
        return;
    }

    // WAL mode gives better concurrent write performance
    sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    if (!createSchema()) {
        std::cerr << "[DataStorage] Schema creation failed." << std::endl;
    } else {
        std::cout << "[DataStorage] Database ready: " << dbPath << std::endl;
    }
}

DataStorage::~DataStorage() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool DataStorage::createSchema() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS fuel_consumption ("
        "  id               INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  aircraft_id      TEXT    NOT NULL,"
        "  timestamp        TEXT    NOT NULL,"
        "  fuel_consumed    REAL    NOT NULL,"
        "  consumption_rate REAL    NOT NULL"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[DataStorage] createSchema error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DataStorage::insert(const FuelConsumptionRecord& record) {
    if (!m_db) return false;

    const char* sql =
        "INSERT INTO fuel_consumption "
        "  (aircraft_id, timestamp, fuel_consumed, consumption_rate) "
        "VALUES (?, ?, ?, ?);";

    std::lock_guard<std::mutex> lock(m_mutex);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DataStorage] prepare failed: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    sqlite3_bind_text  (stmt, 1, record.aircraftId.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, record.timestamp.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, record.fuelConsumed);
    sqlite3_bind_double(stmt, 4, record.consumptionRate);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok) {
        std::cerr << "[DataStorage] insert failed: " << sqlite3_errmsg(m_db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return ok;
}
