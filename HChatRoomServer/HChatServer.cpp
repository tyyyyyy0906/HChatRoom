#include "HChatServer.h"
#include "HChatServerSocket.h"
#include "Global.h"
#include "AppConfig.h"
#include "HChatDataBaseMgr.h"

using namespace GlobalMessage;
using namespace App;

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

bool HChatServer::hasListen() const {
    return managerServer_->isListening();
}

///
/// \brief HChatServer::transformMessageHasBeListen
/// \param type
/// \param data
///  转发此消息给服务端，用于监听客户端的消息。
void HChatServer::transformMessageHasBeListen(const quint8& type, const QString& name, const QJsonValue &data) {
    qDebug() << "[HChatServer::transformMessageHasBeListen] 转发监听消息" << type << data.toString() << (int)data.type();
    int type_ = static_cast<int>(data.type());
    switch(type_) {
    case QJsonValue::Null:
        break;
    case QJsonValue::String:
        if (type == MessageGroup::ClientSendMsg) {
            qDebug() << "signalListenClientStatus = " << data.toString();
            emit signalListenClientStatus(name, data.toString());
        }
        break;
    case QJsonValue::Bool:
        break;
    case QJsonValue::Array:
        break;
    case QJsonValue::Object:
        break;
    default:
        break;
    }
}


HChatMsgServer::HChatMsgServer(QObject *parent) {

}

HChatMsgServer::~HChatMsgServer() {

}

///
/// \brief HChatMsgServer::transMessageToAllClient
/// \param type
/// \param data
/// \\\广播服务消息给所有在线客户端
void HChatMsgServer::transMessageToAllClient(const quint8 &type, const QJsonValue &data) {
    qDebug() << "[HChatMsgServer::transMessageToAllClient] 广播服务消息内容 = " << data.toString();
    for (auto it : m_clients) {
        it->replyMessageToClient(type, data);
    }
}

///
/// \brief HChatMsgServer::transFileToClient
/// \param userId
/// \param json
/// \\\ 分发消息给指定客户端
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
//    HChatDataBaseMgr::instance().updateClientStatus(MessageGroup::ClientUserOnLine, client->userClienID());
    Q_EMIT signalUserStatus(QString("用户 [%1] 上线").arg(userName));
    Q_EMIT signalCurrentUserStatus(userName, MessageGroup::ClientUserOnLine);
    p_->notify("新消息", QString("用户%1上线啦！").arg(userName));
    m_clients.push_back(client);
}

void HChatMsgServer::disConnected() {
    HChatServerSocket *client = (HChatServerSocket *)this->sender();
    if (NULL == client) return;
    for (int i = 0; i < m_clients.size(); i++) {
        if (client == m_clients[i]) {
            m_clients.remove(i);
            QString userName = HChatDataBaseMgr::instance().getUserName(client->userClienID());
//            HChatDataBaseMgr::instance().updateClientStatus(MessageGroup::ClientUserOffLine, m_clients[i]->userClienID());
            p_->notify("新消息", QString("用户%1下线啦！").arg(userName));
            transMessageToAllClient(MessageGroup::RequsetAllFriends, {});
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

///
/// \brief HChatMsgServer::msgToClient
/// \param type
/// \param id
/// \param date
/// \\\ 转发消息到对应客户端
void HChatMsgServer::msgToClient(const quint8 &type, const int &id, const QJsonValue &date) {
    qDebug() << "[HChatMsgServer::msgToClient]: 消息转发 = " << date << id;
    QString userName = HChatDataBaseMgr::instance().getUserName(AppConfig::conID_);
    transformMessageHasBeListen(type, userName, date.toObject().value("msg").toString());

    for (int i = 0; i < m_clients.size(); i++) {
        qDebug() << "要发送的客户端id = " << m_clients[i]->userClienID() << id;
        if (id == m_clients[i]->userClienID()) {
            qDebug() << "要发送的实例" << id << m_clients[i]->userClienID() << date;
            m_clients[i]->replyMessageToClient(type, date);
            return;
        }
    }
}

///
/// \brief HChatMsgServer::msgToAllClient
/// \param data
/// \\\ 群发给所有客户端
void HChatMsgServer::msgToAllClient(const quint8 &type, const QJsonValue &data) {

}

HChatFileServer::HChatFileServer(QObject */*parent*/) {}
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
