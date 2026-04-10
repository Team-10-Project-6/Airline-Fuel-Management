#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ConnectionManager.h"
#include <iostream>
using namespace std;

//Construction / destruction
ConnectionManager::ConnectionManager(const string& serverIp, int serverPort)
    : m_serverIp(serverIp), m_serverPort(serverPort), m_socket(INVALID_SOCKET) {}

ConnectionManager::~ConnectionManager() {
    disconnect();
}

//Establishe a new connection and perform the handshake, return true on success
bool ConnectionManager::connectToServer(const string& clientID) {
    m_socket = openSocket();
    if (m_socket == INVALID_SOCKET) {
        return false;
    }

    if (!doHandshake(m_socket, clientID, m_aircraftId)) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }
    return true;
}

//Attempt to reconnect by closing the existing socket and opening a new one, 
//performing the handshake again to get a new/resumed aircraft ID, return true on success,
//maxAttempts specifies how many times to retry before giving up
bool ConnectionManager::reconnect(int maxAttempts) {
    disconnect();

    cout << "[INFO] Attempting to reconnect..." << endl;

    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        if (connectToServer(m_aircraftId)) {
            cout << "[INFO] Reconnected on attempt " << attempt << "." << endl;
            return true;
        }
        // Sleep(1000); // Optionally wait before retrying
    }

    cerr << "[ERROR] Could not reconnect after " << maxAttempts << " attempts." << endl;
    return false;
}

void ConnectionManager::disconnect() {
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

SOCKET ConnectionManager::getSocket() const {
    return m_socket;
}

const string& ConnectionManager::getAircraftId() const {
    return m_aircraftId;
}

//Open a new socket and connect to the server, return the socket handle or INVALID_SOCKET on failure
SOCKET ConnectionManager::openSocket() const {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cerr << "[ERROR] Failed to create socket. Error: " << GET_SOCKET_ERR() << endl;
        return INVALID_SOCKET;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(m_serverPort));
    addr.sin_addr.s_addr = inet_addr(m_serverIp.c_str());

    cout << "[INFO] Connecting to " << m_serverIp << ":" << m_serverPort << " ..." << endl;

    if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        cerr << "[ERROR] Connection failed. Error: " << GET_SOCKET_ERR() << endl;
        closesocket(sock);
        return INVALID_SOCKET;
    }

    cout << "[INFO] Connected." << endl;
    return sock;
}

//Perform the handshake protocol; i.e. send a greeting and receive an ID, return true on success and populate assignedId
bool ConnectionManager::doHandshake(SOCKET sock, const string& clientID, string& assignedId) const {
    // Send "HELLO <clientID>\n"
    string greeting = "HELLO " + clientID + "\n";
    if (send(sock, greeting.c_str(), static_cast<int>(greeting.size()), 0) == SOCKET_ERROR) {
        cerr << "[ERROR] Handshake send failed. Error: " << GET_SOCKET_ERR() << endl;
        return false;
    }

    // Read the assigned ID from the server
    char buf[256] = {};
    int  bytes    = recv(sock, buf, sizeof(buf) - 1, 0);
    if (bytes <= 0) {
        cerr << "[ERROR] Handshake recv failed. Error: " << GET_SOCKET_ERR() << endl;
        return false;
    }

    //Remove any trailing newline characters from the response
    string response(buf, bytes);
    while (!response.empty() && (response.back() == '\n' || response.back() == '\r')) {
        response.pop_back();
    }

    //Expect a non-empty response containing the assigned aircraft ID
    if (!response.empty()) {
        assignedId = response;
        cout << "[INFO] Server assigned Aircraft ID: " << assignedId << endl;
        return true;
    }

    cerr << "[ERROR] Empty handshake response from server." << endl;
    return false;
}
