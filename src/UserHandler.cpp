#include "UserHandler.h"
#include <iostream>


using namespace std;


template<>
void UserHandler::onEvent<IrcEvent::Connect>(IrcClient& client) {
    for (auto channelData : databaseHandler.getAutoJoinChannels(client.getServerId())) {
        client.join(channelData.name.c_str(), channelData.key.empty() ? 0 : channelData.key.c_str());
    }
}

UserHandler::UserHandler(const UserData& userData, DatabaseHandler& databaseHandler) : userData(userData), databaseHandler(databaseHandler) {

}

void UserHandler::connect(const ServerData& serverData) {
    auto it = ircClients.find(serverData.serverId);
    if (it != ircClients.end()) return; // already connected

    auto jt = ircClients.emplace(piecewise_construct, forward_as_tuple(serverData.serverId),
                                 forward_as_tuple(*this, serverData));
    IrcClient &ircClient = jt.first->second;
    if (ircClient.connect() != 0) {
        cerr << "Error connecting to IRC server." << endl;
        return;
    }
}

const UserData &UserHandler::getUserData() {
    return userData;
}

IrcClient& UserHandler::get(size_t serverId) {
    return ircClients.at(serverId);
}

void UserHandler::disconnect() {
    for (auto p : ircClients) {
        IrcClient& client = p.second;
        client.disconnect();
    }
}
