#ifndef IIRC_USERDATA_H
#define IIRC_USERDATA_H


#include <string>
#include <list>
#include <map>


struct NickData {
    size_t aliasId;
    size_t aliassetId;
    std::string nick;
};

struct UserData {
    size_t userId;
    std::string username;
    // check password against database - dont put it in here
    std::map<int, std::list<NickData>> aliasSets;
};



#endif //IIRC_USERDATA_H
