#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>

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

    // function for handling handshake
    void handleHandshake(SOCKET ConnectionSocket);
};