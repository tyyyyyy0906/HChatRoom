#ifndef HCHATROOMLOGIN_H
#define HCHATROOMLOGIN_H

#include "HChatClientSocket.h"
#include "Global.h"

#include <QWidget>

using namespace GlobalMessage;

namespace Ui {
class HChatRoomLogin;
}

class HChatRoomLogin_;
class HChatRoomLogin : public QWidget
{
    Q_OBJECT
public:
    explicit HChatRoomLogin(QWidget *parent = nullptr);
    ~HChatRoomLogin();

private:
    void initWindow();

private slots:
    void recvClientSocketStatus(const quint8& status);
    void onLoginButtonClicked();

private:
    Ui::HChatRoomLogin *ui;
    HChatClientSocket  *client_;

    friend class HChatRoomLogin_;
    HChatRoomLogin_ *p_;
};

#endif // HCHATROOMLOGIN_H
