#include "HChatRoomClient.h"
#include "ui_HChatRoomClient.h"

#include <QTcpSocket>
#include <QSqlDatabase>

class HChatRoomClient_ {
public:
    HChatRoomClient * this_   = Q_NULLPTR;
    QTcpSocket      * client_ = Q_NULLPTR;
};

HChatRoomClient::HChatRoomClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomClient)
    , p_(new HChatRoomClient_) {
    ui->setupUi(this);
    p_->this_ = this;

    p_->client_ = new QTcpSocket(this);
}

HChatRoomClient::~HChatRoomClient() {
    delete ui;
}

