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
signals:
    void signalUserStatus(const QString &text);
    void signalClientInfo(const QString &addrss_, const QString &port_, const int &client_);
    void signalCurrentUserStatus(const QString& name, const quint8& op);
protected:
    QTcpServer *managerServer_;
    HChatMessageNotifyManage* p_;
protected slots:
    virtual void newConnection() = 0;
    virtual void connected()     = 0;
    virtual void disConnected()  = 0;
};

/// Message Socket
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
    void transFileToClient(const int& clientID, const QJsonValue& data);
private slots:
    void newConnection();
    void connected();
    void disConnected();
    void msgToClient(const quint8& type, const int& id, const QJsonValue& date);
};

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
