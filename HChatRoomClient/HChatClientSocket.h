#ifndef HCHATCLIENTSOCKET_H
#define HCHATCLIENTSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>

class HChatClientSocket : public QObject {
    Q_OBJECT
public:
    explicit HChatClientSocket(QObject *parent = nullptr);
public:
    void closeSocket();

    void connectServer(const QString& anyIPV4, const int& port);
    void connectServer(const QHostAddress &host, const int &port);

signals:
    void uploadConnectStatus (const quint8& status);
    void uploadCurrentMessage(const quint8& info, const QJsonValue& data);

private:
    void checkLoginStatus(const QJsonValue& data);
    void checkUserOnline (const QJsonValue& data);
    void checkUserOffline(const QJsonValue& data);
    void checkFriendsMsg (const QByteArray& data);

public slots:  ///响应QTcpSocket分发的消息
    ///
    /// \brief onRecvTcpConnected
    /// \brief 响应connected消息
    ///
    void onRecvTcpConnected();
    ///
    /// \brief onRecvTcpDisconnted
    /// \brief 响应disconnected消息
    ///
    void onRecvTcpDisconnted();
    ///
    /// \brief onRecvTcpReadyReadData
    /// \brief 响应readyRead消息
    ///
    void onRecvTcpReadyReadData();
    ///
    /// \brief onMessageTransform
    /// \param type
    /// \param value
    ///
    void onMessageTransform(const quint8& type, const QJsonValue& value);

Q_SIGNALS:
    ///
    /// \brief onNewUserConnected
    /// \brief 有客户连接分发此消息
    ///
    void onNewUserConnected();
    ///
    /// \brief onUserHasDisConnected
    /// \brief 有客户断开分发此消息
    ///
    void onUserHasDisConnected();
    ///
    /// \brief onSendMessageToClient
    /// \brief 分发给指定客户端的消息
    /// \param fileType
    /// \param clientID
    /// \param context
    ///
    void onSendMessageToClient(const quint8& fileType, const int& clientID, const QJsonValue& context);

private:
    QTcpSocket *client_;

    int clientID_;
};

class HChatFileSocket : public QObject {
    Q_OBJECT
public:
    explicit HChatFileSocket(QObject *parent = 0);
    ~HChatFileSocket();

    bool isConneciton();

    // 发送文件大小等信息
    void startTransferFile(QString fileReadName);

    // 连接到服务器
    void connectToServer(const QString &ip, const int &port, const int &usrId);
    // 断开服务器
    void closeConnection();

    // 文件传输完成
    void fileTransFinished();

    // 设置当前socket的id
    void setUserId(const int &id);
signals:
    void signalSendFinished();
    void signamFileRecvOk(const quint8 &type, const QString &filePath);
    void signalUpdateProgress(quint64 currSize, quint64 total);
    void signalConnectd();
private:
    quint64         loadSize;   //每次发送数据的大小

    /************* Receive file *******************/
    quint64         bytesReceived;      //已收到数据的大小
    quint64         fileNameSize;       //文件名的大小信息
    QString         fileReadName;       //存放文件名
    QByteArray      inBlock;            //数据缓冲区
    quint64         ullRecvTotalBytes;  //数据总大小
    QFile           *fileToRecv;        //要发送的文件

    /************* Send file **********************/
    quint16         blockSize;          //存放接收到的信息大小
    QFile           *fileToSend;        //要发送的文件
    quint64         ullSendTotalBytes;  //数据总大小
    quint64         bytesWritten;       //已经发送数据大小
    quint64         bytesToWrite;       //剩余数据大小
    QByteArray      outBlock;           //数据缓冲区，即存放每次要发送的数据

    QString         m_strFilePath;
    QTcpSocket      *m_tcpSocket;

    bool            m_bBusy;
    int             m_nWinId;
    quint8          m_nType;
private:
    void initSocket();

private slots:
    // 显示错误
    void displayError(QAbstractSocket::SocketError);
    // 发送文件数据，更新进度条
    void sltUpdateClientProgress(qint64);

    // 文件接收
    void sltReadyRead();
    void sltConnected();
    void sltDisConnected();
};

#endif // HCHATCLIENTSOCKET_H
