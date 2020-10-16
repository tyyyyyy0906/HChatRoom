#include "HChatRoomClient.h"
#include "ui_HChatRoomClient.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>

class HChatRoomClient_ {
public:
    QTcpSocket      * client_   = Q_NULLPTR;
    HChatClientSocket* mSocket_ = Q_NULLPTR;

    QString userName_, windowTitle;
    int timers_;
};

HChatRoomClient::HChatRoomClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomClient)
    , p_(new HChatRoomClient_) {
    ui->setupUi(this);

    p_->client_ = new QTcpSocket(this);

    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    this->setFixedSize(564, 474);

    ui->clientDataTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ddd"));
    p_->timers_ = startTimer(1000);
}

HChatRoomClient::~HChatRoomClient() {
    delete ui;
    delete p_;
}

void HChatRoomClient::connectSocket(HChatClientSocket *socket, const QString &userName) {
    qDebug() << "[HChatRoomClient][connectSocket] 用户名 = " << userName;
    p_->mSocket_ = socket;
    p_->userName_= userName;
    p_->windowTitle = p_->userName_ + "聊天室";
    this->setWindowTitle(p_->windowTitle);
    ui->clientTitleLable->setText(userName);
}

void HChatRoomClient::timerEvent(QTimerEvent *event) {
    if (p_->timers_ == event->timerId())
        ui->clientDataTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ddd"));
}

void HChatRoomClient::recvMessage(const int &id_, const QJsonValue &data) {

}

void HChatRoomClient::recvTcpStatus() {

}

void HChatRoomClient::recvTcpReply(const quint8 &type, const QJsonValue &data) {

}

