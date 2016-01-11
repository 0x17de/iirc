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
    cerr << "EVENT UNKNOWN: " << (event?event:"") << " " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Connect>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT CONNECT: " << (event?event:"") << " " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
    userHandler.onEvent<IrcEvent::Connect>(client);
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Quit>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT QUIT: " << (event?event:"") << " " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Channel>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT CHANNEL: " << (event?event:"") << " " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::PrivMsg>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT PRIVMSG: " << (event?event:"") << " " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
}

template <>
void IrcClientImpl::onEvent<IrcEvent::Join>(const char *event, const char *origin, const char **params, unsigned int count) {
    cout << "EVENT JOIN: " << (event?event:"") << " " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
}


static inline void onEventNumeric(irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count) {
    cerr << "#";
    cerr.fill('0');
    cerr.width(3);
    cerr << event;
    cerr << ": " << (origin?origin:"");
    for (unsigned int i = 0; i < count; ++i)
        cerr << " " << params[i];
    cerr << endl;
}


IrcClientImpl::IrcClientImpl(IrcClient& client, UserHandler& userHandler, const ServerData& serverData) : client(client), userHandler(userHandler), serverData(serverData), callbacks{0}, session{0}, runResult{0} {
    // register various events here
    callbacks.event_connect = ::onEvent<IrcEvent::Connect>;
    callbacks.event_nick = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_quit = ::onEvent<IrcEvent::Quit>;
    callbacks.event_join = ::onEvent<IrcEvent::Join>;
    callbacks.event_part = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_mode = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_umode = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_topic = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_kick = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_channel = ::onEvent<IrcEvent::Channel>;
    callbacks.event_privmsg = ::onEvent<IrcEvent::PrivMsg>;
    callbacks.event_notice = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_channel_notice = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_invite = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_ctcp_req = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_ctcp_rep = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_ctcp_action = ::onEvent<IrcEvent::Unknown>;
    callbacks.event_unknown = ::onEvent<IrcEvent::Unknown>;

    callbacks.event_numeric = ::onEventNumeric;
    //irc_event_dcc_chat_t.event_dcc_chat_req;
    //irc_event_dcc_send_t.event_dcc_send_req;
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
        if (runThread.joinable())
            runThread.join();
        irc_destroy_session(session);
        sessionToClient.erase(session);
        cerr << "RunResult: " << runResult << endl;
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

    auto aliasIt = userHandler.getUserData().aliasSets.find(serverData.aliasset_id);
    if (aliasIt == userHandler.getUserData().aliasSets.end()) {
        cerr << "The aliasset for connection '" << getConnectionId() << "' was not found." << endl;
        return false;
    } else if (aliasIt->second.empty()) {
        cerr << "The aliasset for connection '" << getConnectionId() << "' is empty." << endl;
        return false;
    }

    string host = serverData.host;
    if (serverData.ssl) host = '#' + host;
    const char* password = serverData.password.empty() ? 0 : serverData.password.c_str(); // if no password supply 0
    const char* nick = aliasIt->second.front().nick.c_str(); // use first alias

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

    return true;
}

void IrcClientImpl::join(const char* channel, const char* key) {
    irc_cmd_join(session, channel, key);
}
