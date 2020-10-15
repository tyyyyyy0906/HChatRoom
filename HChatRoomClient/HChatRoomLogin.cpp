#include "HChatRoomLogin.h"
#include "ui_HChatRoomLogin.h"

HChatRoomLogin::HChatRoomLogin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomLogin) {

    ui->setupUi(this);
    initWindow();

    this->client_ = new HChatClientSocket(this);
    this->client_->connectServer(server_Address_, server_MessagePort_);
}

HChatRoomLogin::~HChatRoomLogin() {
    delete ui;
}

void HChatRoomLogin::initWindow() {
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    this->setFixedSize(270, 329);
}
