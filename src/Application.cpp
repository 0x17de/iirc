#include "Application.h"
#include <iostream>
#include <list>
#include <unistd.h>
#include "IniReader.h"
#include "DatabaseHandler.h"
#include "UserHandler.h"


using namespace std;



int Application::run() {
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
    std::unordered_map<size_t, UserHandler> userHandlers;
    for (auto userData : databaseHandler.getUserData()) {
        auto it = userHandlers.emplace(piecewise_construct, forward_as_tuple(userData.userId), forward_as_tuple(userData, databaseHandler));
        UserHandler& userHandler = it.first->second;

        for (auto serverData : databaseHandler.getAutoConnectServers(userData.userId)) {
            IrcClient* client = userHandler.connect(serverData);
            if (!client) continue;
        }
    }

    for (int i = 0; i < 5; ++i) {
        sleep(1);
    }
    // Read IRC configuration from database

    // Autologin to configured servers&channels

    // Start websocket server

    // IDLE
    return 0;
}