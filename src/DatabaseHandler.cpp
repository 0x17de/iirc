#include "DatabaseHandler.h"
#include <iostream>
#include <cstring>
#include "IniReader.h"
#include "soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"

#include "config/db.h"
#include "data/ChannelData.h"


using namespace std;



bool DatabaseHandler::configure(IniReader &iniReader) {
    IniSection databaseSection = iniReader.get("database");
    if (!databaseSection) {
        cerr << "Section 'database' requested from configuration, but not found." << endl;
        return false;
    }

#define READCONF(name) \
        name = databaseSection.get(#name)
    READCONF(type);
    READCONF(path);
    READCONF(host);
    READCONF(port);
    READCONF(username);
    READCONF(password);
    READCONF(database);
    READCONF(tableprefix);
#undef READCONF

#define GENTABLESTRING(name) \
        name ## TableName = tableprefix + #name;
    GENTABLESTRING(config);
    GENTABLESTRING(backlog);
    GENTABLESTRING(channel);
    GENTABLESTRING(server);
    GENTABLESTRING(aliasset);
    GENTABLESTRING(alias);
    GENTABLESTRING(user);
    GENTABLESTRING(nick);
#undef GENTABLESTRING

    const char allowedTableprefixChars[] = "_-";
    for (char c : tableprefix) {
        if (c >= 'a' && c <= 'z') continue;
        if (c >= 'A' && c <= 'Z') continue;
        if (c >= '0' && c <= '9') continue;
        if (strchr(allowedTableprefixChars, c) != nullptr) continue;
        cerr << "Invalid chars in tableprefix configuration. Allowed chars: [a-zA-Z0-9_-]" << endl;
        return false;
    }
}

bool DatabaseHandler::connect() {
    using namespace soci;

    stringstream ss;
    if (type == "postgres") {
        ss << "postgresql://"
        << "dbname=" << database << " "
        << "user=" << username << " "
        << "password=" << password;
    }
    else {
        cerr << "Unknown database type configured: '" << type << "'" << endl;
        return false;
    }

    try {
        sqlSession = make_shared<session>(ss.str());
    }
    catch(soci_error& e) {
        cerr << "Could not connect to server. Reason:" << endl << e.what() << endl << endl;
        return false;
    }

    return true;
}

bool DatabaseHandler::upgradeDatabase(std::string fromVersion, std::string toVersion) {
    using namespace soci;

#define UPGRADE(fromV, toV) \
    if (fromVersion == fromV) { \
        cerr << "Upgrade from '" << fromV << "' to '" << toV << "'..." << endl; \
        fromVersion = toV;
#define ENDUPGRADE() \
        sqlSession->once << "UPDATE " << configTableName << " SET value = :version WHERE key = 'version'", use(fromVersion); \
        cerr << "Database schema successfully upgraded to version '" << fromVersion << "'." << endl; \
    }

    cerr << "Database schema will be upgraded." << endl;

    // Upgrades are listed in order
    if (fromVersion == "xxxxxxxx-x") {// drop all databases
        cerr << "Old databases are to be dropped." << endl;
#define TRYDROP(name) \
        sqlSession->once << "SELECT * FROM information_schema.tables WHERE table_name = :" #name "TableName LIMIT 1", use(name ## TableName); \
        if (sqlSession->got_data()) sqlSession->once << "DROP TABLE " << name ## TableName;

        TRYDROP(config);
        TRYDROP(backlog);
        TRYDROP(channel);
        TRYDROP(server);
        TRYDROP(aliasset);
        TRYDROP(alias);
        TRYDROP(user);
        TRYDROP(nick);
#undef TRYDROP
        fromVersion = "00000000-0";
        cerr << "Drop operation finished." << endl;
    }
    UPGRADE("dddddddd-d", toVersion) // create demo content
        sqlSession->once << "INSERT INTO " << userTableName << " (username, password) VALUES ('it', 'it')";
        sqlSession->once << "INSERT INTO " << aliassetTableName << " (user_id, title) VALUES (1, 'idvUsers')";
        sqlSession->once << "INSERT INTO " << aliasTableName << " (aliasset_id, nick) VALUES (1, 'iircUser')";
        sqlSession->once << "INSERT INTO " << aliasTableName << " (aliasset_id, nick) VALUES (1, 'iircUser_')";
        sqlSession->once << "INSERT INTO " << aliasTableName << " (aliasset_id, nick) VALUES (1, 'iircUser__')";
        sqlSession->once << "INSERT INTO " << serverTableName << " (user_id, host, port, ssl, password, servername, aliasset_id, realnames, autoconnect) VALUES (1, 'localhost', 6667, false, null, 'localhost', 1, 'iirc iirc_ iirc__', true)";
        sqlSession->once << "INSERT INTO " << channelTableName << " (server_id, name, lastread, autojoin) VALUES (1, '#test', 0, true)";
        sqlSession->once << "INSERT INTO " << channelTableName << " (server_id, name, lastread, autojoin) VALUES (1, '#noautologin', 0, false)";
        sqlSession->once << "INSERT INTO " << backlogTableName << " (channel_id, time, type, flags, sender_id, message) VALUES (1, '2015-01-01 01:00:00', 0, 0, 1, 'testmessage1')";
        sqlSession->once << "INSERT INTO " << backlogTableName << " (channel_id, time, type, flags, sender_id, message) VALUES (1, '2015-01-01 02:00:00', 0, 0, 1, 'testmessage2')";
        sqlSession->once << "INSERT INTO " << backlogTableName << " (channel_id, time, type, flags, sender_id, message) VALUES (1, '2015-01-01 03:00:00', 0, 0, 1, 'testmessage3')";
        sqlSession->once << "INSERT INTO " << backlogTableName << " (channel_id, time, type, flags, sender_id, message) VALUES (1, '2015-01-01 04:00:00', 0, 0, 1, 'testmessage4')";
    ENDUPGRADE()
    UPGRADE("00000000-0", toVersion) // create from scratch revision
        sqlSession->once << "CREATE TABLE " << configTableName << " (id serial, key varchar(80), value text)";
        sqlSession->once << "CREATE TABLE " << backlogTableName << " (msg_id serial, channel_id integer, time timestamp, type integer, flags integer, sender_id integer, message text)";
        sqlSession->once << "CREATE TABLE " << channelTableName << " (channel_id serial, server_id integer, name text, key text, lastread integer, autojoin boolean)";
        sqlSession->once << "CREATE TABLE " << serverTableName << " (server_id serial, user_id integer, host text, port integer, ssl boolean, password text, servername text, aliasset_id integer, realnames text, autoconnect boolean)";
        sqlSession->once << "CREATE TABLE " << aliassetTableName << " (aliasset_id serial, user_id integer, title text)";
        sqlSession->once << "CREATE TABLE " << aliasTableName << " (alias_id serial, aliasset_id integer, nick text)";
        sqlSession->once << "CREATE TABLE " << userTableName << " (user_id serial, username text, password text)";
        sqlSession->once << "CREATE TABLE " << nickTableName << " (nick_id serial, nick text)";

        sqlSession->once << "INSERT INTO " << configTableName << " (key, value) VALUES ('version', '00000000-0')"; // will be updated in ENDUPGRADE()
    ENDUPGRADE()

    // Check if upgrade did completely finish
    if (fromVersion != toVersion) {
        cerr << "Can not upgrade from version '" << fromVersion << "' to '" << toVersion << "'." << endl;
        return false;
    }

#undef UPGRADE
#undef ENDUPGRADE

    return true;
}

bool DatabaseHandler::initializeDatabase() {
    using namespace soci;

    string dbMetamodelVersion; // will be read from db

    // Look for configuration table to determine version. Create default table scheme otherwise
    sqlSession->once << "SELECT * FROM information_schema.tables WHERE table_name = :configTableName LIMIT 1", use(configTableName);
    if (!sqlSession->got_data()) {
        dbMetamodelVersion = "00000000-0";
    } else if (!forceDropDatabases) {
        sqlSession->once << "SELECT value FROM " << configTableName << " WHERE key = 'version' LIMIT 1", into(dbMetamodelVersion);
        if (!sqlSession->got_data()) {
            cerr << "Corrupt database: Schema version information missing." << endl;
            return false;
        }
    }

    if (forceDropDatabases)
        dbMetamodelVersion = "xxxxxxxx-x";

    if (sqlMetamodelVersion != dbMetamodelVersion)
        upgradeDatabase(dbMetamodelVersion, sqlMetamodelVersion);

    if (forceAddDemoContent)
        upgradeDatabase("dddddddd-d", sqlMetamodelVersion);

    return true;
}

std::list<UserData> DatabaseHandler::getUserData() {
    using namespace soci;

    list<UserData> result;

    size_t userId;
    string username;
    string password;

    statement st = (sqlSession->prepare << "SELECT user_id, username, password FROM " << userTableName,
            into(userId), into(username), into(password));
    st.execute();

    while (st.fetch()) {
        result.emplace_back();
        UserData& userData = result.back();

        {
            size_t aliasId;
            size_t aliassetId;
            string nick;

            statement stNicks = (sqlSession->prepare << "SELECT alias_id, " << aliasTableName << ".aliasset_id, nick FROM " << aliasTableName <<
                                 ", " << aliassetTableName << " WHERE " << aliasTableName << ".aliasset_id = " << aliassetTableName << ".aliasset_id AND user_id = :userId",
                    into(aliasId), into(aliassetId), into(nick), use(userId));
            stNicks.execute();

            while (stNicks.fetch()) {
                list<NickData> *nickDataList;
                auto it = userData.aliasSets.find(aliassetId);
                if (it != userData.aliasSets.end()) {
                    nickDataList = &it->second;
                } else {
                    auto jt = userData.aliasSets.emplace(piecewise_construct, forward_as_tuple(aliassetId),
                                                         forward_as_tuple());
                    nickDataList = &jt.first->second;
                }
                nickDataList->emplace_back();
                NickData &nickData = nickDataList->back();
                nickData.aliasId = aliasId;
                nickData.aliassetId = aliassetId;
                nickData.nick = nick;
            }
        }

#define ASSIGN(name) \
        userData.name = name

        ASSIGN(userId);
        ASSIGN(username);

#undef ASSIGN
    }

    return result;
}

std::list<ServerData> DatabaseHandler::getAutoConnectServers(size_t userId) {
    using namespace soci;

    list<ServerData> result;

    size_t serverId;
    std::string host;
    size_t port;
    int ssl;
    std::string password;
    indicator password_ind;
    std::string servername;
    size_t aliasset_id;
    std::string realnames;
    int autoconnect;

    statement st = (sqlSession->prepare << "SELECT server_id, host, port, ssl, password, servername, aliasset_id, realnames, autoconnect FROM " << serverTableName << " WHERE user_id = :userId AND autoconnect = true",
            into(serverId), into(host), into(port), into(ssl), into(password, password_ind), into(servername), into(aliasset_id), into(realnames), into(autoconnect), use(userId));
    st.execute();

    while (st.fetch()) {
        result.emplace_back();
        ServerData& serverData = result.back();
#define ASSIGN(name) \
        serverData.name = name

        ASSIGN(serverId);
        ASSIGN(host);
        ASSIGN(port);
        ASSIGN(ssl);
        ASSIGN(servername);
        ASSIGN(aliasset_id);
        ASSIGN(realnames);
        ASSIGN(autoconnect);

#undef ASSIGN
    }

    return result;
}

std::list<ChannelData> DatabaseHandler::getAutoJoinChannels(size_t serverId) {
    using namespace soci;

    list<ChannelData> result;

    size_t channelId;
    string name;
    string key;
    int autojoin;

    statement st = (sqlSession->prepare << "SELECT channel_id, name, key, autojoin FROM " << channelTableName << " WHERE server_id = :serverId AND autojoin = true",
            into(channelId), into(name), use(serverId));
    st.execute();

    while (st.fetch()) {
        result.emplace_back();
        ChannelData& channelData = result.back();
#define ASSIGN(name) \
        channelData.name = name

        ASSIGN(channelId);
        ASSIGN(name);
        ASSIGN(key);
        ASSIGN(autojoin);

#undef ASSIGN
    }

    return result;
}

std::string DatabaseHandler::getChannelName(size_t channelId) {
	using namespace soci;

	string channelName;
	sqlSession->once << "SELECT name FROM " << channelTableName << " WHERE channel_id = :channelId LIMIT 1", into(channelName), use(channelId);
	if (sqlSession->got_data())
		return channelName;
	return string();
}

size_t DatabaseHandler::getOrCreateChannelId(size_t serverId, const std::string &channelName) {
    using namespace soci;

    size_t channelId;
    statement st = (sqlSession->prepare << "SELECT channel_id FROM " << channelTableName << " WHERE server_id = :serverId AND name = :channelName LIMIT 1", into(channelId), use(serverId), use(channelName));
    st.execute();
    if (!st.fetch())
        sqlSession->once << "INSERT INTO " << channelTableName << " (server_id, name) VALUES (:serverId, :channelName)", use(serverId), use(channelName);
    st.execute();
    if (!st.fetch())
        channelId = 0;

    return channelId;
}

size_t DatabaseHandler::getOrCreateSenderId(const std::string &senderNick) {
    using namespace soci;

    size_t senderId;
    statement st = (sqlSession->prepare << "SELECT nick_id FROM " << nickTableName << " WHERE nick = :senderNick LIMIT 1", into(senderId), use(senderNick));
    st.execute();
    if (!st.fetch())
        sqlSession->once << "INSERT INTO " << nickTableName << " (nick) VALUES (:senderNick)", use(senderNick);
    st.execute();
    if (!st.fetch())
        senderId = 0;

    return senderId;
}

size_t DatabaseHandler::storeMessage(size_t senderId, size_t channelId, const std::string &message) {
    using namespace soci;

    sqlSession->once << "INSERT INTO " << backlogTableName << " (channel_id, sender_id, message, time) VALUES (:channelId, :senderId, :message, NOW())", use(channelId), use(senderId), use(message);
}

size_t DatabaseHandler::getUserFromLogin(const std::string &username, const std::string &password) {
    using namespace soci;

    size_t result;

    // TODO: don't check passwords vs plaintext
    sqlSession->once << "SELECT user_id FROM " << userTableName << " WHERE username = :username AND password = :password", into(result), use(username), use(password);
    if (!sqlSession->got_data())
        result = 0;

    return result;
}
