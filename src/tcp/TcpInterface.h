//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPINTERFACE_H
#define IIRC_TCPINTERFACE_H


#include <memory>
#include <vector>
#include <functional>
#include <cstdint>



namespace iircCommon {
    class Header;
}

class UserHandler;
class TcpInterfaceImpl;
class TcpInterface {
    std::shared_ptr<TcpInterfaceImpl> impl;

public:
    TcpInterface();
    void run();
    void stop();

    void onHeader(std::function<bool(const iircCommon::Header& header, UserHandler* t)> callback);
    void onData(std::function<bool(const std::vector<uint8_t>& data, UserHandler* t)> callback);
    void onClose(std::function<void(UserHandler* t)> callback);
    void setUserHandler(UserHandler* t);
};


#endif //IIRC_TCPINTERFACE_H
