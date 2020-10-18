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
#include <QMap>

using namespace GlobalMessage;
using namespace App;

#define SYSTEMTIME QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"))

class HChatRoomClient_ {
public:
    QTcpSocket       * client_  = Q_NULLPTR;
    HChatClientSocket* mSocket_ = Q_NULLPTR;
    HChatFileSocket  * fSocket_ = Q_NULLPTR;

    QString userName_, windowTitle, friendsName;
    int timers_;

    QMap<QString, QJsonArray> infomation;
};

HChatRoomClient::HChatRoomClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomClient)
    , p_(new HChatRoomClient_) {
    ui->setupUi(this);

    p_->client_ = new QTcpSocket(this);
    p_->fSocket_= new HChatFileSocket(this);

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
    connect(this        , SIGNAL(signalSendMessage(quint8, QJsonValue)),
            p_->mSocket_, SLOT(onMessageTransform(quint8, QJsonValue)));
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
        connect(p_->mSocket_, &HChatClientSocket::uploadCurrentMessage, this        , &HChatRoomClient::recvTcpReply);
        connect(this        , &HChatRoomClient::sendFriendsCheck      , p_->mSocket_, &HChatClientSocket::onMessageTransform);
    }
    p_->userName_= userName;
    p_->windowTitle = p_->userName_;
    this->setWindowTitle(p_->windowTitle);

    Q_EMIT sendFriendsCheck(MessageGroup::RequsetAllFriends, p_->userName_);
}

void HChatRoomClient::recvMessage(const int &/*id_*/, const QJsonValue &/*data*/) { }
void HChatRoomClient::recvTcpStatus() {}

///
/// \brief HChatRoomClient::onSendMessageButtonClicked
/// \brief 发送文字消息
void HChatRoomClient::onSendMessageButtonClicked() {
    qDebug() << "[HChatRoomClient][onSendMessageButtonClicked]: Line的内容 = " << ui->clientSendMessageEdit->toPlainText();
    QString content = ui->clientSendMessageEdit->toPlainText();
    if (content.isEmpty()) {
        QString html = QString("<p style='font-size: 19px; color:pink; text-align:right;'>%2</p>").arg(SYSTEMTIME);
        ui->messageBrowseEdit->append(html);
        ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
        return;
    }

    QJsonObject info_;
    QJsonArray  friendId = p_->infomation.find(p_->friendsName).value();
    info_.insert("from"  , AppConfig::conID_);
    info_.insert("to"    , friendId[0]);
    info_.insert("type"  , MessageGroup::ClientSendMsg);
    info_.insert("msg"   , ui->clientSendMessageEdit->toPlainText());
    info_.insert("sender", p_->userName_);

    p_->mSocket_->onMessageTransform(MessageGroup::ClientSendMsg, info_);
    QString html = QString("<p style='font-size: 21px; color:white; text-align:right'><b>%1</b></p>").arg(p_->userName_);
    QString html2= QString("<p style='font-size: 19px; color:pink ; text-align:right'>%2</p>").arg(content);
    QString html1= QString("<p style='font-size: 14px; color:gray ; text-align:center'>%3</p>").arg(SYSTEMTIME);
    ui->messageBrowseEdit->append(html1);
    ui->messageBrowseEdit->setAlignment(Qt::AlignCenter);
    ui->messageBrowseEdit->append(html);
    ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
    ui->messageBrowseEdit->append(html2);
    ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
    ui->clientSendMessageEdit->clear();
}

void HChatRoomClient::onSendImageMessage() {
    QString image_ = QFileDialog::getOpenFileName(this,
                                                  tr("选择图片"),
                                                  "c:",
                                                  tr("图片文件(*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (image_.isEmpty()) return;
    QJsonObject data_;
    QJsonArray  friendId = p_->infomation.find(p_->friendsName).value();

    data_.insert("id"  , AppConfig::conID_);
    data_.insert("to"  , friendId[0]      );
    data_.insert("msg" , image_           );
    data_.insert("type", Picture          );

    qDebug() << "组装图片数据JSON = " << data_;

    p_->fSocket_->startTransferFile(image_);
    p_->mSocket_->onMessageTransform(MessageGroup::ClientSendPicture, data_);
    QString html1= QString("<p style='font-size: 14px; color:gray ; text-align:center'>%3</p>").arg(SYSTEMTIME);
    ui->messageBrowseEdit->append(html1);
    ui->messageBrowseEdit->setAlignment(Qt::AlignCenter);
    ui->messageBrowseEdit->append(QString("<a style='font-size: 21px; color:white; text-align:right'><a>%1</b></p>").arg(p_->userName_));
    ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
    clientDealPicture(image_, false);
    ui->messageBrowseEdit->setAlignment(Qt::AlignRight);
    ui->messageBrowseEdit->append(QString("<p></p>"));
}

///
/// \brief HChatRoomClient::onSendFileMessage
/// \\\ 文件下发处理槽函数
void HChatRoomClient::onSendFileMessage() {
    static QString filePath = AppConfig::conRecvFilePath;
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("选择文件"),
                                                filePath,
                                                tr("文件(*)"));

}

///
/// \brief HChatRoomClient::onChangedFontSize
/// \\\ 调整字体槽函数
void HChatRoomClient::onChangedFontSize() { /* DO Nothing */ }
///
/// \brief HChatRoomClient::recvTcpReply
/// \param type
/// \param data
/// \\\ 接收服务器分发的消息
void HChatRoomClient::recvTcpReply(const quint8 &type, const QJsonValue &data) {
    switch(type) {
    /// 接收到服务器分发的服务广播消息
    case MessageGroup::ServerSendMsg:
        qDebug() << "[HChatRoomClient::recvTcpReply]: 接收到服务器分发的消息" << data;
        parseServerMessage(data);
        break;
    /// 接收分发的上线消息
    case MessageGroup::ClientUserOnLine:
        Q_EMIT sendFriendsCheck(MessageGroup::RequsetAllFriends, p_->userName_);
        break;
    /// 接收分发的下线消息
    case MessageGroup::ClientUserOffLine:
        Q_EMIT sendFriendsCheck(MessageGroup::RequsetAllFriends, p_->userName_);
        break;
    /// 接收分发的好友列表消息
    case MessageGroup::RequsetAllFriends:
        parseServerFriends(data);
        break;
    /// 接收分发的图片消息
    case MessageGroup::ClientSendPicture:
        qDebug() << "接收到来自另一个客户端的消息" << data;
        parseServerPicture(data);
        break;
    /// 接收服务器分发的其他客户端消息
    case MessageGroup::ClientSendMsg:
        qDebug() << "接收到来自另一个客户端的消息" << data;
        parseServerMessage(data);
        break;
    default:
        break;
    }
}

///
/// \brief HChatRoomClient::timerEvent
/// \param event
/// \\\ 重载Core timerEvent方法
void HChatRoomClient::timerEvent(QTimerEvent *event) {
    if (p_->timers_ == event->timerId()) {
        ui->clientDataTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ddd"));
    }
}

///
/// \brief HChatRoomClient::closeEvent
/// \param event
/// \\\ 重载closeEvent
void HChatRoomClient::closeEvent(QCloseEvent *event) {
    p_->client_ ->abort();
    p_->mSocket_->closeSocket();
    event->accept();
    qApp->quit();
}

///
/// \brief HChatRoomClient::eventFilter
/// \param watched
/// \param event
/// \return
/// \\\ 重载eventFilter方法
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

    return html;
}

void HChatRoomClient::clientDealPicture(const QString &path, bool status) {
    ui->messageBrowseEdit->append(QString("<p></p>"));
    ui->messageBrowseEdit->setAlignment(status ? Qt::AlignLeft : Qt::AlignRight);
    QUrl url(QString("file://%1").arg(path));
    QImage p_w_picpath = QImageReader(path ).read();
    QTextDocument *textDocument = ui->messageBrowseEdit->document();
    textDocument->addResource(QTextDocument::ImageResource, url, QVariant(p_w_picpath));
    QTextCursor cursor = ui->messageBrowseEdit->textCursor();
    QTextImageFormat p_w_picpathFormat;
    p_w_picpathFormat.setWidth((p_w_picpath.width() > 150) ? 100 : p_w_picpath.width());
    p_w_picpathFormat.setHeight((p_w_picpath.height()> 150) ? 100 : p_w_picpath.height());
    p_w_picpathFormat.setName(url.toString());
    cursor.insertImage(p_w_picpathFormat);

    ui->messageBrowseEdit->append(QString("<p></p>"));
    ui->messageBrowseEdit->setAlignment(status ? Qt::AlignLeft : Qt::AlignRight);
}

///
/// \brief HChatRoomClient::parseServerMessage
/// \param data
/// \\\ 解析服务端广播的消息
void HChatRoomClient::parseServerMessage(const QJsonValue &data) {
    QString sender= "Server", context;
    if (data.isObject()) {
        QJsonObject info_ = data.toObject();
        if (info_.contains("sender")) {
            sender = info_.value("sender").toString();
        }
        context = info_.value("msg").toString();
//        QString html1= QString("<p style='font-size: 14px; color:gray ; text-align:center'>%3</p>").arg(SYSTEMTIME);
//        QString html2= QString("<p style='font-size: 21px; color:white; text-align:left'><b>%1</b></p>").arg(sender);
//        QString html3= QString("<p style='font-size: 19px; color:pink ; text-align:left'>%2</p>").arg(context);
//        ui->messageBrowseEdit->append(html1);
//        ui->messageBrowseEdit->setAlignment(Qt::AlignCenter);
//        ui->messageBrowseEdit->append(html2);
//        ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
//        ui->messageBrowseEdit->append(html3);
//        ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
    } else if (data.isString()) {
        sender = "Server";
        context = data.toString();
    }

    QString html1= QString("<p style='font-size: 14px; color:gray ; text-align:center'>%3</p>").arg(SYSTEMTIME);
    QString html2= QString("<p style='font-size: 21px; color:white; text-align:left'><b>%1</b></p>").arg(sender);
    QString html3= QString("<p style='font-size: 19px; color:pink ; text-align:left'>%2</p>").arg(context);
    ui->messageBrowseEdit->append(html1);
    ui->messageBrowseEdit->setAlignment(Qt::AlignCenter);
    ui->messageBrowseEdit->append(html2);
    ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
    ui->messageBrowseEdit->append(html3);
    ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
}

///
/// \brief HChatRoomClient::parseServerFriends
/// \param data
/// \\\ 获取好友状态
void HChatRoomClient::parseServerFriends(const QJsonValue &data) {
    if (data.isObject()) {
        QJsonObject ob_ = data.toObject();
        QStringList key = ob_.keys();

        for (auto it : key) {
            p_->infomation.insert(it, ob_.value(it).toArray());
            if (it == p_->userName_) continue;
            p_->friendsName = it;
            QJsonArray result = ob_.value(it).toArray();
            QString info_ = (result[1] == MessageGroup::ClientUserOnLine) ?
                            (it + "\t\t" + "在线") : (it + "\t\t" + "离线");
            ui->clientFriendList->addItem(info_);
        }
    }
}

///
/// \brief HChatRoomClient::parseServerPicture
/// \param data
/// \\\ 解析服务器发来的图片显示
void HChatRoomClient::parseServerPicture(const QJsonValue &data) {
    qDebug() << "[HChatRoomClient::parseServerPicture] 解析服务器发来的图片显示 = " << data;
    if (data.isObject()) {
        QString fileName = data.toObject().value("msg").toString();
        QString html = QString("<p style='font-size: 21px; color:white; text-align:left'><b>%1</b></p>").arg(p_->friendsName);
        QString html1= QString("<p style='font-size: 14px; color:gray ; text-align:center'>%3</p>").arg(SYSTEMTIME);
        ui->messageBrowseEdit->append(html1);
        ui->messageBrowseEdit->setAlignment(Qt::AlignCenter);
        ui->messageBrowseEdit->append(html);
        ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
        clientDealPicture(fileName, true);
        ui->messageBrowseEdit->setAlignment(Qt::AlignLeft);
    }
}
