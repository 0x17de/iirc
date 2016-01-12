//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPINTERFACE_H
#define IIRC_TCPINTERFACE_H


#include <memory>
#include <vector>
#include <functional>
#include <cstdint>



struct Header {
    size_t type;
    uint64_t length;
};

class TcpInterfaceImpl;
class TcpInterface {
    std::shared_ptr<TcpInterfaceImpl> impl;

public:
    TcpInterface();
    void run();
    void stop();

    void onHeader(std::function<bool(const Header& header, void* t)> callback);
    void onData(std::function<bool(const std::vector<uint8_t>& data, void* t)> callback);
    void setCallbackParameter(void* t);
};


#endif //IIRC_TCPINTERFACE_H
