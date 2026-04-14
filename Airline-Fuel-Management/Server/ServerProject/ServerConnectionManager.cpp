#include "ServerConnectionManager.h"
#include <iostream>
#include <thread>
//#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <ctime>

#pragma warning(disable : 4996)

using namespace std;

struct FlightState {
    double initialFuel = -1;
    double lastFuel = -1;
    long long lastTime = -1;
};

static std::unordered_map<std::string, FlightState> flightStates;
static std::mutex flightStatesMutex;

ServerConnectionManager::ServerConnectionManager(int serverPort, TaskScheduler& scheduler) 
    : port(serverPort), WelcomeSocket(INVALID_SOCKET), isRunning(false), scheduler(scheduler), connectionPool(100) {}

ServerConnectionManager::~ServerConnectionManager() {
    stop();
}

void ServerConnectionManager::startListening() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return;
    }

    // create welcome socket
    if ((WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        cerr << "Failed to create WelcomeSocket." << endl;
        return;
    }

    // server struct
    struct sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET;
    SvrAddr.sin_addr.s_addr = INADDR_ANY; 
    SvrAddr.sin_port = htons(port); 

    // Add '::' before bind to use the global/WinSock version
    if ((::bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        closesocket(WelcomeSocket);
        WSACleanup();
        return;
    }

    // listen for new clients
    if (listen(WelcomeSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(WelcomeSocket);
        WSACleanup();
        return;
    }

    isRunning = true;
    cout << "Waiting for client connection on port " << port << "..." << endl;

    SOCKET ConnectionSocket = SOCKET_ERROR;

    while (isRunning) {
        // wait for incoming connection
        if ((ConnectionSocket = accept(WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
            if (isRunning) cerr << "Accept failed." << endl;
            continue;
        }

        cout << "Client connection made." << endl;

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
        cerr << "Handshake failed: No data received." << endl;
        return false;
    }

    string request(RxBuffer);

    // parse handshake message
    if (request.find("HELLO NEW") != string::npos) {    // if new client...
        clientID = to_string(airplaneCounter++);        // ...assign new ID
        cout << "Assigning new Aircraft ID: " << clientID << endl;
    }
    else if (request.find("HELLO ") != string::npos) {  // if existing client...
        clientID = request.substr(6);                   // ...extract existing ID

        // remove newline if found
        size_t pos = clientID.find('\n');
        if (pos != string::npos) {
            clientID.erase(pos);
        }
        cout << "Resuming flight for Aircraft ID: " << clientID << endl;
    }
    else {
        cerr << "Invalid handshake request." << endl;
        return false;
    }

    // send back assigned or existing ID
    string response = clientID + "\n";
    if (send(ConnectionSocket, response.c_str(), response.size(), 0) == SOCKET_ERROR) {
        cerr << "Failed to send handshake response." << endl;
        return false;
    }

    return true;
}

void ServerConnectionManager::handleClientSession(SOCKET ConnectionSocket, const string& clientID) {
    cout << "[" << clientID << "] Session started." << endl;

    ServerPacketParser parser;
    char rxBuffer[4096];

    while (true) {
        int bytesReceived = recv(ConnectionSocket, rxBuffer, sizeof(rxBuffer), 0);
        if (bytesReceived <= 0) {
            // Connection closed or error
            if (bytesReceived == 0) {
                cout << "[" << clientID << "] Client disconnected." << endl;
            } else {
                cerr << "[" << clientID << "] recv error: " << WSAGetLastError() << endl;
            }
            break;
        }

        // Check for flight completion message before feeding the parser
        string chunk(rxBuffer, bytesReceived);
        if (chunk.find("FLIGHT_COMPLETE") != string::npos) {
            cout << "[" << clientID << "] Flight complete." << endl;
            // post final average calculation to scheduler
            scheduler.enqueueTask([clientID]() {
                std::lock_guard<std::mutex> lock(flightStatesMutex);
                auto it = flightStates.find(clientID);
                if (it != flightStates.end()) {
                    double totalFuelConsumed = it->second.initialFuel - it->second.lastFuel;
                    std::ofstream outFile("telemetry_Client" + clientID + ".txt", std::ios::app);
                    if (outFile.is_open()) {
                        outFile << "FLIGHT_COMPLETE: Total Fuel Consumed: " << totalFuelConsumed << "\n";
                    }
                    cout << "[" << clientID << "] Final total fuel consumed: " << totalFuelConsumed << endl;
                    flightStates.erase(it);
                }
            });
            break;
        }

        parser.feed(rxBuffer, bytesReceived);

        TelemetryPacket packet;
        while (parser.tryParse(packet)) {

            // post telemetry processing to task scheduler
            scheduler.enqueueTask([packet, clientID]() {
                // telemetry log to console
                cout << "[" << clientID << "] "
                    << "ts=" << packet.timestamp
                    << " fuel=" << packet.fuel
                    << endl;
                    
                std::lock_guard<std::mutex> lock(flightStatesMutex);
                auto& state = flightStates[clientID];
                
                long long currentSeconds = 0;
                int m=0, d=0, y=0, h=0, min=0, s=0;
                if (sscanf(packet.timestamp.c_str(), "%d_%d_%d %d:%d:%d", &m, &d, &y, &h, &min, &s) == 6) {
                    struct tm tm = {0};
                    tm.tm_year = y - 1900;
                    tm.tm_mon = m - 1;
                    tm.tm_mday = d;
                    tm.tm_hour = h;
                    tm.tm_min = min;
                    tm.tm_sec = s;
                    currentSeconds = mktime(&tm);
                }

                if (state.initialFuel < 0) {
                    state.initialFuel = packet.fuel;
                    state.lastFuel = packet.fuel;
                    state.lastTime = currentSeconds;
                } else {
                    double fuelConsumed = state.lastFuel - packet.fuel; // per interval consumption
                    std::ofstream outFile("telemetry_Client" + clientID + ".txt", std::ios::app);
                    if (outFile.is_open()) {
                        outFile << "ts=" << packet.timestamp << " fuel_consumed=" << fuelConsumed << "\n";
                    }
                    state.lastFuel = packet.fuel;
                    state.lastTime = currentSeconds;
                }
            });
        }
    }

    cout << "[" << clientID << "] Session ended." << endl;
}

void ServerConnectionManager::stop() {
    isRunning = false;
    if (WelcomeSocket != INVALID_SOCKET) {
        closesocket(WelcomeSocket);
        WelcomeSocket = INVALID_SOCKET;
    }
    WSACleanup();
}