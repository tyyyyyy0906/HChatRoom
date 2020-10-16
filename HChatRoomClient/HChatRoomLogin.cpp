#include "HChatRoomLogin.h"
#include "AppConfig.h"
#include "ui_HChatRoomLogin.h"
#include "HChatRoomClient.h"

#include <QJsonObject>

using namespace App;

class HChatRoomLogin_ {
public:
    bool hasConnected = false;
};

HChatRoomLogin::HChatRoomLogin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomLogin)
    , p_(new HChatRoomLogin_){

    ui->setupUi(this);
    initWindow();

    this->client_ = new HChatClientSocket(this);
    this->client_->connectServer(AppConfig::conServerAddress, AppConfig::conServerMsgPort);

    connect(client_, SIGNAL(uploadConnectStatus(quint8)), this, SLOT(recvClientSocketStatus(quint8)));
}

HChatRoomLogin::~HChatRoomLogin() {
    delete ui;
    delete p_;
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
    qDebug() << "[HChatRoomLogin][recvClientSocketStatus]：接收服务端分发的操作状态消息 = " << status ;
    switch(status) {
    case LoginStatus::ConnectedToHost:
        p_->hasConnected = true;
        qDebug("Socket已连接");
        break;
    case LoginStatus::DisConnectToHost:
        p_->hasConnected = false;
        qDebug("Socket已断开");
        break;
    case LoginStatus::LoginSuccess: {
        disconnect(client_, SIGNAL(uploadConnectStatus(quint8)), this, SLOT(recvClientSocketStatus(quint8)));
        qDebug("账号登录成功");
        AppConfig::conUserName = ui->clientLoginIDEdit->text();
        AppConfig::conPassWord = ui->clientPasswdEdit ->text();
        AppConfig::saveConfig();
        HChatRoomClient *client_ = new HChatRoomClient();
        client_->connectSocket(this->client_, AppConfig::conUserName);
        client_->show();
        this->close();
    }
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
    if (!p_->hasConnected) {
        client_->connectServer(AppConfig::conServerAddress, AppConfig::conServerMsgPort);
        qDebug() << "未能连接服务器";
        return;
    }

    QJsonObject obj_;
    obj_.insert("name"  , ui->clientLoginIDEdit->text());
    obj_.insert("passwd", ui->clientPasswdEdit ->text());
    client_->onMessageTransform(MessageGroup::ClientLogin, obj_);
}
