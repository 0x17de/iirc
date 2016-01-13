#include "Application.h"
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <thread>
#include "tcp/TcpClient.h"
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
    tcpInterface.onHeader([](iircCommon::DataType dataType, uint64_t dataSize, UserHandler** client){
        // TODO: validate header for plausibility
        // TODO: true: accept header data - otherwise disconnect client.
        return true;
    });

    tcpInterface.onData([this](iircCommon::DataType dataType, const vector<uint8_t>& data, std::shared_ptr<TcpClient> client, UserHandler** userHandler){
        if (*userHandler == 0) { // not logged in
            if (dataType == iircCommon::DataType::Login) {
                iircClient::Login login;
                if (!login.ParseFromArray((char *) data.data(), data.size()))
                    return false; // stop if could not parse

                cerr << "LOGIN " << login.username() << endl;
                size_t userId = databaseHandler.getUserFromLogin(login.username(), login.password());
                if (userId == 0) return false;

                try {
                    *userHandler = &userHandlers.at(userId);
                }
                catch (out_of_range &e) {
                    return false; // TODO: in database, but not loaded currently
                }
                (*userHandler)->addTcpClient(client);

                iircServer::LoginResult loginResult;
                loginResult.set_success(true);
                uint64_t loginSize = loginResult.ByteSize();

                ostream os(client.get());
                const uint16_t newDataType = iircCommon::DataType::LoginResult;
                os.write((char *) &newDataType, sizeof(uint16_t));
                os.write((char *) &loginSize, sizeof(uint64_t));
                loginResult.SerializeToOstream(&os);
                os.flush();
            }
        }
        else { // logged in
            if (dataType == iircCommon::DataType::ChatMessage) {
                iircClient::ChatMessage chatMessage;
                if (!chatMessage.ParseFromArray((char*)data.data(), data.size()))
                    return false;

                cerr << "CHAT " << chatMessage.message() << endl;

				try {
	                IrcClient& ircClient = (*userHandler)->get(chatMessage.serverid());
	                string channelName = ircClient.getChannelData(chatMessage.channelid()).name;
					cerr << "CHANNEL: " << channelName << endl;
					ircClient.send(channelName, chatMessage.message());

					size_t senderId = databaseHandler.getOrCreateSenderId("iircUser"); // TODO: replace with currently active nickname
					databaseHandler.storeMessage(senderId, chatMessage.channelid(), chatMessage.message());
				}
				catch(out_of_range& e) {
					cerr << "ChatMessage: " << e.what() << endl;
					return false;
				}
            }
        }

        return true;
    });

    tcpInterface.onClose([](TcpClient* client, UserHandler** userHandler) {
        if (*userHandler)
            (*userHandler)->removeTcpClient(client);
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
