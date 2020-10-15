#include "HChatClientSocket.h"
#include "HChatDataBaseMgr.h"
#include "Global.h"
#include "AppConfig.h"

#include <QDebug>
#include <QJsonParseError>

using namespace GlobalMessage;
using namespace App;

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
        QJsonObject obj_ = data.toObject();
        int code_        = obj_.value("code").toInt();
        QString msg_     = obj_.value("msg" ).toString();

        if (0 == code_ && msg_ == "ok") {
            clientID_ = obj_.value("id").toInt();
            AppConfig::conID_ = clientID_;
            Q_EMIT uploadConnectStatus(LoginStatus::LoginSuccess);
        }
        else if (-1 == code_){
            Q_EMIT uploadConnectStatus(LoginStatus::LoginFailued);
        }
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
    Q_EMIT uploadConnectStatus(LoginStatus::ConnectedToHost);
}

void HChatClientSocket::onRecvTcpDisconnted() {
    Q_EMIT onUserHasDisConnected();
    Q_EMIT uploadConnectStatus(LoginStatus::DisConnectToHost);
}

void HChatClientSocket::onRecvTcpReadyReadData() {
    QByteArray byRead = client_->readAll();
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(byRead, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            QJsonObject obj_= doucment.object();
            QJsonValue  val_= obj_.value("data");
            int types_      = obj_.value("type").toInt();

            switch (types_){
            case MessageGroup::ClientRegister: break;
            case MessageGroup::ClientLogin: checkLoginStatus(val_); break;
            case MessageGroup::ClientUserOnLine:
                Q_EMIT uploadCurrentMessage(MessageGroup::ClientUserOnLine, val_);
                break;
            case MessageGroup::ClientUserOffLine:
                Q_EMIT uploadCurrentMessage(MessageGroup::ClientUserOffLine, val_);
                break;
            case MessageGroup::ClientLoginOut:
                client_->abort();
                break;
            case MessageGroup::ClientSendMsg:
                Q_EMIT uploadCurrentMessage(MessageGroup::ClientSendMsg, val_);
                break;
            case MessageGroup::ClientSendFile:
                Q_EMIT uploadCurrentMessage(MessageGroup::ClientSendFile, val_);
                break;
            case MessageGroup::ClientSendPicture:
                Q_EMIT uploadCurrentMessage(MessageGroup::ClientSendPicture, val_);
                break;
            default:
                break;
            }
        }
    }
}

void HChatClientSocket::onMessageTransform(const quint8 &type, const QJsonValue &value) {

}
