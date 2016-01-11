#ifndef IIRC_IRCCLIENT_H
#define IIRC_IRCCLIENT_H


#include <memory>
#include <string>
#include <list>



class UserHandler;
class ServerData;
class IrcClientImpl;
class IrcClient {
    std::shared_ptr<IrcClientImpl> impl;

public:
    IrcClient(UserHandler& userHandler, const ServerData& serverData);
    size_t getServerId();
    bool connect();
    void join(const char* channel, const char* key);
};


#endif //IIRC_IRCCLIENT_H
