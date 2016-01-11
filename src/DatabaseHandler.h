#ifndef IIRC_DATABASEHANDLER_H
#define IIRC_DATABASEHANDLER_H


#include <memory>
#include <string>
#include <list>
#include "data/UserData.h"
#include "data/ServerData.h"
#include "data/ChannelData.h"


namespace soci {
    class session;
}
class IniReader;

class DatabaseHandler {
    std::shared_ptr<soci::session> sqlSession;

    std::string type;
    std::string path; // sqlite3
    std::string host;
    std::string port;
    std::string username;
    std::string password;
    std::string database;
    std::string tableprefix;

#define GENTABLESTRING(name) \
        std::string name ## TableName;
    GENTABLESTRING(config);
    GENTABLESTRING(backlog);
    GENTABLESTRING(channel);
    GENTABLESTRING(server);
    GENTABLESTRING(aliasset);
    GENTABLESTRING(alias);
    GENTABLESTRING(user);
    GENTABLESTRING(nick);
#undef GENTABLESTRING

public:
    bool configure(IniReader& iniReader);
    bool connect();
    bool initializeDatabase();
    bool upgradeDatabase(std::string fromVersion, std::string toVersion);

    std::list<UserData> getUserData();
    std::list<ServerData> getAutoConnectServers(size_t userId);
    std::list<ChannelData> getAutoJoinChannels(size_t serverId);
};


#endif //IIRC_DATABASEHANDLER_H
