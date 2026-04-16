/**
 * @file ServerConnectionManager.h
 * @brief TCP Connection manager that accepts connections and spawns packet processing tasks.
 */
#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <unordered_map>
#include <mutex>
#include "ServerPacketParser.h"
#include "TaskScheduler.h"
#include "TelemetryProcessor.h"
#include "DataStorage.h"

/**
 * @class ServerConnectionManager
 * @brief Controls the main network listener and coordinates handshakes, tasks parsing, and session disconnects.
 */
class ServerConnectionManager {
public:
    /**
     * @brief Constructs a new ServerConnectionManager.
     * @param port The port to listen on.
     * @param scheduler TaskScheduler pool to delegate tasks into.
     */
    ServerConnectionManager(int port, TaskScheduler& scheduler);

    /**
     * @brief ServerConnectionManager Destructor. Cleans up sockets.
     */
    ~ServerConnectionManager();

    /**
     * @brief Binds the listening socket and begins accepting client connections in an infinite loop.
     */
    void startListening();

    /**
     * @brief Stops the server listener immediately.
     */
    void stop();

private:
    int port;
    SOCKET WelcomeSocket;
    bool isRunning;

    // counter for airplane id
    std::atomic<int> airplaneCounter{0};

    // task queue pool
	TaskScheduler& scheduler;

    // computes per-packet fuel consumption and rate
    TelemetryProcessor m_telemetryProcessor;

    // persists fuel consumption records to SQLite
    DataStorage m_dataStorage;

    // tracks the first fuel reading per aircraft for FLIGHT_COMPLETE totals
    std::unordered_map<std::string, double> m_initialFuel;
    std::mutex m_initialFuelMutex;

    /**
     * @brief Performs the initial HELLO handshake process with a connected client.
     * @param ConnectionSocket The raw client socket handle.
     * @param[out] clientID The parsed or generated Client/Aircraft ID passed by reference.
     * @return true if string handshake successfully completed, false on timeouts or bad formats.
     */
    bool handleHandshake(SOCKET ConnectionSocket, std::string& clientID);

    /**
     * @brief Constantly reads loop telemetry packets for an already-handshaked connection session.
     * @param ConnectionSocket The raw client socket handle.
     * @param clientID The associated Client/Aircraft ID for log/metrics associations.
     */
    void handleClientSession(SOCKET ConnectionSocket, const std::string& clientID);
};