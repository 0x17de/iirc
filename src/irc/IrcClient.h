#ifndef IIRC_IRCCLIENT_H
#define IIRC_IRCCLIENT_H


#include <memory>
#include <string>
#include <map>
#include <set>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "data/ChannelData.h"


typedef boost::multi_index::multi_index_container<
  ChannelData,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<
      boost::multi_index::member<
        ChannelData, std::string, &ChannelData::name
      >
    >,
    boost::multi_index::ordered_unique<
      boost::multi_index::member<
        ChannelData, size_t, &ChannelData::channelId
      >
    >
  >
> ChannelData_multi;



struct ChannelData;
class UserHandler;
class ServerData;
class IrcClientImpl;
class IrcClient {
    std::shared_ptr<IrcClientImpl> impl;

public:
    IrcClient(UserHandler& userHandler, const ServerData& serverData);
    size_t getServerId();
    const ServerData& getServerData() const;
    const ChannelData& getChannelData(size_t channelId);
    const ChannelData& getChannelData(const std::string& channelName);
    const ChannelData_multi& getChannelDataMulti() const;
	
    bool connect();
    void disconnect();
    void join(const char* channel, const char* key);
	void send(const std::string& channel, const std::string& message);
};



#endif //IIRC_IRCCLIENT_H
