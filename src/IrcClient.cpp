#include "IrcClient.h"
#include "data/ServerData.h"
#include "UserHandler.h"
#include "IrcClientImpl.h"


using namespace std;



IrcClient::IrcClient(UserHandler& userHandler, const ServerData& serverData) : impl(make_shared<IrcClientImpl>(*this, userHandler, serverData)){
}

size_t IrcClient::getServerId() {
    return impl->serverData.serverId;
}

bool IrcClient::connect() {
    return impl->connect();
}

void IrcClient::join(const char* channel, const char* key) {
    impl->join(channel, key);
}
