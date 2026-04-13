#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    sockaddr_in serv_addr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }

    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <Server_IP> <Port>" << std::endl;
        WSACleanup();
        return -1;
    }

    // Parse command line arguments for server IP and port
    try {
        string server_ip = argv[1];
        int server_port = stoi(argv[2]);

        SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ClientSocket == SOCKET_ERROR || ClientSocket == INVALID_SOCKET) {
            cerr << "Problem creating socket." << endl;
            WSACleanup();
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(server_port);
        serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

        // Establish a connection to the server
        cout << "Trying to connect to " << server_ip << "..." << endl;
        if (connect(ClientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            cerr << "Connection Failed. Error: " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            WSACleanup();
            return -1;
        }

        cout << "Successfully connected to the server!" << endl;

        // TODO: Implement communication with the server (ID aquisition and Telemtry data parsing/sending)

        // Cleanup
        closesocket(ClientSocket);
        WSACleanup();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        WSACleanup();
        return -1;
    }
}