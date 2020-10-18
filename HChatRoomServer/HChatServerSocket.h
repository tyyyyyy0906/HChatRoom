#ifndef HCHATSERVERSOCKET_H
#define HCHATSERVERSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QJsonObject>
#include <QFile>

class HChatServerSocket : public QObject {
    Q_OBJECT
public:
    explicit HChatServerSocket(QObject *parent = nullptr, QTcpSocket *tcpSocket = NULL);
    ~HChatServerSocket();

public:
    void    closeSocketServer();
    int     userClienID      () const;
    QString getClientAddress () const;
    quint16 getClientPort    () const;

public slots:
    void replyMessageToClient(const quint8& type, const QJsonValue& data);
private slots:
    void onRecvTcpConnected();
    void onRecvTcpDisconnect();
    void onRecvTcpReadyReadData();

private:
    void dealClientRegister(const QJsonValue& data);
    void dealClientLogin   (const QJsonValue& data);
    void dealClientLoginOut(const QJsonValue& data);
    void dealClientOnline  (const QJsonValue& data);
    void dealReplyClientMsg(const QByteArray& data);
    void dealRequsetFriends(const QJsonValue& data);
    void dealClientPicture (const QByteArray& data);

signals:
    void tcpConnected();
    void tcpDisconnected();
    void tcpRecFile(const QJsonValue& data);
    void tcpRefreshFirendList(const QJsonValue& data);
    void tcpTransformMsgToClient(const quint8& type, const int& clineID, const QJsonValue& value);

private:
    QTcpSocket *socket_;
    int id_;
};

class HChatClientFileSocket : public QObject {
    Q_OBJECT
public:
    explicit HChatClientFileSocket(QObject *parent = 0, QTcpSocket *tcpSocket = NULL);
    ~HChatClientFileSocket();

    void close();
    bool checkUserId(const qint32 nId, const qint32 &winId);

    // 文件传输完成
    void fileTransFinished();

    void startTransferFile(QString fileName);
signals:
    void signalConnected();
    void signalDisConnected();
    void signalRecvFinished(int id, const QJsonValue &json);

private:
    quint64    loadSize;
    quint64    bytesReceived;      //已收到数据的大小
    quint64    fileNameSize;       //文件名的大小信息
    QString    fileReadName;       //存放文件名
    QByteArray inBlock;            //数据缓冲区
    quint64    ullRecvTotalBytes;  //数据总大小
    QFile     *fileToRecv;         //要发送的文件

    QTcpSocket *m_tcpSocket;

    /************* Receive file *******************/
    quint16 blockSize;             //存放接收到的信息大小
    QFile *fileToSend;             //要发送的文件
    quint64 ullSendTotalBytes;     //数据总大小
    quint64 bytesWritten;          //已经发送数据大小
    quint64 bytesToWrite;          //剩余数据大小
    QByteArray outBlock;           //数据缓冲区，即存放每次要发送的数据

    bool m_bBusy;

    // 需要转发的用户id
    qint32 m_nUserId;
    // 当前用户的窗口好友的id
    qint32 m_nWindowId;
private:
    void initSocket();
private slots:
    void displayError(QAbstractSocket::SocketError); // 显示错误
    // 文件接收
    void sltReadyRead();
    // 发送
    void sltUpdateClientProgress(qint64 numBytes);
};

#endif // HCHATSERVERSOCKET_H
