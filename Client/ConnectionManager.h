#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>

class ConnectionManager {
public:
    //Construction/destruction
    ConnectionManager(const std::string& serverIp, int serverPort);

    ~ConnectionManager();

    bool connectToServer(const std::string& clientID);

    bool reconnect(int maxAttempts = 10);

    void disconnect();

    SOCKET getSocket() const;

    const std::string& getAircraftId() const;

private:
    
    SOCKET openSocket() const;

    bool doHandshake(SOCKET sock, const std::string& clientID, std::string& assignedId) const;

    std::string m_serverIp; //Server IP address
    int m_serverPort; //Server TCP port
    SOCKET m_socket; //Active socket handle
    std::string m_aircraftId; //Server assigned aircraft ID number
};