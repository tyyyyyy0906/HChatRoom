#ifndef HCHATROOMLOGIN_H
#define HCHATROOMLOGIN_H

#include "HChatClientSocket.h"
#include "Global.h"

#include <QWidget>

using namespace GlobalMessage;

namespace Ui {
class HChatRoomLogin;
}

class HChatRoomLogin : public QWidget
{
    Q_OBJECT
public:
    explicit HChatRoomLogin(QWidget *parent = nullptr);
    ~HChatRoomLogin();

private:
    void initWindow();

private:
    Ui::HChatRoomLogin *ui;
    HChatClientSocket  *client_;
};

#endif // HCHATROOMLOGIN_H
