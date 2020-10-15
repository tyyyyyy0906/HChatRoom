#ifndef HCHATROOMCLIENT_H
#define HCHATROOMCLIENT_H

#include <QWidget>

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

private:
    Ui::HChatRoomClient *ui;
    friend class HChatRoomClient_;
    HChatRoomClient_ *p_;
};
#endif // HCHATROOMCLIENT_H
