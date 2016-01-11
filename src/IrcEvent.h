#ifndef IIRC_IRCEVENT_H
#define IIRC_IRCEVENT_H



enum class IrcEvent {
    Unknown,
    Connect,
    Quit,
    Channel,
    Join,
    PrivMsg
};



#endif //IIRC_IRCEVENT_H
