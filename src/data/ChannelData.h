#ifndef IIRC_CHANNELDATA_H
#define IIRC_CHANNELDATA_H


#include <string>
#include <set>



struct ChannelData {
	ChannelData();
	ChannelData(size_t channelId, std::string name);
    size_t channelId = 0;
    std::string name;
    std::string key;
    bool autojoin;
    std::set<std::string> userList;
};



#endif //IIRC_CHANNELDATA_H
