#include "IrcClient.h"
#include "data/ServerData.h"
#include "UserHandler.h"
#include "IrcClientImpl.h"


using namespace std;



IrcClient::IrcClient(UserHandler& userHandler, const ServerData& serverData) : impl(make_shared<IrcClientImpl>(userHandler, serverData)){
}

bool IrcClient::connect() {
    return impl->connect();
}
