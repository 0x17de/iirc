#include "UserHandler.h"


using namespace std;



UserHandler::UserHandler(const UserData& userData) : userData(userData) {

}

bool UserHandler::connect(const ServerData& serverData) {
    auto it = ircClients.find(serverData.serverId);
    if (it != ircClients.end()) return true; // already connected

    auto jt = ircClients.emplace(piecewise_construct, forward_as_tuple(serverData.serverId), forward_as_tuple(*this, serverData));
    IrcClient &ircClient = jt.first->second;
    ircClient.connect();

    return false;
}

const UserData &UserHandler::getUserData() {
    return userData;
}
