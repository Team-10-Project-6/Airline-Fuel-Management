/**
 * @file main.cpp
 * @brief Entry point for the Airline Fuel Management Server application.
 */
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "ServerConnectionManager.h"

using namespace std;

/**
 * @brief Main execution function of the server binary.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments (expects port number). 
 * @return int Exit status code.
 */
int main(int argc, char* argv[]) {

    // parse port number
    if (argc < 2) {
        cerr << "Usage: server <port>\n";
        return 1;
    }
    int port;
    try {
        port = stoi(argv[1]);
    } catch (const exception&) {
        cerr << ("Invalid port number: " + string(argv[1]) + "\n");
        return 1;
    }
    if (port < 1 || port > 65535) {
        cerr << "Port must be between 1 and 65535.\n";
        return 1;
    }

    cout << ("Initializing server on port " + to_string(port) + "...\n");

    // initialize task scheduler
    TaskScheduler scheduler(8);

    // create connection manager
    ServerConnectionManager connectionManager(port, scheduler);

    // start listening for client
    connectionManager.startListening();

    return 0;
}