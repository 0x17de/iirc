#include <exception>
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

const IrcChannelData& IrcClient::getChannelData(size_t channelId) {
	auto& index = impl->channelMulti.get<1>();
    auto it = index.find(channelId);
    if (it == index.end())
		throw out_of_range("Could not find channel.");
	return *it;
}

const IrcChannelData& IrcClient::getChannelData(const std::string &channelName) {
    auto& index = impl->channelMulti.get<0>();
    auto it = index.find(channelName);
    if (it == index.end())
		throw out_of_range("Could not find channel.");
	return *it;
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
