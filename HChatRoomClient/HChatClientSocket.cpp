#include "HChatClientSocket.h"
#include "HChatDataBaseMgr.h"
#include "Global.h"

#include <QDebug>
#include <QJsonParseError>

HChatClientSocket::HChatClientSocket(QObject *parent)
    : QObject(parent)
    , client_(new QTcpSocket){

    clientID_ = -1;
    connect(client_, SIGNAL(connected())   , this, SLOT(onRecvTcpConnected()));
    connect(client_, SIGNAL(disconnected()), this, SLOT(onRecvTcpDisconnted()));
    connect(client_, SIGNAL(readyRead())   , this, SLOT(onRecvTcpReadyReadData()));
    connect(client_, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            [](QAbstractSocket::SocketError socketError) {
        qDebug() << "current socket error result = " << socketError;
    });
}

void HChatClientSocket::getStatus() {

}

void HChatClientSocket::closeSocket() {
    client_->abort();
}

void HChatClientSocket::connectServer(const QString& anyIPV4, const int& port) {
    if (client_->state() == QAbstractSocket::UnconnectedState) {
        client_->connectToHost(anyIPV4, static_cast<quint16>(port));
    }
}

void HChatClientSocket::disconnectServer() {

}

void HChatClientSocket::checkLoginStatus(const QJsonValue &data) {
    if (data.isObject()) {
        QJsonObject info_= data.toObject();
        QString strName  = info_.value("name").toString();
        QString strPwd   = info_.value("passwd").toString();
        QJsonObject obj_ = HChatDataBaseMgr::instance().loginCheck(strName, strPwd);

        clientID_ = obj_.value("id").toInt();
        qDebug() << "login" << obj_;

        if (clientID_ > 0) Q_EMIT onNewUserConnected();
        /// 发送查询结果至客户端
        onMessageTransform(GlobalMessage::MessageGroup::ClientLogin, obj_);
    }
}

void HChatClientSocket::checkUserOnline(const QJsonValue &data) {
    if (data.isArray()) {
        QJsonArray arr_ = data.toArray();
        for (int i = 0; i < arr_.size(); ++i) {
            int id_     = arr_.at(i).toInt();
            int status_ = HChatDataBaseMgr::instance().getUserOnlineStatus(id_);
            /// 给在线的好友通报一下状态
            if (GlobalMessage::LoginStatus::ClientOnline == status_) {
                QJsonObject obj_;
                obj_.insert("id", clientID_);
                obj_.insert("text", "online");
                Q_EMIT onSendMessageToClient(GlobalMessage::MessageGroup::ClientUserOnLine, id_, obj_);
            }
        }
    }
}

void HChatClientSocket::checkUserOffline(const QJsonValue &data) {
    if (data.isObject()) {
        QJsonObject obj_ = data.toObject();
        QJsonArray arr_  = obj_.value("friends").toArray();
        int userID_      = obj_.value("id").toInt();
        HChatDataBaseMgr::instance().updateUserStatus(userID_, GlobalMessage::LoginStatus::ClientOffline);

        for (int i = 0; i < arr_.size(); ++i) {
            userID_ = arr_.at(i).toInt();
            int status_ = HChatDataBaseMgr::instance().getUserOnlineStatus(userID_);
            /// 给在线的好友通报一下状态
            if (GlobalMessage::LoginStatus::ClientOnline == status_) {
                QJsonObject info_;
                info_.insert("id", clientID_);
                info_.insert("text", "offline");
                Q_EMIT onSendMessageToClient(GlobalMessage::MessageGroup::ClientUserOffLine, userID_, info_);
            }
        }
    }
}

///
/// \brief HChatClientSocket::checkFriendsMsg
/// \brief 解析Image, Text, File
/// \param data
///
void HChatClientSocket::checkFriendsMsg(const QByteArray &data) {
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(data, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            QJsonObject jsonObj = doucment.object();
            int type_ = jsonObj.value("type").toInt();
            QJsonValue dataVal = jsonObj.value("data");
            QJsonObject dataObj = dataVal.toObject();
            /// 向外转发
            Q_EMIT onSendMessageToClient(type_, dataObj.value("to").toInt(), dataObj);
        }
    }
}

void HChatClientSocket::onRecvTcpConnected() {
    Q_EMIT onNewUserConnected();
}

void HChatClientSocket::onRecvTcpDisconnted() {
    Q_EMIT onUserHasDisConnected();
}

void HChatClientSocket::onRecvTcpReadyReadData() {
//    Q_EMIT onSendMessageToClient()
}

void HChatClientSocket::onMessageTransform(const quint8 &type, const QJsonValue &value) {

}
