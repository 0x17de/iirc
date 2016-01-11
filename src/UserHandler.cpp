#include "UserHandler.h"
#include <iostream>
#include <sstream>


using namespace std;


template<>
void UserHandler::onEvent<IrcEvent::Connect>(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count) {
    for (auto channelData : databaseHandler.getAutoJoinChannels(client.getServerId())) {
        client.join(channelData.name.c_str(), channelData.key.empty() ? 0 : channelData.key.c_str());
    }
}
template<>
void UserHandler::onEvent<IrcEvent::PrivMsg>(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count) {
    if (count < 2) return;

    string senderName(splitNickFromIdentifier(origin));
    size_t senderId = databaseHandler.getOrCreateSenderId(senderName);
    string channel(params[0]);
    size_t channelId = client.getChannelId(channel);
    string message(params[1]);

    if (channelId == 0) {
        cerr << "Could not query channel id. Message not saved into database." << endl;
        return;
    }

    databaseHandler.storeMessage(senderId, channelId, message);

    // TODO: save to database
}
template<>
void UserHandler::onEvent<IrcEvent::Channel>(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count) {
    onEvent<IrcEvent::PrivMsg>(client, event, origin, params, count);
}
template<>
void UserHandler::onEvent<IrcEvent::Join>(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count) {

}
template<>
void UserHandler::onEvent<IrcEvent::Part>(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count) {

}
template<>
void UserHandler::onEvent<IrcEvent::Quit>(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count) {

}

std::string UserHandler::splitNickFromIdentifier(const std::string identifier) {
    string nick;
    // extract sender name from identifier
    auto senderExclamationPos = identifier.find('!');
    if (senderExclamationPos == string::npos) {
        nick = identifier;
    } else {
        nick = identifier.substr(0, senderExclamationPos);
    }
    return nick;
}

UserHandler::UserHandler(const UserData& userData, DatabaseHandler& databaseHandler) : userData(userData), databaseHandler(databaseHandler) {
}

bool UserHandler::connect(const ServerData& serverData) {
    auto it = ircClients.find(serverData.serverId);
    if (it != ircClients.end()) return true; // already connected

    auto jt = ircClients.emplace(piecewise_construct, forward_as_tuple(serverData.serverId),
                                 forward_as_tuple(*this, serverData));
    IrcClient &ircClient = jt.first->second;
    if (!ircClient.connect()) {
        cerr << "Error connecting to IRC server." << endl;
        return false;
    }
    return true;
}

const UserData &UserHandler::getUserData() {
    return userData;
}

DatabaseHandler& UserHandler::getDatabaseHandler() {
    return databaseHandler;
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
