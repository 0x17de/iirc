#ifndef IIRC_SERVERDATA_H
#define IIRC_SERVERDATA_H


#include <string>
#include <list>



struct ServerData {
    size_t serverId = 0;
    std::string host;
    size_t port;
    bool ssl;
    std::string password;
    std::string servername;
    size_t aliasset_id = 0;
    std::list<std::string> aliasset;
    std::string realnames;
    bool autoconnect = false;
};



#endif //IIRC_SERVERDATA_H
