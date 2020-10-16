#ifndef HCHATROOMCLIENT_H
#define HCHATROOMCLIENT_H

#include <QWidget>
#include <QTimerEvent>

#include "HChatClientSocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HChatRoomClient; }
QT_END_NAMESPACE

class HChatRoomClient_;
class HChatRoomClient : public QWidget
{
    Q_OBJECT
public:
    HChatRoomClient(QWidget *parent = nullptr);
    ~HChatRoomClient();

    void connectSocket(HChatClientSocket* socket, const QString& userName);

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void recvMessage(const int& id_, const QJsonValue& data);
    void recvTcpStatus();
    void recvTcpReply(const quint8& type, const QJsonValue& data);

private:
    Ui::HChatRoomClient *ui;
    friend class HChatRoomClient_;
    HChatRoomClient_ *p_;
};
#endif // HCHATROOMCLIENT_H
