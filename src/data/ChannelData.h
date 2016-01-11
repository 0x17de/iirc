#ifndef IIRC_CHANNELDATA_H
#define IIRC_CHANNELDATA_H


#include <string>



struct ChannelData {
    size_t channelId = 0;
    std::string name;
    std::string key;
    bool autojoin;
};


#endif //IIRC_CHANNELDATA_H
