//
// Created by it on 1/12/16.
//

#ifndef IIRC_TCPINTERFACE_H
#define IIRC_TCPINTERFACE_H


#include <memory>



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
};


#endif //IIRC_TCPINTERFACE_H
