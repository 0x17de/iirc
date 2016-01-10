#ifndef IIRC_USERHANDLER_H
#define IIRC_USERHANDLER_H


#include <stddef.h>
#include <unordered_map>
#include "IrcClient.h"
#include "data/UserData.h"
#include "data/ServerData.h"


class UserHandler {
    UserData userData;
    std::unordered_map<size_t, IrcClient> ircClients;

public:
    UserHandler(const UserData& userData);
    const UserData& getUserData();
    bool connect(const ServerData& serverData);
};


#endif //IIRC_USERHANDLER_H
