#ifndef HCHATCLIENTSOCKET_H
#define HCHATCLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>

class HChatClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit HChatClientSocket(QObject *parent = nullptr);
public:
    void getStatus();
    void closeSocket();

    void connectServer(const QString& anyIPV4, const int& port);
    void disconnectServer();

signals:
    void uploadConnectStatus(const quint8& status);
    void uploadCurrentMessage(const quint8& info, const QJsonValue& data);

private:
    void checkLoginStatus(const QJsonValue& data);
    void checkUserOnline (const QJsonValue& data);
    void checkUserOffline(const QJsonValue& data);
    void checkFriendsMsg (const QByteArray& data);

public slots:  ///响应QTcpSocket分发的消息
    ///
    /// \brief onRecvTcpConnected
    /// \brief 响应connected消息
    ///
    void onRecvTcpConnected();
    ///
    /// \brief onRecvTcpDisconnted
    /// \brief 响应disconnected消息
    ///
    void onRecvTcpDisconnted();
    ///
    /// \brief onRecvTcpReadyReadData
    /// \brief 响应readyRead消息
    ///
    void onRecvTcpReadyReadData();
    ///
    /// \brief onMessageTransform
    /// \param type
    /// \param value
    ///
    void onMessageTransform(const quint8& type, const QJsonValue& value);

Q_SIGNALS:
    ///
    /// \brief onNewUserConnected
    /// \brief 有客户连接分发此消息
    ///
    void onNewUserConnected();
    ///
    /// \brief onUserHasDisConnected
    /// \brief 有客户断开分发此消息
    ///
    void onUserHasDisConnected();
    ///
    /// \brief onSendMessageToClient
    /// \brief 分发给指定客户端的消息
    /// \param fileType
    /// \param clientID
    /// \param context
    ///
    void onSendMessageToClient(const quint8& fileType, const int& clientID, const QJsonValue& context);

private:
    QTcpSocket *client_;

    int clientID_;
};

#endif // HCHATCLIENTSOCKET_H
