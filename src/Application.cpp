#include "Application.h"
#include <iostream>
#include <list>
#include <csignal>
#include <unistd.h>
#include <thread>
#include <map>
#include <vector>
#include "IniReader.h"
#include "DatabaseHandler.h"
#include "UserHandler.h"
#include "tcp/TcpInterface.h"


using namespace std;



static sig_atomic_t running = true;

static void sigint(int) {
    running = false;

    static int count = 0;
    ++count;
    if (count >= 3)
        exit(1);
}
static void sigterm(int) {
    if (running == false)
        exit(1);
    running = false;
}

int Application::run() {
    signal(SIGINT, sigint);
    signal(SIGTERM, sigterm);

    // Parse configuration
    IniReader iniReader;
    if (!iniReader.parse("config.ini")) {
        cerr << "Could not parse configuration." << endl;
        return 1;
    }

    // Connect to database
    DatabaseHandler databaseHandler;
    databaseHandler.configure(iniReader);
    if (!databaseHandler.connect()) {
        cerr << "Could not connect to database." << endl;
        return 2;
    }

    // Initialize database tables (once)
    if (!databaseHandler.initializeDatabase()) {
        cerr << "Error during initialization of database tables." << endl;
        return 3;
    }

    // Initialize users
    std::map<size_t, UserHandler> userHandlers;
    for (auto userData : databaseHandler.getUserData()) {
        auto it = userHandlers.emplace(piecewise_construct, forward_as_tuple(userData.userId), forward_as_tuple(userData, databaseHandler));
        UserHandler& userHandler = it.first->second;

        for (auto serverData : databaseHandler.getAutoConnectServers(userData.userId))
            userHandler.connect(serverData);
    }

    TcpInterface tcpInterface;

    tcpInterface.onHeader([](const iirc::Header& header, UserHandler* client){
        // TODO: validate header for plausibility
        // TODO: true: accept header data - otherwise disconnect client.
        return true;
    });

    tcpInterface.onData([](const vector<uint8_t>& data, UserHandler* client){
        // TODO: after valid login:
        //tcpInterface.setUserHandler(...);
        //userHandler.addTcpClient(...);
        // TODO: deserialize data and notify responsible userHandler
        return true;
    });

    tcpInterface.onClose([](UserHandler* client){
        //userHandler.removeTcpClient(...);
    });

    thread tcpThread([&] {
        try {
            tcpInterface.run();
        }
        catch(exception& e) {
            cerr << "(TcpInterface)Exception: " << e.what() << endl;
        }
        cerr << "TcpThread exit." << endl;
    });

    while (running) sleep(1); // IDLE

    tcpInterface.stop();
    tcpThread.join();


    return 0;
}