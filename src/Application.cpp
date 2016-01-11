#include "Application.h"
#include <iostream>
#include <list>
#include <csignal>
#include "IniReader.h"
#include "DatabaseHandler.h"
#include "UserHandler.h"


using namespace std;


static sig_atomic_t running = true;

static void sigint(int) {
    running = false;
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
    std::unordered_map<size_t, UserHandler> userHandlers;
    for (auto userData : databaseHandler.getUserData()) {
        auto it = userHandlers.emplace(piecewise_construct, forward_as_tuple(userData.userId), forward_as_tuple(userData, databaseHandler));
        UserHandler& userHandler = it.first->second;

        for (auto serverData : databaseHandler.getAutoConnectServers(userData.userId))
            userHandler.connect(serverData);
    }

    while(running) {}

    // Read IRC configuration from database

    // Autologin to configured servers&channels

    // Start websocket server

    // IDLE



    return 0;
}