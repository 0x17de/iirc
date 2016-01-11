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
    cerr << "onEvent" << endl;
    IrcClientImpl::getClientFromSessionId(session)->onEvent<T>(event, origin, params, count);
}


template <>
void IrcClientImpl::onEvent<IrcEvent::Unknown>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT UNKNOWN" << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Connect>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT CONNECT" << endl;
    userHandler.onEvent<IrcEvent::Connect>(client);
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Quit>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT QUIT" << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Channel>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT CHANNEL" << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::PrivMsg>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT PRIVMSG" << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Join>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT JOIN" << endl;
}


IrcClientImpl::IrcClientImpl(IrcClient& client, UserHandler& userHandler, const ServerData& serverData) : client(client), userHandler(userHandler), serverData(serverData), callbacks{0}, session{0} {
    // register various events here
    callbacks.event_connect = ::onEvent<IrcEvent::Connect>;
    callbacks.event_nick;
    callbacks.event_quit = ::onEvent<IrcEvent::Quit>;
    callbacks.event_join = ::onEvent<IrcEvent::Join>;
    callbacks.event_part;
    callbacks.event_mode;
    callbacks.event_umode;
    callbacks.event_topic;
    callbacks.event_kick;
    callbacks.event_channel = ::onEvent<IrcEvent::Channel>;
    callbacks.event_privmsg = ::onEvent<IrcEvent::PrivMsg>;
    callbacks.event_notice;
    callbacks.event_channel_notice;
    callbacks.event_invite;
    callbacks.event_ctcp_req;
    callbacks.event_ctcp_rep;
    callbacks.event_ctcp_action;
    callbacks.event_unknown = ::onEvent<IrcEvent::Unknown>;

    /* irc_eventcode_callback_t.event_numeric;
    irc_event_dcc_chat_t.event_dcc_chat_req;
    irc_event_dcc_send_t.event_dcc_send_req; */
}

IrcClientImpl::~IrcClientImpl() {
    destroySession();
}

bool IrcClientImpl::createSession() {
    if (session != 0) return false; // already created
    session = irc_create_session(&callbacks);
    if (session == 0) return false;
    sessionToClient.emplace(session, this);
    return true;
}

bool IrcClientImpl::destroySession() {
    if (session != 0) {
        irc_cmd_quit(session, 0);
        runThread.join();
        irc_destroy_session(session);
        sessionToClient.erase(session);
    }
    session = 0;
}
std::string IrcClientImpl::getConnectionId() {
    return userHandler.getUserData().username + "@" + serverData.servername;
}

bool IrcClientImpl::disconnect() {
    destroySession();
}

void IrcClientImpl::displayError() {
    int errorNo = irc_errno(session);
    cerr << "IRC Error #" << errorNo << ": " << irc_strerror(errorNo) << endl;
}

bool IrcClientImpl::connect() {
    if (!createSession()) {
        cerr << "Could not create IRC session for connection '" << getConnectionId() << "'." << endl;
        return false;
    }

    if (serverData.aliasset.empty() == 0) {
        cerr << "The aliasset is empty for connection  '" << getConnectionId() << "'." << endl;
        return false;
    }

    string host = serverData.host;
    if (serverData.ssl) host = '#' + host;
    const char* password = serverData.password.empty() ? 0 : serverData.password.c_str(); // if no password supply 0
    const char* nick = serverData.aliasset.front().c_str(); // use first alias

    int result = irc_connect(session, host.c_str(), serverData.port, password, nick, 0 /* HIDE */, 0 /* HIDE */);
    if (result != 0) {
        displayError();
        disconnect();
        return false;
    }

    // wait for event_connect
    runThread = thread([&] {
        runResult = irc_run(session);
    });

    return false;
}

void IrcClientImpl::join(const char* channel, const char* key) {
    irc_cmd_join(session, channel, key);
}
