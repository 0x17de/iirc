#include <exception>
#include "IrcClient.h"
#include "data/ServerData.h"
#include "data/ChannelData.h"
#include "UserHandler.h"
#include "IrcClientImpl.h"


using namespace std;



IrcClient::IrcClient(UserHandler& userHandler, const ServerData& serverData) : impl(make_shared<IrcClientImpl>(*this, userHandler, serverData)){
}

size_t IrcClient::getServerId() {
    return impl->serverData.serverId;
}

const ServerData& IrcClient::getServerData() const {
    return impl->serverData;
}

const ChannelData& IrcClient::getChannelData(size_t channelId) {
	return impl->getChannelData(channelId);
}
const ChannelData& IrcClient::getChannelData(const std::string &channelName) {
    return impl->getChannelData(channelName);
}

const ChannelData_multi& IrcClient::getChannelDataMulti() const {
	return impl->channelMulti;
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

void IrcClient::send(const std::string& channel, const std::string& message) {
	impl->send(channel, message);
}

