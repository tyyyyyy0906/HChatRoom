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

HChatServerSocket::~HChatServerSocket() {

}

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

    qDebug() << "[HChatServerSocket][replyMessageToClient]: 数据组装结构" << info_;
    // 将数据发给指定客户端
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
    qDebug() << "准备接收到客户端的数据";
    QByteArray reply = socket_->readAll();
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(reply, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            // 转化为对象
            QJsonObject obj_= doucment.object();
            int type_       = obj_.value("type").toInt();
            QJsonValue data = obj_.value("data");

            qDebug() << "[HChatServerSocket][onRecvTcpReadyReadData]：接收客户端的数据 = " << type_ << data;

            switch (type_) {
            case MessageGroup::ClientRegister: {
                /// 处理注册（未添加）
                qDebug("处理客户端注册");
                dealClientRegister(data);
            }
                break;
            case MessageGroup::ClientLogin: {
                qDebug("处理客户端登录");
                dealClientLogin(data);
            }
                break;
            case MessageGroup::ClientUserOnLine: {
                qDebug("处理客户端上线");
                dealClientOnline(data);
            }
                break;
            case MessageGroup::ClientLoginOut: {
                qDebug("处理客户端下线");
                dealClientLoginOut(data);
                Q_EMIT tcpDisconnected();
                socket_->abort();
            }
                break;
            case MessageGroup::ClientSendMsg:
            case MessageGroup::ClientSendFile:
            case MessageGroup::ClientSendPicture: {
                qDebug("处理客户端消息");
                dealReplyClientMsg(reply);
            }
                break;
            case MessageGroup::DownLoadFile : {
                Q_EMIT tcpRecFile(data);
            }
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

}

void HChatServerSocket::dealClientOnline(const QJsonValue &data) {

}

void HChatServerSocket::dealReplyClientMsg(const QByteArray &data) {
    qDebug() << "[HChatServerSocket][dealReplyClientMsg] 解析客户端发送的消息" << data;
    QJsonParseError error_;
    QJsonDocument document = QJsonDocument::fromJson(data, &error_);
    if (document.isNull() || error_.error != QJsonParseError::NoError) return;
    if (document.isObject()) {
        QJsonObject ob_  = document.object();
        int type_        = ob_.value("type").toInt();
        QJsonValue data_ = ob_.value("data");
        int id_          = ob_.value("from").toInt();
        qDebug() << "[HChatServerSocket][dealReplyClientMsg] 解析数据结果 = "
                 << ob_ << type_ << data_ << id_;
        Q_EMIT tcpTransformMsgToClient(type_, id_, data_);
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

HChatClientFileSocket::~HChatClientFileSocket() {

}

void HChatClientFileSocket::close() {

}

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

    ullSendTotalBytes = fileToSend->size(); // 文件总大小

    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_8);

    QString currentFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);

    // 依次写入总大小信息空间，文件名大小信息空间，文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;

    // 这里的总大小是文件名大小等信息和实际文件大小的总和
    ullSendTotalBytes += outBlock.size();

    // 返回outBolock的开始，用实际的大小信息代替两个qint64(0)空间
    sendOut.device()->seek(0);
    sendOut << ullSendTotalBytes << qint64((outBlock.size() - sizeof(qint64)*2));

    // 发送完头数据后剩余数据的大小
    bytesToWrite = ullSendTotalBytes - m_tcpSocket->write(outBlock);

    outBlock.resize(0);
    m_bBusy = true;
    qDebug() << "Begin to send file" << fileName << m_nUserId << m_nWindowId;
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

    // 连接时的消息
    if (0 == bytesReceived && (-1 == m_nUserId) && (-1 == m_nWindowId) &&
       (m_tcpSocket->bytesAvailable() == (sizeof(qint32) * 2))) {
        // 保存ID，方便发送文件
        in >> m_nUserId >> m_nWindowId;
        qDebug() << "获取客户端ID = " << m_nUserId << m_nWindowId;
        Q_EMIT signalConnected();
        return;
    }

    // 如果接收到的数据小于等于20个字节，那么是刚开始接收数据，我们保存为头文件信息
    if (bytesReceived <= (sizeof(qint64)*2)) {
        int nlen = sizeof(qint64) * 2;
        // 接收数据总大小信息和文件名大小信息
        if ((m_tcpSocket->bytesAvailable() >= nlen) && (fileNameSize == 0)) {
            in >> ullRecvTotalBytes >> fileNameSize;
            if (0 != ullRecvTotalBytes) bytesReceived += nlen;
        }

        // 接收文件名，并建立文件
        if ((m_tcpSocket->bytesAvailable() >= (qint64)fileNameSize   ) &&
               ((qint64)fileNameSize != 0) && (0 != ullRecvTotalBytes)) {
            in >> fileReadName;
            bytesReceived += fileNameSize;

            fileToRecv->setFileName((-2 == m_nWindowId ? AppConfig::conRecvHeadPath : AppConfig::conRecvFilePath) + fileReadName);

            if (!fileToRecv->open(QFile::WriteOnly | QIODevice::Truncate)) {
                qDebug() << "文件打开失败" << fileReadName;
                return;
            }
            qDebug() << "准备接收文件" << fileReadName;
        }
    }

    //如果接收的数据小于总数据，那么写入文件
    if (bytesReceived < ullRecvTotalBytes) {
        bytesReceived += m_tcpSocket->bytesAvailable();
        inBlock = m_tcpSocket->readAll();

        if (fileToRecv->isOpen())
            fileToRecv->write(inBlock);

        inBlock.resize(0);
    }

    // 接收数据完成时
    if ((bytesReceived >= ullRecvTotalBytes) && (0 != ullRecvTotalBytes)) {
        fileToRecv->close();
        bytesReceived = 0;
        ullRecvTotalBytes = 0;
        fileNameSize = 0;
        qDebug() << "文件接收完成" << fileToRecv->fileName();
        // 数据接受完成
        fileTransFinished();
    }
}

void HChatClientFileSocket::sltUpdateClientProgress(qint64 numBytes) {
    // 已经发送数据的大小
    bytesWritten += (int) numBytes;
    // 如果已经发送了数据
    if (bytesToWrite > 0) {
        // 每次发送loadSize大小的数据，这里设置为4KB，如果剩余的数据不足4KB，就发送剩余数据的大小
        outBlock = fileToSend->read(qMin(bytesToWrite, loadSize));

        // 发送完一次数据后还剩余数据的大小
        bytesToWrite -= (int)m_tcpSocket->write(outBlock);

        // 清空发送缓冲区
        outBlock.resize(0);
    }
    else {
        // 如果没有发送任何数据，则关闭文件
        if (fileToSend->isOpen())
            fileToSend->close();
    }

    // 发送完毕
    if (bytesWritten >= ullSendTotalBytes) {
        if (fileToSend->isOpen())
            fileToSend->close();
        bytesWritten      = 0;
        ullSendTotalBytes = 0;
        bytesToWrite      = 0;
        qDebug() << "发送成功" << fileToSend->fileName();
        fileTransFinished();
    }
}
