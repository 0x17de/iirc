#include "IrcClient.h"
#include <iostream>
#include <libircclient.h>
#include "data/ServerData.h"


using namespace std;



class IrcClientImpl {
public:
    ServerData serverData;
    irc_callbacks_t callbacks;
    irc_session_t* session;

    IrcClientImpl(const ServerData& serverData) : callbacks{0} {
        this->serverData = serverData;
    }
    ~IrcClientImpl() {
        irc_destroy_session(session);
    }
    bool createSession() {
        session = irc_create_session(&callbacks);
        return (bool)session;
    }
};

IrcClient::IrcClient(const ServerData& serverData) : impl(make_shared<IrcClientImpl>(serverData)){
}

bool IrcClient::connect() {
    if (!impl->createSession()) {
        cerr << "Could not create IRC session for connection '" << impl->serverData.servername << "'." << endl;
        return false;
    }

    

    return true;
}
