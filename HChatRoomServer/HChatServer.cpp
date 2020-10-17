#include "HChatServer.h"
#include "HChatServerSocket.h"
#include "Global.h"
#include "HChatDataBaseMgr.h"

using namespace GlobalMessage;

HChatServer::HChatServer(QObject *parent)
    : QObject(parent)
    , managerServer_(new QTcpServer)
    , p_(new HChatMessageNotifyManage){

    connect(managerServer_, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

HChatServer::~HChatServer() {
    if (this->managerServer_->isListening())
        this->managerServer_->close();
}

bool HChatServer::startListen(int port) {
    if (managerServer_->isListening()) managerServer_->close();
    return managerServer_->listen(QHostAddress::Any, port);
}

void HChatServer::closeListen() {
    managerServer_->close();
}


HChatMsgServer::HChatMsgServer(QObject *parent) {

}

HChatMsgServer::~HChatMsgServer() {

}

void HChatMsgServer::transFileToClient(const int &userId, const QJsonValue &json) {
    for (int i = 0; i < m_clients.size(); i++) {
        if (userId == m_clients.at(i)->userClienID()) {
            m_clients.at(i)->replyMessageToClient(MessageGroup::ClientSendFile, json);
            return;
        }
    }
}

void HChatMsgServer::newConnection() {
    HChatServerSocket *client = new HChatServerSocket(this, managerServer_->nextPendingConnection());
    connect(client, SIGNAL(tcpConnected())   , this, SLOT(connected()));
    connect(client, SIGNAL(tcpDisconnected()), this, SLOT(disConnected()));
}

void HChatMsgServer::connected() {
    HChatServerSocket *client = (HChatServerSocket *)this->sender();
    if (NULL == client) return;
    connect(client, SIGNAL(tcpTransformMsgToClient(quint8, int, QJsonValue)),
            this  , SLOT(msgToClient(quint8, int, QJsonValue)));
    connect(client, SIGNAL(tcpRecFile(QJsonValue)), this, SIGNAL(signalDownloadFile(QJsonValue)));

    QString userName = HChatDataBaseMgr::instance().getUserName(client->userClienID());
    Q_EMIT signalUserStatus(QString("用户 [%1] 上线").arg(userName));
    Q_EMIT signalCurrentUserStatus(userName, MessageGroup::ClientUserOnLine);
    p_->notify("新消息", QString("用户%1上线啦！").arg(userName));
    m_clients.push_back(client);
}

void HChatMsgServer::disConnected() {
    HChatServerSocket *client = (HChatServerSocket *)this->sender();
    if (NULL == client) return;
    for (int i = 0; i < m_clients.size(); i++) {
        if (client == m_clients.at(i)) {
            m_clients.remove(i);
            QString userName = HChatDataBaseMgr::instance().getUserName(client->userClienID());
            Q_EMIT signalUserStatus(QString("用户 [%1] 下线").arg(userName));
            Q_EMIT signalCurrentUserStatus(userName, MessageGroup::ClientUserOffLine);
            return;
        }
    }
    disconnect(client, SIGNAL(tcpConnected())   , this, SLOT(connected()));
    disconnect(client, SIGNAL(tcpDisconnected()), this, SLOT(disConnected()));
    disconnect(client, SIGNAL(tcpTransformMsgToClient(quint8, int, QJsonValue)),
               this  , SLOT  (msgToClient(quint8, int, QJsonValue)));
    disconnect(client, SIGNAL(tcpRecFile(QJsonValue)), this, SIGNAL(signalDownloadFile(QJsonValue)));
}

void HChatMsgServer::msgToClient(const quint8 &type, const int &id, const QJsonValue &date) {
    for (int i = 0; i < m_clients.size(); i++) {
        if (id == m_clients.at(i)->userClienID()) {
            m_clients.at(i)->replyMessageToClient(type, date);
            return;
        }
    }
}

HChatFileServer::HChatFileServer(QObject *parent) {}
HChatFileServer::~HChatFileServer() {
    foreach (HChatClientFileSocket *client,  m_clients) {
        m_clients.removeOne(client);
        client->close();
    }
}

void HChatFileServer::newConnection() {
    HChatClientFileSocket *client = new HChatClientFileSocket(this, managerServer_->nextPendingConnection());
    connect(client, SIGNAL(signalConnected())   , this, SLOT(cConnected()));
    connect(client, SIGNAL(signalDisConnected()), this, SLOT(disConnected()));
}

void HChatFileServer::connected() {
    HChatClientFileSocket *client = (HChatClientFileSocket *)this->sender();
    if (NULL == client) return;
    m_clients.push_back(client);
}

void HChatFileServer::disConnected() {
    HChatClientFileSocket *client = (HChatClientFileSocket *)this->sender();
    if (NULL == client) return;
    for (int i = 0; i < m_clients.size(); i++) {
        if (client == m_clients.at(i)) {
            m_clients.remove(i);
            return;
        }
    }
    disconnect(client, SIGNAL(signalConnected())   , this, SLOT(connected()));
    disconnect(client, SIGNAL(signalDisConnected()), this, SLOT(disConnected()));
}

void HChatFileServer::clientDownloadFile(const QJsonValue &json) {
    if (json.isObject()) {
        QJsonObject jsonObj = json.toObject();
        qint32 nId          = jsonObj.value("from").toInt();
        qint32 nWid         = jsonObj.value("id").toInt();;
        QString fileName = jsonObj.value("msg").toString();
        qDebug() << "get file" << jsonObj << m_clients.size();
        for (int i = 0; i < m_clients.size(); i++) {
            if (m_clients.at(i)->checkUserId(nId, nWid)) {
                m_clients.at(i)->startTransferFile(fileName);
                return;
            }
        }
    }
}
