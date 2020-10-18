#ifndef HCHATSERVER_H
#define HCHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QVector>

#include "HChatServerSocket.h"
#include "HChatMessageNotifyManage.h"


/// 服务器管理类
class HChatServer : public QObject {
    Q_OBJECT
public:
    explicit HChatServer(QObject *parent = 0);
    ~HChatServer();

    bool startListen(int port = 6666);
    void closeListen();
    bool hasListen() const;
    void transformMessageHasBeListen(const quint8& type, const QString& name, const QJsonValue& data);
signals:
    void signalUserStatus(const QString &text);
    void signalClientInfo(const QString &addrss_, const QString &port_, const int &client_);
    void signalCurrentUserStatus(const QString& name, const quint8& op);
    void signalListenClientStatus(const QString& name, const QString& content);
protected:
    QTcpServer *managerServer_;
    HChatMessageNotifyManage* p_;
protected slots:
    virtual void newConnection() = 0;
    virtual void connected()     = 0;
    virtual void disConnected()  = 0;
};

/// 消息 Socket
class HChatMsgServer : public HChatServer {
    Q_OBJECT
public:
    explicit HChatMsgServer(QObject *parent = 0);
    ~HChatMsgServer();

signals:
    void signalDownloadFile(const QJsonValue& data);

private:
    QVector<HChatServerSocket *> m_clients;

public slots:
    void transMessageToAllClient(const quint8& type, const QJsonValue& data);
    void transFileToClient(const int& clientID, const QJsonValue& data);
private slots:
    void newConnection();
    void connected();
    void disConnected();
    void msgToClient(const quint8& type, const int& id, const QJsonValue& date);
    void msgToAllClient(const quint8 &type, const QJsonValue& data);
};

/// 文件 Socket
class HChatFileServer : public HChatServer {
    Q_OBJECT
public :
    explicit HChatFileServer(QObject *parent = 0);
    ~HChatFileServer();
signals:
    void signalRecvFinished(int id, const QJsonValue &json);
private:
    QVector <HChatClientFileSocket *> m_clients;
private slots:
    void newConnection();
    void connected();
    void disConnected();
    void clientDownloadFile(const QJsonValue &json);
};
#endif // HCHATSERVER_H
