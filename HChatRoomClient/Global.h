#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>

namespace GlobalMessage {
    enum LoginStatus {
        ConnectedToHost = 0x01,
        DisConnectToHost,
        LoginSuccess,
        LoginFailued,
        ClientOnline,
        ClientOffline
    };

    enum MessageGroup {
        ClientNULL,
        ClientRegister = 0x10,
        ClientLogin,
        ClientLoginOut,

        // 在线与否状态
        ClientUserOnLine = 0x15,
        ClientUserOffLine,

        // 消息状态
        ClientSendMsg    = 0x40,
        ClientSendFile,
        ClientSendPicture
    };

    static const QString server_Address_     = "127.0.0.1";
    static const int     server_MessagePort_ = 66666;
    static const int     server_FilePort_    = 10001;

} /// GlobalMessage

#endif // GLOBAL_H
