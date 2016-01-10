#ifndef IIRC_SERVERDATA_H
#define IIRC_SERVERDATA_H


#include <string>


struct ServerData {
    size_t serverId;
    std::string host;
    size_t port;
    bool ssl;
    std::string password;
    std::string servername;
    size_t aliasset_id;
    std::string realnames;
    bool autoconnect;
};



#endif //IIRC_SERVERDATA_H
