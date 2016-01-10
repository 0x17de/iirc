//
// Created by it on 1/10/16.
//

#include <iostream>
#include <libircclient.h>
#include <libirc_events.h>
#include "IrcClientImpl.h"


using namespace std;



std::unordered_map<irc_session_t*,IrcClientImpl*> IrcClientImpl::sessionToClient;
IrcClientImpl* IrcClientImpl::getClientFromSessionId(irc_session_t* session) {
    return sessionToClient.at(session);
}


// Helper wrapper from C to C++
template <IrcEvent T>
static inline void onEvent(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count) {
    IrcClientImpl::getClientFromSessionId(session)->onEvent<T>(event, origin, params, count);
}


template <>
void IrcClientImpl::onEvent<IrcEvent::Unknown>(const char *event, const char *origin, const char **params, unsigned int count) {

}

template <>
void IrcClientImpl::onEvent<IrcEvent::Connect>(const char *event, const char *origin, const char **params, unsigned int count) {

}

template <>
void IrcClientImpl::onEvent<IrcEvent::Quit>(const char *event, const char *origin, const char **params, unsigned int count) {

}

template <>
void IrcClientImpl::onEvent<IrcEvent::Channel>(const char *event, const char *origin, const char **params, unsigned int count) {

}

template <>
void IrcClientImpl::onEvent<IrcEvent::PrivMsg>(const char *event, const char *origin, const char **params, unsigned int count) {

}

template <>
void IrcClientImpl::onEvent<IrcEvent::Join>(const char *event, const char *origin, const char **params, unsigned int count) {

}


IrcClientImpl::IrcClientImpl(UserHandler& userHandler, const ServerData& serverData) : userHandler(userHandler), serverData(serverData), callbacks{0}, session{0} {
    // register various events here
    callbacks.event_unknown = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_connect = ::onEvent<IrcEvent::Connect>;
    callbacks.event_quit = ::onEvent<IrcEvent::Quit>;
    callbacks.event_channel = ::onEvent<IrcEvent::Channel>;
    callbacks.event_privmsg = ::onEvent<IrcEvent::PrivMsg>;
    callbacks.event_join = ::onEvent<IrcEvent::Join>;
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