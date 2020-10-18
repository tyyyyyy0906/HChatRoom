#include "HChatServerSocket.h"
#include "Global.h"
#include "HChatDataBaseMgr.h"
#include "AppConfig.h"

#include <QJsonParseError>
#include <QDataStream>
#include <QJsonDocument>

using namespace GlobalMessage;
using namespace App;

HChatServerSocket::HChatServerSocket(QObject *parent, QTcpSocket *tcpSocket)
    : QObject(parent) {

    if (tcpSocket == NULL) socket_ = new QTcpSocket(this);
    socket_ = tcpSocket;
    id_ = -1;
    connect(socket_, SIGNAL(connected())   , this, SLOT(onRecvTcpConnected()));
    connect(socket_, SIGNAL(disconnected()), this, SLOT(onRecvTcpDisconnect()));
    connect(socket_, SIGNAL(readyRead())   , this, SLOT(onRecvTcpReadyReadData()));
}

HChatServerSocket::~HChatServerSocket() { }

int HChatServerSocket::userClienID() const {
    return id_;
}

QString HChatServerSocket::getClientAddress() const {
    return socket_->peerAddress().toString();
}

quint16 HChatServerSocket::getClientPort() const {
    return socket_->peerPort();
}

void HChatServerSocket::closeSocketServer() {
    socket_->abort();
}

void HChatServerSocket::replyMessageToClient(const quint8 &type, const QJsonValue &data) {
    qDebug() << "[HChatServerSocket][replyMessageToClient]：转发消息给请求的客户端" << type << data;
    if (!socket_->isOpen()) {
        qDebug() << "[HChatServerSocket][replyMessageToClient]：服务器未开启";
        return;
    }
    QJsonObject info_;
    info_.insert("type", type);
    info_.insert("from", id_ );
    info_.insert("data", data);

    qDebug() << "[HChatServerSocket][replyMessageToClient]: 数据组装Json" << info_;
    QJsonDocument document;
    document.setObject(info_);
    socket_->write(document.toJson(QJsonDocument::Compact));
}

void HChatServerSocket::onRecvTcpConnected() {
    qDebug() << "检测到客户端接入";
    HChatDataBaseMgr::instance().updateClientStatus(id_, MessageGroup::ClientUserOnLine);
    Q_EMIT tcpConnected();
}

void HChatServerSocket::onRecvTcpDisconnect() {
    qDebug() << "检测到客户端断开";
    HChatDataBaseMgr::instance().updateClientStatus(id_, MessageGroup::ClientUserOffLine);
    Q_EMIT tcpDisconnected();
}

void HChatServerSocket::onRecvTcpReadyReadData() {
    qDebug() << "[HChatServerSocket::onRecvTcpReadyReadData] 准备处理接收到客户端的数据";
    QByteArray reply = socket_->readAll();
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(reply, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            QJsonObject obj_= doucment.object();
            int type_       = obj_.value("type").toInt();
            QJsonValue data = obj_.value("data");

            qDebug() << "[HChatServerSocket][onRecvTcpReadyReadData]：接收客户端的数据 = " << type_ << data;

            switch (type_) {
            case MessageGroup::ClientRegister:
                /// 处理注册（功能未开放）
                qDebug("处理客户端注册");
                dealClientRegister(data);
                break;
            case MessageGroup::ClientLogin:
                qDebug("处理客户端登录");
                dealClientLogin(data);
                break;
            case MessageGroup::ClientUserOnLine:
                qDebug("处理客户端上线");
                dealClientOnline(data);
                break;
            case MessageGroup::ClientLoginOut:
                qDebug("处理客户端下线");
                dealClientLoginOut(data);
                Q_EMIT tcpDisconnected();
                break;
            case MessageGroup::ClientSendMsg:
                qDebug("处理客户端消息");
                dealReplyClientMsg(reply);
                break;
            case MessageGroup::ClientSendFile:
                break;
            case MessageGroup::ClientSendPicture:
                dealClientPicture(reply);
                break;
            case MessageGroup::DownLoadFile:
                Q_EMIT tcpRecFile(data);
                break;
            case MessageGroup::RequsetAllFriends:
                dealRequsetFriends(data);
                break;
            default:
                break;
            }
        }
    }
}


void HChatServerSocket::dealClientRegister(const QJsonValue &/*data*/) { }

///
/// \brief HChatServerSocket::dealClientLogin
/// 查询数据中要登录的用户数据是否存在
/// \param data
///
void HChatServerSocket::dealClientLogin(const QJsonValue &data) {
    qDebug() << "[HChatServerSocket][dealClientLogin]：处理客户端的登录请求 = " << data ;
    QJsonObject info_;
    if (data.isObject()) {
        QJsonObject info_ = data.toObject();
        QString userName  = info_.value("name"  ).toString();
        QString userPassWd= info_.value("passwd").toString();

        qDebug() << "[HChatServerSocket][dealClientLogin]：处理客户端的登录请求数据："
                 << "userName = " << userName << "userPassWd = " << userPassWd;
        info_ = HChatDataBaseMgr::instance().checkUserInfoLogin(userName, userPassWd);
        qDebug() << "[HChatServerSocket][dealClientLogin]: 查询数据结果 = " << info_;
        id_ = info_.value("id").toInt();
        if (id_ > 0) Q_EMIT tcpConnected();
        replyMessageToClient(MessageGroup::ClientLogin, info_);
    }
}

void HChatServerSocket::dealClientLoginOut(const QJsonValue &data) {
    if (data.isObject()) {
        QJsonObject obj_ = data.toObject();
        QJsonArray array = obj_.value("friends").toArray();
        int          nId = obj_.value("id").toInt();
        HChatDataBaseMgr::instance().updateClientStatus(nId, MessageGroup::ClientUserOffLine);
        for (int i = 0; i < array.size(); ++i) {
            nId = array.at(i).toInt();
            int nStatus = HChatDataBaseMgr::instance().userOnLineStatus(nId);
            if (MessageGroup::ClientUserOnLine == nStatus) {
                QJsonObject info_;
                info_.insert("id", id_);
                info_.insert("text", "offline");
                Q_EMIT tcpTransformMsgToClient(MessageGroup::ClientUserOffLine, nId, info_);
            }
        }
    }
}

void HChatServerSocket::dealClientOnline(const QJsonValue &data) {
    if (data.isArray()) {
        QJsonArray array_ = data.toArray();
        for (int i = 0; i < array_.size(); ++i) {
            int nID = array_[i].toInt();
            int nStatus = HChatDataBaseMgr::instance().userOnLineStatus(nID);
            if (MessageGroup::ClientUserOnLine == nStatus) {
                QJsonObject ob_;
                ob_.insert("id", nID);
                ob_.insert("text", "online");
                Q_EMIT tcpTransformMsgToClient(MessageGroup::ClientUserOnLine, nID, ob_);
            }
        }
    }
}

void HChatServerSocket::dealReplyClientMsg(const QByteArray &data) {
    qDebug() << "[HChatServerSocket][dealReplyClientMsg] 解析客户端发送的消息" << data;
    QJsonParseError error_;
    QJsonDocument document = QJsonDocument::fromJson(data, &error_);
    if (document.isNull() || error_.error != QJsonParseError::NoError) return;
    if (document.isObject()) {
        QJsonObject ob_  = document.object();
        int type_        = ob_.value("type").toInt();
        QJsonObject data_= ob_.value("data").toObject();
        int cid_         = ob_.value("from").toInt();
        int from_        = ob_.value("to").toInt();

        AppConfig::conID_= cid_;

        QString message_ = data_.value("msg").toString();
        QString sender_  = data_.value("sender").toString();

        QJsonObject info = {
            { "msg"   , message_ },
            { "sender", sender_  }
        };
        int     target_  = data_.value("to").toInt();

        qDebug() << "[HChatServerSocket][dealReplyClientMsg] 解析数据结果 = "
                 << ob_ << type_ << message_ << cid_ << from_ << target_;
        Q_EMIT tcpTransformMsgToClient(MessageGroup::ClientSendMsg, target_, info);
    }
}

void HChatServerSocket::dealRequsetFriends(const QJsonValue &data) {
    qDebug() << "[HChatServerSocket::dealRequsetFriends] 处理好友请求 = " << data;
    QJsonObject info_ = data.toObject();
    int clientId_   = info_.value("from").toInt();
    int msgTypes_   = info_.value("type").toInt();
    Q_UNUSED(clientId_)
    Q_UNUSED(msgTypes_)
    QString userName= info_.value("data").toString();
    QJsonObject userInfo_ = HChatDataBaseMgr::instance().getFriends();
    replyMessageToClient(MessageGroup::RequsetAllFriends, userInfo_);
}

void HChatServerSocket::dealClientPicture(const QByteArray &data) {
    qDebug() << "[HChatServerSocket][dealClientPicture] 解析客户端发送的图片消息" << data;
    QJsonParseError error_;
    QJsonDocument document = QJsonDocument::fromJson(data, &error_);
    if (document.isNull() || error_.error != QJsonParseError::NoError) return;
    if (document.isObject()) {
        QJsonObject info_ = document.object();
        int from = info_.value("from" ).toInt();
        int type = info_.value("type").toInt();
        Q_UNUSED(from)
        Q_UNUSED(type)
        QJsonValue msg = info_.value("data");
        int to   = msg.toObject().value("to").toInt();
        Q_EMIT tcpTransformMsgToClient(MessageGroup::ClientSendPicture, to, msg);
    }
}

HChatClientFileSocket::HChatClientFileSocket(QObject *parent, QTcpSocket *tcpSocket) {
    loadSize         = 50 * 1024;
    ullSendTotalBytes= 0;
    ullRecvTotalBytes= 0;
    bytesWritten     = 0;
    bytesToWrite     = 0;
    bytesReceived    = 0;
    fileNameSize     = 0;
    m_bBusy          = false;
    m_nUserId        = -1;
    m_nWindowId      = -1;
    fileToSend       = new QFile(this);
    fileToRecv       = new QFile(this);

    if (tcpSocket == NULL) m_tcpSocket = new QTcpSocket(this);
    m_tcpSocket = tcpSocket;

    connect(m_tcpSocket, SIGNAL(readyRead())         , this, SLOT(sltReadyRead()));
    connect(m_tcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sltUpdateClientProgress(qint64)));
    connect(m_tcpSocket, SIGNAL(disconnected())      , this, SIGNAL(signalDisConnected()));
}

HChatClientFileSocket::~HChatClientFileSocket() {}

void HChatClientFileSocket::close() {}

bool HChatClientFileSocket::checkUserId(const qint32 nId, const qint32 &winId) {
    return ((m_nUserId == nId) && (m_nWindowId == winId));
}

void HChatClientFileSocket::fileTransFinished() {
    ullSendTotalBytes   = 0;
    ullRecvTotalBytes   = 0;
    bytesWritten        = 0;
    bytesToWrite        = 0;
    bytesReceived       = 0;
    fileNameSize        = 0;
    m_bBusy             = false;
}

void HChatClientFileSocket::startTransferFile(QString fileName) {
    if (m_bBusy) return;
    if (!m_tcpSocket->isOpen()) return;

    // 要发送的文件
    fileToSend = new QFile((-2 == m_nWindowId ? AppConfig::conRecvHeadPath : AppConfig::conRecvFilePath) + fileName);

    if (!fileToSend->open(QFile::ReadOnly)) {
        qDebug() << "open file error!";
        return;
    }

    ullSendTotalBytes = fileToSend->size();

    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_8);

    QString currentFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);

    sendOut << qint64(0) << qint64(0) << currentFileName;

    ullSendTotalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut << ullSendTotalBytes << qint64((outBlock.size() - sizeof(qint64)*2));
    bytesToWrite = ullSendTotalBytes - m_tcpSocket->write(outBlock);
    outBlock.resize(0);
    m_bBusy = true;
    qDebug() << "开始发送文件" << fileName << m_nUserId << m_nWindowId;
}

void HChatClientFileSocket::initSocket() {
    loadSize            = 50 * 1024;
    ullSendTotalBytes   = 0;
    ullRecvTotalBytes   = 0;
    bytesWritten        = 0;
    bytesToWrite        = 0;
    bytesReceived       = 0;
    fileNameSize        = 0;
    m_bBusy = false;

    fileToSend = new QFile(this);
    fileToRecv = new QFile(this);

    m_tcpSocket = new QTcpSocket(this);

    // 当有数据发送成功时，我们更新进度条
    connect(m_tcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sltUpdateClientProgress(qint64)));
    connect(m_tcpSocket, SIGNAL(readyRead())         , this, SLOT(sltReadyRead()));
    connect(m_tcpSocket, SIGNAL(connected())         , this, SLOT(sltConnected()));
    connect(m_tcpSocket, SIGNAL(disconnected())      , this, SLOT(sltDisConnected()));
}

void HChatClientFileSocket::displayError(QAbstractSocket::SocketError) {
    m_tcpSocket->abort();
}

void HChatClientFileSocket::sltReadyRead() {
    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_5_0);

    if (0 == bytesReceived && (-1 == m_nUserId) && (-1 == m_nWindowId) &&
       (m_tcpSocket->bytesAvailable() == (sizeof(qint32) * 2))) {
        in >> m_nUserId >> m_nWindowId;
        qDebug() << "获取客户端ID = " << m_nUserId << m_nWindowId;
        Q_EMIT signalConnected();
        return;
    }
    if (bytesReceived <= (sizeof(qint64)*2)) {
        int nlen = sizeof(qint64) * 2;
        /// 接收数据总大小信息和文件名大小信息
        if ((m_tcpSocket->bytesAvailable() >= nlen) && (fileNameSize == 0)) {
            in >> ullRecvTotalBytes >> fileNameSize;
            if (0 != ullRecvTotalBytes) bytesReceived += nlen;
        }

        /// 接收文件名，并建立文件
        if ((m_tcpSocket->bytesAvailable() >= (qint64)fileNameSize   ) &&
               ((qint64)fileNameSize != 0) && (0 != ullRecvTotalBytes)) {
            in >> fileReadName;
            bytesReceived += fileNameSize;

            fileToRecv->setFileName(AppConfig::conRecvFilePath + fileReadName);
            if (!fileToRecv->open(QFile::WriteOnly | QIODevice::Truncate)) {
                qDebug() << "文件打开失败" << fileReadName;
                return;
            }
        }
    }

    /// 如果接收的数据小于总数据，那么写入文件
    if (bytesReceived < ullRecvTotalBytes) {
        bytesReceived += m_tcpSocket->bytesAvailable();
        inBlock = m_tcpSocket->readAll();
        if (fileToRecv->isOpen()) fileToRecv->write(inBlock);
        inBlock.resize(0);
    }

    /// 接收数据完成时
    if ((bytesReceived >= ullRecvTotalBytes) && (0 != ullRecvTotalBytes)) {
        fileToRecv->close();
        bytesReceived = 0;
        ullRecvTotalBytes = 0;
        fileNameSize = 0;
        fileTransFinished();
    }
}

void HChatClientFileSocket::sltUpdateClientProgress(qint64 numBytes) {
    /// 已经发送数据的大小
    bytesWritten += (int) numBytes;
    /// 如果已经发送了数据
    if (bytesToWrite > 0) {
        /// 每次发送loadSize大小的数据，这里设置为4KB，如果剩余的数据不足4KB，就发送剩余数据的大小
        outBlock = fileToSend->read(qMin(bytesToWrite, loadSize));

        /// 发送完一次数据后还剩余数据的大小
        bytesToWrite -= (int)m_tcpSocket->write(outBlock);

        /// 清空发送缓冲区
        outBlock.resize(0);
    }
    else {
        /// 如果没有发送任何数据，则关闭文件
        if (fileToSend->isOpen())
            fileToSend->close();
    }

    /// 发送完毕
    if (bytesWritten >= ullSendTotalBytes) {
        if (fileToSend->isOpen()) fileToSend->close();
        bytesWritten      = 0;
        ullSendTotalBytes = 0;
        bytesToWrite      = 0;
        fileTransFinished();
    }
}
