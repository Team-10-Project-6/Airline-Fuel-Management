#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "ServerConnectionManager.h"

using namespace std;

int main(int argc, char* argv[]) {

    // parse port number
    if (argc < 2) {
        cerr << "Usage: server <port>" << endl;
        return 1;
    }
    int port;
    try {
        port = stoi(argv[1]);
    } catch (const exception&) {
        cerr << "Invalid port number: " << argv[1] << endl;
        return 1;
    }
    if (port < 1 || port > 65535) {
        cerr << "Port must be between 1 and 65535." << endl;
        return 1;
    }

    cout << "Initializing server on port " << port << "..." << endl;

    // create connection manager
    ServerConnectionManager connectionManager(port);

    // start listening for client
    connectionManager.startListening();

    return 0;
}