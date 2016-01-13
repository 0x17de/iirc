//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPINTERFACE_H
#define IIRC_TCPINTERFACE_H


#include <memory>
#include <vector>
#include <functional>
#include <cstdint>
#include "common.pb.h"



class UserHandler;
class TcpClient;
class TcpInterfaceImpl;
class TcpInterface {
    std::shared_ptr<TcpInterfaceImpl> impl;

public:
    TcpInterface();
    void run();
    void stop();

    void onHeader(std::function<bool(iircCommon::DataType dataType, uint64_t dataSize, UserHandler** t)> callback);
    void onData(std::function<bool(iircCommon::DataType dataType, const std::vector<uint8_t>& data, std::shared_ptr<TcpClient> client, UserHandler** t)> callback);
    void onClose(std::function<void(TcpClient* tcpClient, UserHandler** t)> callback);
};


#endif //IIRC_TCPINTERFACE_H
