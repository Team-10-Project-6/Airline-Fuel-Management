#include <iostream>
#include <string>
#include "ServerConnectionManager.h"

using namespace std;

int main(int argc, char* argv[]) {

    // parse port number
    int port = stoi(argv[1]);

    cout << "Initializing server on port " << port << "..." << endl;

    // create connection manager
    ServerConnectionManager connectionManager(port);

    // start listening for client
    connectionManager.startListening();

    return 0;
}