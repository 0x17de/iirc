#include "UserHandler.h"
#include <iostream>
#include <cstdint>
#include <libirc_rfcnumeric.h>
#include "server.pb.h"


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
    size_t channelId = client.getChannelData(channel).channelId;
    string message(params[1]);

    if (channelId == 0) {
        cerr << "Could not query channel id. Message not saved into database." << endl;
        return;
    }

    size_t messageId = databaseHandler.storeMessage(senderId, channelId, message);

    if (tcpClients.size() == 0) return; // only prepare data if clients connected
    iircServer::BacklogNotification backlogNotification;
    backlogNotification.set_serverid(client.getServerId());
    auto channelBacklog = backlogNotification.add_channelbacklog();
    channelBacklog->set_channelid(channelId);
    auto backlog = channelBacklog->add_backlog();
    backlog->set_messageid(messageId);
    backlog->set_timestamp(0);
    backlog->set_nick(senderName);
    backlog->set_message(message);
    send(iircCommon::DataType::BacklogNotification, backlogNotification);
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

void UserHandler::onNumericEvent(IrcClient& client, unsigned int event, const char *origin, const char **params, unsigned int count) {
	if (event == LIBIRC_RFC_RPL_NAMREPLY && count >= 4 /* userlist */) {
		iircServer::UserList userList;
		size_t channelId = client.getChannelData(params[2]).channelId;
		userList.set_serverid(client.getServerId());
		userList.set_channelid(channelId);

		istringstream is(params[3]);
		for (string nick : client.getChannelData(channelId).userList) {
			auto user = userList.add_users();
			user->set_nick(nick);
		}
		send(iircCommon::DataType::UserList, userList);
	}
}


void UserHandler::send(iircCommon::DataType type, ::google::protobuf::Message& message) {
    uint16_t newType = type;
    uint64_t dataSize = message.ByteSize();

    for (auto tcpClientWeak : tcpClients) {
        auto tcpClient = tcpClientWeak.lock();
        if (!tcpClient) {
            removeTcpClient(tcpClientWeak);
            continue;
        }
        tcpClient->send(type, message);
    }
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
    auto it = ircClientByServer.find(serverData.serverId);
    if (it != ircClientByServer.end()) return true; // already connected

    auto jt = ircClientByServer.emplace(piecewise_construct, forward_as_tuple(serverData.serverId),
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
    return ircClientByServer.at(serverId);
}

const std::unordered_map<size_t, IrcClient>& UserHandler::getServerList() {
	return ircClientByServer;
}

void UserHandler::disconnect() {
    for (auto p : ircClientByServer) {
        IrcClient& client = p.second;
        client.disconnect();
    }
}

void UserHandler::addTcpClient(std::weak_ptr<TcpClient> tcpClient) {
    tcpClients.push_back(tcpClient);
}

void UserHandler::removeTcpClient(std::weak_ptr<TcpClient> tcpClientWeak) {
    auto tcpClient = tcpClientWeak.lock();
    removeTcpClient(tcpClient ? tcpClient.get() : 0);
}

void UserHandler::removeTcpClient(TcpClient* tcpClient) {
    tcpClients.remove_if([&](std::weak_ptr<TcpClient> savedClientWeak) {
        if (auto savedClient = savedClientWeak.lock())
            return !savedClient || savedClient.get() == tcpClient;
    });
}
