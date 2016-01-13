#ifndef IIRC_USERHANDLER_H
#define IIRC_USERHANDLER_H


#include <stddef.h>
#include <unordered_map>
#include "tcp/TcpClient.h"
#include "irc/IrcClient.h"
#include "irc/IrcEvent.h"
#include "data/UserData.h"
#include "data/ServerData.h"
#include "DatabaseHandler.h"
#include "common.pb.h"
#include "server.pb.h"



class UserHandler {
    UserData userData;
    DatabaseHandler& databaseHandler;
    std::unordered_map<size_t, IrcClient> ircClientByServer;
    std::list<std::weak_ptr<TcpClient>> tcpClients;

public:
    UserHandler(const UserData& userData, DatabaseHandler& databaseHandler);
    const UserData& getUserData();
    DatabaseHandler& getDatabaseHandler();
    bool connect(const ServerData& serverData);
    void disconnect();
    IrcClient& get(size_t serverId);
    const std::unordered_map<size_t, IrcClient>& getServerList();

    static std::string splitNickFromIdentifier(const std::string identifier);

    template <IrcEvent>
    void onEvent(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count);
    void onNumericEvent(IrcClient& client, unsigned int event, const char *origin, const char **params, unsigned int count);

    void send(iircCommon::DataType type, ::google::protobuf::Message& message);

    void addTcpClient(std::weak_ptr<TcpClient> tcpClient);
    void removeTcpClient(TcpClient* tcpClient);
    void removeTcpClient(std::weak_ptr<TcpClient> tcpClient);
};


#endif //IIRC_USERHANDLER_H
