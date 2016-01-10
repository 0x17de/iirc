//
// Created by it on 1/10/16.
//

#include <iostream>
#include "IrcClientImpl.h"


using namespace std;



std::unordered_map<irc_session_t*,IrcClientImpl*> IrcClientImpl::sessionToClient;
IrcClientImpl* IrcClientImpl::getClientFromSessionId(irc_session_t* session) {
    return sessionToClient.at(session);
}


IrcClientImpl::IrcClientImpl(UserHandler& userHandler, const ServerData& serverData) : userHandler(userHandler), serverData(serverData), callbacks{0}, session{0} {
    // TODO: assign callbacks
}

IrcClientImpl::~IrcClientImpl() {
    destroySession();
}

bool IrcClientImpl::createSession() {
    if (session != 0) return false; // already created
    session = irc_create_session(&callbacks);
    return session != 0;
}

bool IrcClientImpl::destroySession() {
    if (session != 0) {
        sessionToClient.erase(session);
        irc_destroy_session(session);
    }
    session = 0;
}
std::string IrcClientImpl::getConnectionId() {
    return userHandler.getUserData().username + "@" + serverData.servername;
}

bool IrcClientImpl::disconnect() {
    destroySession();
}

bool IrcClientImpl::connect() {
    if (!createSession()) {
        cerr << "Could not create IRC session for connection '" << getConnectionId() << "'." << endl;
        return false;
    }
    sessionToClient.emplace(session, this);

    if (serverData.aliasset.empty() == 0) {
        cerr << "The aliasset is empty for connection  '" << getConnectionId() << "'." << endl;
        return false;
    }

    string host = serverData.host;
    if (serverData.ssl) host = '#' + host;
    const char* password = serverData.password.empty() ? 0 : serverData.password.c_str(); // if no password supply 0
    const char* nick = serverData.aliasset.front().c_str(); // use first alias

    int result = irc_connect(session, host.c_str(), serverData.port, password, nick, 0 /* HIDE */, 0 /* HIDE */);
    if (result == 0) return true; // wait for event_connect

    disconnect();
    return false;
}