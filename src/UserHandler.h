#ifndef IIRC_USERHANDLER_H
#define IIRC_USERHANDLER_H


#include <stddef.h>
#include <unordered_map>
#include "IrcClient.h"
#include "IrcEvent.h"
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
    void connect(const ServerData& serverData);
    void disconnect();
    IrcClient& get(size_t serverId);

    template<IrcEvent>
    void onEvent(IrcClient& client);
};


#endif //IIRC_USERHANDLER_H
