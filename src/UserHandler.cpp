#include "UserHandler.h"


using namespace std;



UserHandler::UserHandler(const UserData& userData) : userData(userData) {

}

bool UserHandler::connect(const ServerData& serverData) {
    auto it = ircClients.find(serverData.serverId);
    if (it != ircClients.end()) return true; // already connected

    auto jt = ircClients.emplace(serverData.serverId, serverData);
    IrcClient &ircClient = jt.first->second;
    ircClient.connect();

    return false;
}
