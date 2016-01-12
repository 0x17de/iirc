#include "Application.h"
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <thread>
#include "tcp/TcpClient.h"
#include "common.pb.h"
#include "server.pb.h"
#include "client.pb.h"

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
    if (!iniReader.parse("config.ini")) {
        cerr << "Could not parse configuration." << endl;
        return 1;
    }

    // Connect to database
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
    for (auto userData : databaseHandler.getUserData()) {
        auto it = userHandlers.emplace(piecewise_construct, forward_as_tuple(userData.userId), forward_as_tuple(userData, databaseHandler));
        UserHandler& userHandler = it.first->second;

        for (auto serverData : databaseHandler.getAutoConnectServers(userData.userId))
            userHandler.connect(serverData);
    }

    // Register callbacks for clients
    tcpInterface.onHeader([](const iircCommon::Header& header, UserHandler** client){
        // TODO: validate header for plausibility
        // TODO: true: accept header data - otherwise disconnect client.
        return true;
    });

    tcpInterface.onData([this](const iircCommon::Header& header, const vector<uint8_t>& data, TcpClient* client, UserHandler** userHandler){
        if (header.type() == iircCommon::Type::Login) {
            iircClient::Login login;
            if (!login.ParseFromArray((char*)data.data(), header.length()))
                return false; // stop if could not parse
            cerr << "LOGIN " << login.username() << endl;
            size_t userId = databaseHandler.getUserFromLogin(login.username(), login.password());
            if (userId == 0) return false;

            iircCommon::Header header;
            iircServer::LoginResult loginResult;
            loginResult.set_success(true);
            header.set_type(iircCommon::Type::LoginResult);
            header.set_length(loginResult.ByteSize());

            ostream os(client);
            header.SerializeToOstream(&os);
            loginResult.SerializeToOstream(&os);
            os.flush();
        }

        // TODO: after valid login:
        //tcpInterface.setUserHandler(...);
        //userHandler.addTcpClient(...);
        // TODO: deserialize data and notify responsible userHandler
        return true;
    });

    tcpInterface.onClose([](UserHandler** client){
        //userHandler.removeTcpClient(...);
    });

    // Start tcp server for clients
    thread tcpThread([&] {
        try {
            tcpInterface.run();
        }
        catch(exception& e) {
            cerr << "(TcpInterface)Exception: " << e.what() << endl;
        }
        cerr << "TcpThread exit." << endl;
    });

    // Idle
    while (running) sleep(1);

    // Cleanup
    tcpInterface.stop();
    tcpThread.join();

    return 0;
}