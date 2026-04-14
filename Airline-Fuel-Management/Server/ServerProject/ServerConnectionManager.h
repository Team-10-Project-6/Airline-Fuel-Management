#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include "ServerPacketParser.h"

class ServerConnectionManager {
public:
    ServerConnectionManager(int port);
    ~ServerConnectionManager();

    void startListening();
    void stop();

private:
    int port;
    SOCKET WelcomeSocket;
    bool isRunning;

    // counter for airplane id
    int airplaneCounter;

    // Performs the HELLO handshake; sets clientID and returns true on success
    bool handleHandshake(SOCKET ConnectionSocket, std::string& clientID);

    // Receives telemetry packets for an already-handshaked connection
    void handleClientSession(SOCKET ConnectionSocket, const std::string& clientID);
};