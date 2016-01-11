//
// Created by it on 1/10/16.
//

#ifndef IIRC_IRCCLIENTIMPL_H
#define IIRC_IRCCLIENTIMPL_H


#include <unordered_map>
#include <string>
#include <list>
#include <thread>
#include <map>

#include <libircclient.h>
#include <thread>
#include "UserHandler.h"
#include "IrcEvent.h"


class IrcClient;
class IrcClientImpl {
public:
    IrcClient& client;
    UserHandler& userHandler;
    ServerData serverData;
    irc_callbacks_t callbacks;
    irc_session_t* session;
    std::thread runThread;
    int runResult;
    std::map<std::string, size_t> joinedChannels;

private:
    bool createSession();
    bool destroySession();
    static std::unordered_map<irc_session_t*,IrcClientImpl*> sessionToClient;

public:
    static IrcClientImpl* getClientFromSessionId(irc_session_t* session);

    IrcClientImpl(IrcClient& client, UserHandler& userHandler, const ServerData& serverData);
    ~IrcClientImpl();

    template <IrcEvent> void onEvent(const char * event, const char * origin, const char ** params, unsigned int count);
    std::string getConnectionId();
    bool disconnect();
    void displayError();
    bool connect();
    void join(const char* channel, const char* key);
};


#endif //IIRC_IRCCLIENTIMPL_H
