#include "HChatRoomClient.h"
#include "AppConfig.h"
#include "Global.h"
#include "iconhelper.h"

#include "ui_HChatRoomClient.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>

using namespace GlobalMessage;
using namespace App;

#define SYSTEMTIME QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"))

class HChatRoomClient_ {
public:
    QTcpSocket       * client_  = Q_NULLPTR;
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
    this->setFixedSize(781, 751);
    ui->messageBrowseEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->clientSendMessageEdit->installEventFilter(this);

    ui->clientDataTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ddd"));
    p_->timers_ = startTimer(1000);

    AppConfig::installStyle(this);

    connect(ui->sendButtonClicked, SIGNAL(clicked()), this, SLOT(onSendMessageButtonClicked()));

    connect(ui->messageBrowseEdit, &QTextEdit::customContextMenuRequested, [&](const QPoint&) {
        QMenu  * menu = new QMenu();
        QAction* copy_      = new QAction(QIcon(":/Menu/server_menu_copy.svg"     ), tr("拷贝"), menu);
        QAction* clear_     = new QAction(QIcon(":/Menu/server_menu_delete.svg"   ), tr("清空"), menu);
        QAction* select_All = new QAction(QIcon(":/Menu/server_menu_selectall.svg"), tr("全选"), menu);
        copy_ ->setShortcut(QKeySequence::Copy);
        clear_->setShortcut(QKeySequence::Delete);
        select_All->setShortcut(QKeySequence::SelectAll);

        connect(copy_, &QAction::triggered, ui->messageBrowseEdit, [&]  {
            ui->messageBrowseEdit->copy();
        });
        connect(clear_, &QAction::triggered, ui->messageBrowseEdit, [&] {
            ui->messageBrowseEdit->clear();
        });
        connect(select_All, &QAction::triggered, ui->messageBrowseEdit, [&] {
            ui->messageBrowseEdit->selectAll();
        });

        AppConfig::installStyle(menu);
        menu->addAction(copy_ );
        menu->addAction(clear_);
        menu->addAction(select_All);
        menu->move(QCursor::pos());
        menu->show();
    });

    IconHelper::Instance()->setIcon(ui->clientSendImageButton, QChar(0xe610), 300);
    IconHelper::Instance()->setIcon(ui->clientSendFileButton , QChar(0xe609), 300);
    IconHelper::Instance()->setIcon(ui->clientFontSizeButton , QChar(0xe705), 300);
    IconHelper::Instance()->setIcon(ui->clientColorButton    , QChar(0xe822), 300);
    IconHelper::Instance()->setIcon(ui->sendButtonClicked    , QChar(0xe60d), 300);

    connect(ui->clientSendImageButton, SIGNAL(clicked()), this, SLOT(onSendImageMessage()));
    connect(ui->clientSendFileButton , SIGNAL(clicked()), this, SLOT(onSendFileMessage()));
    connect(ui->clientFontSizeButton , SIGNAL(clicked()), this, SLOT(onChangedFontSize()));
}

HChatRoomClient::~HChatRoomClient() {
    delete ui;
    delete p_;
    p_->client_->abort();
    p_->mSocket_->closeSocket();
}

void HChatRoomClient::connectSocket(HChatClientSocket *socket, const QString &userName) {
    qDebug() << "[HChatRoomClient][connectSocket] 用户名 = " << userName;
    if (NULL != socket) {
        p_->mSocket_ = socket;
        connect(p_->mSocket_, &HChatClientSocket::uploadCurrentMessage, this, &HChatRoomClient::recvTcpReply);
    }
    p_->userName_= userName;
    p_->windowTitle = p_->userName_;
    this->setWindowTitle(p_->windowTitle);
    ui->clientTitleLable->setText(userName);

}

void HChatRoomClient::recvMessage(const int &id_, const QJsonValue &data) {

}

void HChatRoomClient::recvTcpStatus() {

}

///
/// \brief HChatRoomClient::onSendMessageButtonClicked
/// \brief 发送文字消息
void HChatRoomClient::onSendMessageButtonClicked() {
    qDebug() << "[HChatRoomClient][onSendMessageButtonClicked]: Line的内容 = " << ui->clientSendMessageEdit->toPlainText();
    QString content = ui->clientSendMessageEdit->toPlainText();
    ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
    if (content.isEmpty()) {
        ui->messageBrowseEdit->append(SYSTEMTIME);
        return;
    }
    p_->mSocket_->onMessageTransform(MessageGroup::ClientSendMsg, ui->clientSendMessageEdit->toPlainText());
    ui->messageBrowseEdit->append(messageConcat(content));
    ui->clientSendMessageEdit->clear();
}

void HChatRoomClient::onSendImageMessage() {
    QString image_ = QFileDialog::getOpenFileName(this,
                                                  tr("选择图片"),
                                                  "c:",
                                                  tr("图片文件(*.png *.jpg *.jpeg *.bmp *.gif)"));
    ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
    ui->messageBrowseEdit->append(QString("<a style='font-size: 21px; color:white; text-align:right'><a>%1</b></p>").arg(p_->userName_));
    ui->messageBrowseEdit->append(QString("\n"));
    QUrl url(QString("file://%1").arg(image_));
    QImage p_w_picpath = QImageReader(image_ ).read();

    QTextDocument *textDocument = ui->messageBrowseEdit->document();
    textDocument->addResource(QTextDocument::ImageResource, url, QVariant(p_w_picpath));
    QTextCursor cursor = ui->messageBrowseEdit->textCursor();
    QTextImageFormat p_w_picpathFormat;
    p_w_picpathFormat.setWidth((p_w_picpath.width() > 150) ? 100 : p_w_picpath.width());
    p_w_picpathFormat.setHeight((p_w_picpath.height()> 150) ? 100 : p_w_picpath.height());
    p_w_picpathFormat.setName(url.toString());
    cursor.insertImage(p_w_picpathFormat);
    ui->messageBrowseEdit->append(QString("\n"));
}

void HChatRoomClient::onSendFileMessage() {

}

void HChatRoomClient::onChangedFontSize() {

}

///
/// \brief HChatRoomClient::recvTcpReply
/// \param type
/// \param data
/// \\\ 接收服务器分发的消息
void HChatRoomClient::recvTcpReply(const quint8 &type, const QJsonValue &data) {
    switch(type) {
    case MessageGroup::ServerSendMsg:
        qDebug() << "[HChatRoomClient::recvTcpReply]: 接收到服务器分发的消息" << data;
        parseServerMessage(data);
        break;
    case MessageGroup::ClientUserOnLine:
        break;
    case MessageGroup::ClientUserOffLine:
        break;
    default:
        break;
    }
}

void HChatRoomClient::timerEvent(QTimerEvent *event) {
    if (p_->timers_ == event->timerId()) {
        ui->clientDataTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ddd"));
    }
}

void HChatRoomClient::closeEvent(QCloseEvent *event) {
    p_->client_ ->abort();
    p_->mSocket_->closeSocket();
    event->accept();
}

bool HChatRoomClient::eventFilter(QObject *watched, QEvent *event) {
    QKeyEvent   *keys_ = static_cast<QKeyEvent *>(event);
    quint32      type_ = static_cast<quint32>(event->type());

    if (keys_->key() == Qt::Key_Return && type_ == QEvent::KeyPress) {
        if (watched == ui->clientSendMessageEdit) {
            onSendMessageButtonClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

QString HChatRoomClient::messageConcat(QString context, QString device) const {
    QString html = QString("<p style='font-size: 21px; color:white; text-align:right'><b>%1</b></p>").arg(device) +
                   QString("<p style='font-size: 19px; color:pink ; text-align:right'>%2</p>").arg(context);

//    return QString("%1").arg(device != "Client" ? device : p_->userName_) + "\n" + context + "\n";
    return html;
}

void HChatRoomClient::parseServerMessage(const QJsonValue &data) {
    if (data.isString()) {
        ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
        ui->messageBrowseEdit->append(messageConcat(data.toString(), "Server"));
    }
}

