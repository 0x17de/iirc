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

size_t IrcClient::getChannelId(const std::string &channelName) {
    size_t result;

    auto it = impl->joinedChannels.find(channelName);
    if (it == impl->joinedChannels.end())
        result = 0;
    else
        result = it->second;

    if (result == 0) // lookup in database
        result = impl->userHandler.getDatabaseHandler().getOrCreateChannelId(getServerId(), channelName);

    return result;
}

bool IrcClient::connect() {
    return impl->connect();
}

void IrcClient::disconnect() {
    impl->disconnect();
}

void IrcClient::join(const char* channel, const char* key) {
    impl->join(channel, key);
}
