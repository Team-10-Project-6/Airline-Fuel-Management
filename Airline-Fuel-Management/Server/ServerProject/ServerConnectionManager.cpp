#include "ServerConnectionManager.h"
#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include <boost/asio/post.hpp>

#pragma warning(disable : 4996)

using namespace std;

ServerConnectionManager::ServerConnectionManager(int serverPort, TaskScheduler& scheduler)
    : port(serverPort), WelcomeSocket(INVALID_SOCKET), isRunning(false), airplaneCounter(0),
      scheduler(scheduler), connectionPool(100), m_dataStorage("fuel_data.db") {}

ServerConnectionManager::~ServerConnectionManager() {
    stop();
}

void ServerConnectionManager::startListening() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return;
    }

    // create welcome socket
    if ((WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        cerr << "Failed to create WelcomeSocket.\n";
        return;
    }

    // server struct
    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = INADDR_ANY; 
    SvrAddr.sin_port = htons(port); 

    // Add '::' before bind to use the global/WinSock version
    if ((::bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
        cerr << "Bind failed.\n";
        closesocket(WelcomeSocket);
        WSACleanup();
        return;
    }

    // listen for new clients
    if (listen(WelcomeSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed.\n";
        closesocket(WelcomeSocket);
        WSACleanup();
        return;
    }

    isRunning = true;
    cout << ("Waiting for client connection on port " + to_string(port) + "...\n");

    SOCKET ConnectionSocket = SOCKET_ERROR;

    while (isRunning) {
        // wait for incoming connection
        if ((ConnectionSocket = accept(WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
            if (isRunning) cerr << "Accept failed.\n";
            continue;
        }

        cout << "Client connection made.\n";

        // Post the client connection handling to the thread pool
        boost::asio::post(connectionPool, [this, ConnectionSocket]() {
            std::string clientID;
            if (handleHandshake(ConnectionSocket, clientID)) {
                handleClientSession(ConnectionSocket, clientID);
            }
            closesocket(ConnectionSocket);
        });

    }
    
    connectionPool.join();
}

bool ServerConnectionManager::handleHandshake(SOCKET ConnectionSocket, string& clientID) {
    char RxBuffer[256] = {0};

    // receive hello message from client
    int bytesReceived = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
    if (bytesReceived <= 0) {
        cerr << "Handshake failed: No data received.\n";
        return false;
    }

    string request(RxBuffer);

    // parse handshake message
    if (request.find("HELLO NEW") != string::npos) {    // if new client...
        clientID = to_string(airplaneCounter++);        // ...assign new ID
        cout << ("Assigning new Aircraft ID: " + clientID + "\n");
    }
    else if (request.find("HELLO ") != string::npos) {  // if existing client...
        clientID = request.substr(6);                   // ...extract existing ID

        // remove newline if found
        size_t pos = clientID.find('\n');
        if (pos != string::npos) {
            clientID.erase(pos);
        }
        cout << ("Resuming flight for Aircraft ID: " + clientID + "\n");
    }
    else {
        cerr << "Invalid handshake request.\n";
        return false;
    }

    // send back assigned or existing ID
    string response = clientID + "\n";
    if (send(ConnectionSocket, response.c_str(), response.size(), 0) == SOCKET_ERROR) {
        cerr << "Failed to send handshake response.\n";
        return false;
    }

    return true;
}

void ServerConnectionManager::handleClientSession(SOCKET ConnectionSocket, const string& clientID) {
    cout << ("[" + clientID + "] Session started.\n");

    ServerPacketParser parser;
    char rxBuffer[4096];

    // Reference-counted completion guard.
    // Starts at 1 as a sentinel for "FLIGHT_COMPLETE not yet received".
    // Each telemetry task increments before enqueue, decrements on completion.
    // FLIGHT_COMPLETE decrements the sentinel.
    // Whoever reaches 0 last runs the summary — exactly once, no polling, no blocking.
    auto pending = std::make_shared<std::atomic<int>>(1);

    auto runSummary = [this, clientID]() {
        double total = m_dataStorage.sumConsumed(clientID);
        if (total >= 0.0) {
            cout << "[" + clientID + "] Total fuel consumed: " + to_string(total) + "\n";
        } else {
            cout << "[" + clientID + "] Flight complete (no consumption records found).\n";
        }
        std::lock_guard<std::mutex> lock(m_initialFuelMutex);
        m_initialFuel.erase(clientID);
    };

    while (true) {
        int bytesReceived = recv(ConnectionSocket, rxBuffer, sizeof(rxBuffer), 0);
        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                cout << ("[" + clientID + "] Client disconnected.\n");
            } else {
                cerr << ("[" + clientID + "] recv error: " + to_string(WSAGetLastError()) + "\n");
            }
            break;
        }

        // Check for flight completion message before feeding the parser
        string chunk(rxBuffer, bytesReceived);
        if (chunk.find("FLIGHT_COMPLETE") != string::npos) {
            cout << ("[" + clientID + "] Flight complete.\n");
            // Decrement the sentinel. If all telemetry tasks already finished, run summary now.
            if (--(*pending) == 0) {
                scheduler.enqueueTask(runSummary);
            }
            break;
        }

        parser.feed(rxBuffer, bytesReceived);

        TelemetryPacket packet;
        while (parser.tryParse(packet)) {
            ++(*pending); // claim a slot before posting
            scheduler.enqueueTask([this, packet, clientID, pending, runSummary]() {
                cout << "[" + clientID + "] ts=" + packet.timestamp + " fuel=" + to_string(packet.fuel) + "\n";

                // record initial fuel for this aircraft
                {
                    std::lock_guard<std::mutex> lock(m_initialFuelMutex);
                    m_initialFuel.emplace(clientID, packet.fuel); // no-op if already present
                }

                FuelConsumptionRecord record;
                if (m_telemetryProcessor.process(packet, record)) {
                    cout << "[" + clientID + "] consumed: " + to_string(record.fuelConsumed) + " liters"
                            " rate: " + to_string(record.consumptionRate) + " liters per second\n";
                    if (!m_dataStorage.insert(record)) {
                        cerr << "[" + clientID + "] Failed to persist fuel record.\n";
                    }
                }

                // Release this task's slot. If FLIGHT_COMPLETE already fired and we're
                // the last task to finish, run the summary.
                if (--(*pending) == 0) {
                    runSummary();
                }
            });
        }
    }

    cout << ("[" + clientID + "] Session ended.\n");
}

void ServerConnectionManager::stop() {
    isRunning = false;
    if (WelcomeSocket != INVALID_SOCKET) {
        closesocket(WelcomeSocket);
        WelcomeSocket = INVALID_SOCKET;
    }
    WSACleanup();
}