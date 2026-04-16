/**
 * @file ConnectionManager.h
 * @brief Manages the TCP socket connection to the server.
 */
#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define GET_SOCKET_ERR() WSAGetLastError()
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    // Create common types
    typedef int SOCKET;
    #define INVALID_SOCKET  (SOCKET)(~0)
    #define SOCKET_ERROR            (-1)
    #define closesocket(s) close(s)
    #define GET_SOCKET_ERR() errno
    #define SD_SEND SHUT_WR
#endif

#include <string>

/**
 * @class ConnectionManager
 * @brief Handles establishing, maintaining, and tearing down the TCP connection to the telemetry server.
 */
class ConnectionManager {
public:
    //Construction/destruction
    /**
     * @brief Constructs a new ConnectionManager object.
     * @param serverIp The IP address of the telemetry server.
     * @param serverPort The TCP port to connect to.
     */
    ConnectionManager(const std::string& serverIp, int serverPort);

    /**
     * @brief Destructor that ensures the connection is properly closed.
     */
    ~ConnectionManager();

    /**
     * @brief Connects to the server and performs the initial handshake.
     * @param clientID The requested client ID string.
     * @return true if connection and handshake succeed, false otherwise.
     */
    bool connectToServer(const std::string& clientID);

    /**
     * @brief Attempts to reconnect to the server if the connection is lost.
     * @param maxAttempts Maximum number of reconnect attempts before giving up.
     * @return true if successfully reconnected, false otherwise.
     */
    bool reconnect(int maxAttempts = 10);

    /**
     * @brief Gracefully disconnects from the server and closes the socket.
     */
    void disconnect();

    /**
     * @brief Gets the active socket handle.
     * @return SOCKET The active socket handle.
     */
    SOCKET getSocket() const;

    /**
     * @brief Gets the vehicle/aircraft ID assigned by the server.
     * @return const std::string& The assigned aircraft ID.
     */
    const std::string& getAircraftId() const;

private:
    
    SOCKET openSocket() const;

    bool doHandshake(SOCKET sock, const std::string& clientID, std::string& assignedId) const;

    std::string m_serverIp; //Server IP address
    int m_serverPort; //Server TCP port
    SOCKET m_socket; //Active socket handle
    std::string m_aircraftId; //Server assigned aircraft ID number
};