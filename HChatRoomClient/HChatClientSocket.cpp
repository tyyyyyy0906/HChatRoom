#include "HChatClientSocket.h"
#include "HChatDataBaseMgr.h"
#include "Global.h"
#include "AppConfig.h"

#include <QDebug>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>

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
///
/// \brief HChatClientSocket::closeSocket
/// \\\ 关闭Tcp
void HChatClientSocket::closeSocket() {
    if (client_->isOpen()) client_->abort();
}

///
/// \brief HChatClientSocket::connectServer
/// \param anyIPV4
/// \param port
/// \\\ 连接server
void HChatClientSocket::connectServer(const QString& anyIPV4, const int& port) {
    if (client_->state() == QAbstractSocket::UnconnectedState) {
        client_->connectToHost(anyIPV4, static_cast<quint16>(port));
    }
}
///
/// \brief HChatClientSocket::connectServer
/// \param anyIPV4
/// \param port
/// \\\ 连接server
void HChatClientSocket::connectServer(const QHostAddress &anyIPV4, const int &port) {
    if (client_->state() == QAbstractSocket::UnconnectedState) {
        client_->connectToHost(anyIPV4, static_cast<quint16>(port));
    }
}

void HChatClientSocket::checkLoginStatus(const QJsonValue &data) {
    qDebug() << "[HChatClientSocket][checkLoginStatus]：接收服务端分发登录状态消息 = " << data ;
    if (data.isObject()) {
        QJsonObject obj_ = data.toObject();
        int code_        = obj_.value("code").toInt();
        QString msg_     = obj_.value("msg" ).toString();
        if (0 == code_ && msg_ == "ok") {
            clientID_ = obj_.value("id").toInt();
            qDebug() << "current Login id = " << clientID_;
            AppConfig::conID_ = clientID_;
            Q_EMIT uploadConnectStatus(LoginStatus::LoginSuccess);
        }
        else if (-1 == code_){
            Q_EMIT uploadConnectStatus(LoginStatus::LoginFailued);
        }
    }
}

void HChatClientSocket::checkUserOnline(const QJsonValue &data) {
//    if (data.isArray()) {
//        QJsonArray arr_ = data.toArray();
//        for (int i = 0; i < arr_.size(); ++i) {
//            int id_     = arr_.at(i).toInt();
//            int status_ = HChatDataBaseMgr::instance().getUserOnlineStatus(id_);
//            /// 给在线的好友通报一下状态
//            if (GlobalMessage::LoginStatus::ClientOnline == status_) {
//                QJsonObject obj_;
//                obj_.insert("id", clientID_);
//                obj_.insert("text", "online");
//                Q_EMIT onSendMessageToClient(GlobalMessage::MessageGroup::ClientUserOnLine, id_, obj_);
//            }
//        }
//    }
}

void HChatClientSocket::checkUserOffline(const QJsonValue &data) {
//    if (data.isObject()) {
//        QJsonObject obj_ = data.toObject();
//        QJsonArray arr_  = obj_.value("friends").toArray();
//        int userID_      = obj_.value("id").toInt();
//        HChatDataBaseMgr::instance().updateUserStatus(userID_, GlobalMessage::LoginStatus::ClientOffline);

//        for (int i = 0; i < arr_.size(); ++i) {
//            userID_ = arr_.at(i).toInt();
//            int status_ = HChatDataBaseMgr::instance().getUserOnlineStatus(userID_);
//            /// 给在线的好友通报一下状态
//            if (GlobalMessage::LoginStatus::ClientOnline == status_) {
//                QJsonObject info_;
//                info_.insert("id", clientID_);
//                info_.insert("text", "offline");
//                Q_EMIT onSendMessageToClient(GlobalMessage::MessageGroup::ClientUserOffLine, userID_, info_);
//            }
//        }
//    }
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
            Q_EMIT onSendMessageToClient(type_, dataObj.value("to").toInt(), dataObj);
        }
    }
}

///
/// \brief HChatClientSocket::onRecvTcpConnected
/// \\\ 接收TCP链接的消息
void HChatClientSocket::onRecvTcpConnected() {
    Q_EMIT onNewUserConnected();
    Q_EMIT uploadConnectStatus(LoginStatus::ConnectedToHost);
}
///
/// \brief HChatClientSocket::onRecvTcpDisconnted
/// \\\ 获取Tcp断开消息
void HChatClientSocket::onRecvTcpDisconnted() {
    Q_EMIT onUserHasDisConnected();
    Q_EMIT uploadConnectStatus(LoginStatus::DisConnectToHost);
}

///
/// \brief HChatClientSocket::onRecvTcpReadyReadData
/// \brief 读取服务端发送的数据请求
///
void HChatClientSocket::onRecvTcpReadyReadData() {
    qDebug() << "[HChatClientSocket][onRecvTcpReadyReadData]：接收服务端分发状态";
    QByteArray byRead = client_->readAll();
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(byRead, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            QJsonObject obj_= doucment.object();
            QJsonValue  val_= obj_.value("data");
            int types_      = obj_.value("type").toInt();

            qDebug() << "[HChatClientSocket][onRecvTcpReadyReadData]：data = " << val_ << types_;

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
                qDebug() << "接收服务器分发的图片消息" << val_;
                Q_EMIT uploadCurrentMessage(MessageGroup::ClientSendPicture, val_);
                break;
            case MessageGroup::ServerSendMsg:
                qDebug() << "接收服务器广播分发的消息" << val_;
                Q_EMIT uploadCurrentMessage(types_, val_);
                break;
            case MessageGroup::RequsetAllFriends:
                Q_EMIT uploadCurrentMessage(MessageGroup::RequsetAllFriends, val_);
                break;
            default:
                break;
            }
        }
    }
}

void HChatClientSocket::onMessageTransform(const quint8 &type, const QJsonValue &value) {
    qDebug() << "[HChatClientSocket][onMessageTransform]: 接受到要转发的消息 type = " << type << clientID_ << value;
    if (!client_->isOpen()) {
        client_->connectToHost(AppConfig::conServerAddress, AppConfig::conServerMsgPort);
        client_->waitForConnected(1000);
    }
    if (!client_->isOpen()) return;

    QJsonObject obj_;
    obj_.insert("type", type);
    obj_.insert("from", clientID_);
    obj_.insert("data", value);

    qDebug() << "[HChatClientSocket::onMessageTransform] 数据组装的结果 = " << obj_;

    QJsonDocument document;
    document.setObject(obj_);
    client_->write(document.toJson(QJsonDocument::Compact));
}

HChatFileSocket::HChatFileSocket(QObject *parent) {
    m_nType = MessageGroup::ClientLogin;
    m_strFilePath = AppConfig::conRecvFilePath;
    initSocket();
}

HChatFileSocket::~HChatFileSocket() {

}

bool HChatFileSocket::isConneciton() {
    return m_tcpSocket->isOpen();
}

void HChatFileSocket::setUserId(const int &id) {
    m_nWinId = id;
}

void HChatFileSocket::initSocket() {
    loadSize         = 50 * 1024;
    ullSendTotalBytes= 0;
    ullRecvTotalBytes= 0;
    bytesWritten     = 0;
    bytesToWrite     = 0;
    bytesReceived    = 0;
    m_nWinId         = -1;
    fileNameSize     = 0;
    m_bBusy          = false;

    fileToSend = new QFile(this);
    fileToRecv = new QFile(this);

    m_tcpSocket = new QTcpSocket(this);

    // 当有数据发送成功时，我们更新进度条
    connect(m_tcpSocket, SIGNAL(bytesWritten(qint64)),
            this       , SLOT(sltUpdateClientProgress(qint64)));
    // 当有数据接收成功时，我们更新进度条
    connect(m_tcpSocket, SIGNAL(readyRead())   , this, SLOT(sltReadyRead()));
    connect(m_tcpSocket, SIGNAL(connected())   , this, SLOT(sltConnected()));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(sltDisConnected()));
}

void HChatFileSocket::displayError(QAbstractSocket::SocketError) {

}

void HChatFileSocket::sltUpdateClientProgress(qint64) {

}

void HChatFileSocket::sltReadyRead() {
    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_5_0);

    // 如果接收到的数据小于等于20个字节，那么是刚开始接收数据，我们保存为头文件信息
    if (bytesReceived <= (sizeof(qint64)*2)) {
        int nlen = sizeof(qint64) * 2;
        // 接收数据总大小信息和文件名大小信息
        if ((m_tcpSocket->bytesAvailable() >= nlen) && (fileNameSize == 0)) {
            in >> ullRecvTotalBytes >> fileNameSize;
            if (0 != ullRecvTotalBytes) bytesReceived += nlen;
        }

        // 接收文件名，并建立文件
        if((m_tcpSocket->bytesAvailable() >= (qint64)fileNameSize) &&
                ((qint64)fileNameSize != 0) &&
                (0 != ullRecvTotalBytes)) {
            in >> fileReadName;
            fileReadName = AppConfig::conRecvFilePath + fileReadName;
            bytesReceived += fileNameSize;

            fileToRecv->setFileName(fileReadName);

            if (!fileToRecv->open(QFile::WriteOnly | QIODevice::Truncate)) {
                qDebug() << "recv file open failed" << fileReadName;
                return;
            }

            qDebug() << "Begin to recv file" << m_nWinId << fileReadName;
        }
    }

    // 如果接收的数据小于总数据，那么写入文件
    if (bytesReceived < ullRecvTotalBytes) {
        bytesReceived += m_tcpSocket->bytesAvailable();
        inBlock = m_tcpSocket->readAll();
        if (fileToRecv->isOpen()) fileToRecv->write(inBlock);
        inBlock.resize(0);
    }

    // 更新进度条
    Q_EMIT signalUpdateProgress(bytesReceived, ullRecvTotalBytes);

    // 接收数据完成时
    if ((bytesReceived >= ullRecvTotalBytes) && (0 != ullRecvTotalBytes)) {
        fileToRecv->close();
        bytesReceived     = 0;
        ullRecvTotalBytes = 0;
        fileNameSize      = 0;

        Q_EMIT signamFileRecvOk(Text, fileToRecv->fileName());
        qDebug() << "File recv ok" << fileToRecv->fileName();
        // 数据接受完成
        fileTransFinished();
    }
}

void HChatFileSocket::sltConnected() {
    m_nType = MessageGroup::ClientLogin;
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_0);

    // 给服务器socket上报自己的id，方便下次查询
    sendOut << qint32(AppConfig::conID_) << qint32(m_nWinId);
    m_tcpSocket->write(outBlock);
    Q_EMIT signalConnectd();
}

void HChatFileSocket::sltDisConnected() {
    if (m_tcpSocket->isOpen()) m_tcpSocket->close();
}

void HChatFileSocket::startTransferFile(QString fileReadName) {
    if (m_bBusy) return;
    m_nType = MessageGroup::ClientSendFile;

    /// 如果没有连接服务器，重新连接下
    if (!m_tcpSocket->isOpen()) {
        connectToServer(AppConfig::conServerAddress, AppConfig::conServerFilePort, m_nWinId);
    }

    /// 要发送的文件
    fileToSend = new QFile(fileReadName);

    if (!fileToSend->open(QFile::ReadOnly)) {
        qDebug() << "open file error!";
        return;
    }

    /// 文件总大小
    ullSendTotalBytes = fileToSend->size();

    /// 文件数据流
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_8);

    /// 当前文件名，不包含路径
    QString currentFileName = fileReadName.right(fileReadName.size() - fileReadName.lastIndexOf('/')-1);

    /// 依次写入总大小信息空间，文件名大小信息空间，文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;

    /// 这里的总大小是文件名大小等信息和实际文件大小的总和
    ullSendTotalBytes += outBlock.size();

    /// 返回outBolock的开始，用实际的大小信息代替两个qint64(0)空间
    sendOut.device()->seek(0);
    sendOut << ullSendTotalBytes << qint64((outBlock.size() - sizeof(qint64)*2));

    /// 发送完头数据后剩余数据的大小
    bytesToWrite = ullSendTotalBytes - m_tcpSocket->write(outBlock);

    outBlock.resize(0);
    m_bBusy = true;
}

void HChatFileSocket::connectToServer(const QString &ip, const int &port, const int &usrId) {
    if (m_tcpSocket->isOpen()) return;
    m_nWinId = usrId;
    m_tcpSocket->connectToHost(QHostAddress(ip), port);
}

void HChatFileSocket::closeConnection() {
    fileTransFinished();
    m_tcpSocket->abort();
}

void HChatFileSocket::fileTransFinished() {
    m_bBusy = false;
    ullSendTotalBytes= 0;
    ullRecvTotalBytes= 0;
    bytesWritten     = 0;
    bytesToWrite     = 0;
    bytesReceived    = 0;
    fileNameSize     = 0;
}
