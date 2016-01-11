#ifndef IIRC_USERHANDLER_H
#define IIRC_USERHANDLER_H


#include <stddef.h>
#include <unordered_map>
#include "irc/IrcClient.h"
#include "irc/IrcEvent.h"
#include "data/UserData.h"
#include "data/ServerData.h"
#include "DatabaseHandler.h"


class UserHandler {
    UserData userData;
    DatabaseHandler& databaseHandler;
    std::unordered_map<size_t, IrcClient> ircClients;

public:
    UserHandler(const UserData& userData, DatabaseHandler& databaseHandler);
    const UserData& getUserData();
    DatabaseHandler& getDatabaseHandler();
    bool connect(const ServerData& serverData);
    void disconnect();
    IrcClient& get(size_t serverId);

    static std::string splitNickFromIdentifier(const std::string identifier);

    template<IrcEvent>
    void onEvent(IrcClient& client, const char *event, const char *origin, const char **params, unsigned int count);
};


#endif //IIRC_USERHANDLER_H
