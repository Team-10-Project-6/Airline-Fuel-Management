#include <iostream>
#include <string>
#include "ConnectionManager.h"
#include "TelemetryParser.h"
#include "PacketHandler.h"

using namespace std;

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Initialise Winsock on Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] WSAStartup failed." << endl;
        return -1;
    }
#endif

    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <Server_IP> <Port> <Telemetry_File> [Aircraft_ID]" << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return -1;
    }

    try {
        const string serverIp   = argv[1];
        const int serverPort = stoi(argv[2]);
        const string filePath   = argv[3];

        //Pass "NEW" for an anonymous first-time connection, or the server provided ID number to resume an existing flight
        const string clientID = (argc >= 5) ? string(argv[4]) : "NEW";

        //Handshake & connection setup
        ConnectionManager connManager(serverIp, serverPort);
        if (!connManager.connectToServer(clientID)) {
#ifdef _WIN32
            WSACleanup();
#endif
            return -1;
        }

		//Open telemetry file.
        TelemetryParser parser(filePath);
        if (!parser.open()) {
            cerr << "[ERROR] Failed to open telemetry file." << endl;
            connManager.disconnect();
#ifdef _WIN32
            WSACleanup();   
#endif
            return -1;
        }

        //Create a PacketHandler and start sending lines until end of file or an error occurs
        PacketHandler packetHandler(connManager);
        while (parser.hasMore()) {
            string line;
            if (parser.nextLine(line)) {
                //cout << "DEBUG: Sending line: " << line << " (Line " << parser.lineNumber() << ")" << endl;
                if (!packetHandler.sendLine(line, parser.lineNumber())) {
                    cerr << "[ERROR] Failed to send line " << parser.lineNumber() << ". Aborting." << endl;
                    connManager.disconnect();
#ifdef _WIN32
                    WSACleanup();
#endif
                    return -1;
                }
            }
        }

        //Flight completion
        cout << "[INFO] Flight complete for aircraft: " << connManager.getAircraftId() << endl;

        // //Must be a better way to ensure all packets are sent before disconnecting
        // Sleep(10000); // Sleep for 1 second 

        connManager.disconnect();

    } catch (const exception& e) {
        cerr << "[ERROR] " << e.what() << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return -1;
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}