#ifndef IIRC_IRCCLIENT_H
#define IIRC_IRCCLIENT_H


#include <memory>



class UserHandler;
class ServerData;
class IrcClientImpl;
class IrcClient {
    std::shared_ptr<IrcClientImpl> impl;

public:
    IrcClient(UserHandler& userHandler, const ServerData& serverData);
    bool connect();
};


#endif //IIRC_IRCCLIENT_H
