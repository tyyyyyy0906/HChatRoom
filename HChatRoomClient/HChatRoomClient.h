#ifndef HCHATROOMCLIENT_H
#define HCHATROOMCLIENT_H

#include <QWidget>
#include <QTimerEvent>
#include <QCloseEvent>
#include <QJsonObject>

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
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString messageConcat  (QString context = "", QString device = "Client") const;
    void parseServerMessage(const QJsonValue& data);

private slots:
    void recvMessage(const int& id_, const QJsonValue& data);
    void recvTcpReply(const quint8& type, const QJsonValue& data);
    void recvTcpStatus();
    void onSendMessageButtonClicked();

    void onSendImageMessage();
    void onSendFileMessage ();
    void onChangedFontSize ();
private:
    Ui::HChatRoomClient *ui;
    friend class HChatRoomClient_;
    HChatRoomClient_ *p_;
};
#endif // HCHATROOMCLIENT_H
