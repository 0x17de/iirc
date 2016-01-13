#ifndef IIRC_IRCCLIENT_H
#define IIRC_IRCCLIENT_H


#include <memory>
#include <string>
#include <list>



struct IrcChannelData {
	size_t channelId;
	std::string name;
};


class UserHandler;
class ServerData;
class IrcClientImpl;
class IrcClient {
    std::shared_ptr<IrcClientImpl> impl;

public:
    IrcClient(UserHandler& userHandler, const ServerData& serverData);
    size_t getServerId();
    const IrcChannelData& getChannelData(size_t channelId);
    const IrcChannelData& getChannelData(const std::string& channelName);
    bool connect();
    void disconnect();
    void join(const char* channel, const char* key);
	void send(const std::string& channel, const std::string& message);
};


#endif //IIRC_IRCCLIENT_H
