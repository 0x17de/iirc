#include "UserHandler.h"


using namespace std;


template<>
void UserHandler::onEvent<IrcEvent::Connect>(IrcClient& client) {
    for (auto channelData : databaseHandler.getAutoJoinChannels(client.getServerId())) {
        client.join(channelData.name.c_str(), channelData.key.empty() ? 0 : channelData.key.c_str());
    }
}

UserHandler::UserHandler(const UserData& userData, DatabaseHandler& databaseHandler) : userData(userData), databaseHandler(databaseHandler) {

}

IrcClient* UserHandler::connect(const ServerData& serverData) {
    auto it = ircClients.find(serverData.serverId);
    if (it != ircClients.end()) return &it->second; // already connected

    auto jt = ircClients.emplace(piecewise_construct, forward_as_tuple(serverData.serverId), forward_as_tuple(*this, serverData));
    IrcClient &ircClient = jt.first->second;
    ircClient.connect();

    return &ircClient;
}

const UserData &UserHandler::getUserData() {
    return userData;
}

IrcClient& UserHandler::get(size_t serverId) {
    return ircClients.at(serverId);
}
