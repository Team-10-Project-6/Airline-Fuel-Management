#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <unordered_map>
#include <mutex>
#include <boost/asio/thread_pool.hpp>
#include "ServerPacketParser.h"
#include "TaskScheduler.h"
#include "TelemetryProcessor.h"
#include "DataStorage.h"

class ServerConnectionManager {
public:
    ServerConnectionManager(int port, TaskScheduler& scheduler);
    ~ServerConnectionManager();

    void startListening();
    void stop();

private:
    int port;
    SOCKET WelcomeSocket;
    bool isRunning;

    // counter for airplane id
    int airplaneCounter;

    // task queue pool
	TaskScheduler& scheduler;

    // internal connection thread pool
	boost::asio::thread_pool connectionPool;

    // computes per-packet fuel consumption and rate
    TelemetryProcessor m_telemetryProcessor;

    // persists fuel consumption records to SQLite
    DataStorage m_dataStorage;

    // tracks the first fuel reading per aircraft for FLIGHT_COMPLETE totals
    std::unordered_map<std::string, double> m_initialFuel;
    std::mutex m_initialFuelMutex;

    // Performs the HELLO handshake; sets clientID and returns true on success
    bool handleHandshake(SOCKET ConnectionSocket, std::string& clientID);

    // Receives telemetry packets for an already-handshaked connection
    void handleClientSession(SOCKET ConnectionSocket, const std::string& clientID);
};