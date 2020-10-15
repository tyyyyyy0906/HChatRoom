#include "HChatRoomLogin.h"
#include "AppConfig.h"
#include "ui_HChatRoomLogin.h"

using namespace App;

HChatRoomLogin::HChatRoomLogin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomLogin) {

    ui->setupUi(this);
    initWindow();

    this->client_ = new HChatClientSocket(this);
    this->client_->connectServer(AppConfig::conServerAddress, AppConfig::conServerMsgPort);

    connect(client_, SIGNAL(uploadConnectStatus(quint8)), this, SLOT(recvClientSocketStatus(quint8)));
}

HChatRoomLogin::~HChatRoomLogin() {
    delete ui;
}

void HChatRoomLogin::initWindow() {
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    this->setFixedSize(270, 329);
    this->setWindowTitle(QStringLiteral("聊天室"));

    connect(ui->clientLoginButton, SIGNAL(clicked()), this, SLOT(onLoginButtonClicked()));
}

///
/// \brief HChatRoomLogin::recvClientSocketStatus
/// \brief
/// \param status
///
void HChatRoomLogin::recvClientSocketStatus(const quint8 &status) {
    switch(status) {
    case LoginStatus::ConnectedToHost:
        qDebug("Socket已连接");
        break;
    case LoginStatus::DisConnectToHost:
        qDebug("Socket已断开");
        break;
    case LoginStatus::LoginSuccess:
        disconnect(client_, SIGNAL(uploadConnectStatus(quint8)), this, SLOT(recvClientSocketStatus(quint8)));
        qDebug("账号登录成功");
        break;
    case LoginStatus::LoginFailued:
        qDebug("账号登录失败");
        break;
    case LoginStatus::ClientOnline:
        qDebug("客户端上线");
        break;
    case LoginStatus::ClientOffline:
        qDebug("客户端下线");
        break;
    default: break;
    }
}

///
/// \brief HChatRoomLogin::onLoginButtonClicked
/// \brief 登录按钮响应槽函数
///
void HChatRoomLogin::onLoginButtonClicked() {

}
