// (author: sjxnhjp@gmail.com)

#include "HChatRoomServerMain.h"
#include "HChatServer.h"
#include "AppConfig.h"
#include "Global.h"
#include "ui_HChatRoomServerMain.h"

#include <QEvent>
#include <QMouseEvent>
#include <QList>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QTcpServer>
#include <QSpinBox>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QDateTime>
#include <QVector>
#include <QTextEdit>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QApplication>

using namespace App;
using namespace GlobalMessage;

#define SYSTEMTIME QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"))

class HChatRoomServerMain_ {
public:
    int listernPort = 66666;

    HChatMsgServer  *messageServer = Q_NULLPTR;
    HChatFileServer *sendFileServer= Q_NULLPTR;
    QSystemTrayIcon *systemTrayIcon= Q_NULLPTR;
};

HChatRoomServerMain::HChatRoomServerMain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomServerMain)
    , s_(new HChatRoomServerMain_) {

    ui->setupUi(this);
    s_->messageServer = new HChatMsgServer (this);
    s_->sendFileServer= new HChatFileServer(this);
    s_->systemTrayIcon= new QSystemTrayIcon(this);

    this->initHChatRoomServerWindowStyle();

    /// ui控件关联slote
    connect(ui->serverStartButton, SIGNAL(clicked()), this, SLOT(onStartServerButton()));
    connect(ui->serverSendButton , SIGNAL(clicked()), this, SLOT(onSendMessageToClient()));
    connect(ui->serverPortEdit   , QOverload<int>::of(&QSpinBox::valueChanged), [=](int i) {
        s_->listernPort = i; qDebug("current port changed\t%d", i);
    });

    connect(ui->serverConnectMsgEdit, &QTextEdit::customContextMenuRequested, [&](const QPoint&) {
        QMenu  * menu = new QMenu();
        QAction* copy_      = new QAction(QIcon(":/Server/image/Menu/server_menu_copy.svg"     ), tr("拷贝"), menu);
        QAction* clear_     = new QAction(QIcon(":/Server/image/Menu/server_menu_delete.svg"   ), tr("清空"), menu);
        QAction* select_All = new QAction(QIcon(":/Server/image/Menu/server_menu_selectall.svg"), tr("全选"), menu);
        copy_ ->setShortcut(QKeySequence::Copy);
        clear_->setShortcut(QKeySequence::Delete);
        select_All->setShortcut(QKeySequence::SelectAll);

        connect(copy_, &QAction::triggered, ui->serverConnectMsgEdit, [&]  {
            ui->serverConnectMsgEdit->copy();
        });
        connect(clear_, &QAction::triggered, ui->serverConnectMsgEdit, [&] {
            ui->serverConnectMsgEdit->clear();
        });
        connect(select_All, &QAction::triggered, ui->serverConnectMsgEdit, [&] {
            ui->serverConnectMsgEdit->selectAll();
        });

        AppConfig::installStyle(menu);
        menu->addAction(copy_ );
        menu->addAction(clear_);
        menu->addAction(select_All);
        menu->move(QCursor::pos());
        menu->show();
    });

    /// socket关联slote
//    connect(s_->server_, SIGNAL(newConnection()),this, SLOT(onNewConnection()));
}

HChatRoomServerMain::~HChatRoomServerMain() {
    delete ui;
    delete s_;
}

void HChatRoomServerMain::initHChatRoomServerWindowStyle() {
    this->setWindowTitle(QStringLiteral("服务器"));
    this->setFixedSize(520, 440);
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    ui->serverSendButton->setText(QStringLiteral("发送"));
    ui->serverSendButton->setEnabled(false);
    ui->serverSendEdit  ->setEnabled(false);
    ui->serverSendEdit  ->installEventFilter(this);
    ui->serverConnectMsgEdit->setReadOnly(true);
    ui->serverConnectMsgEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->serverConnectClient ->item(0)->setSelected(false);

    s_->systemTrayIcon->setIcon(QIcon(":/Server/image/TrayIcon/server_tray_icon.svg"));
    QMenu *tray_ = new QMenu(this);
    tray_->addAction(QIcon(":/Server/image/TrayIcon/server_tray_show.svg"), tr("显示"));
    tray_->addAction(QIcon(":/Server/image/TrayIcon/server_quit.svg"     ), tr("退出"));
    s_->systemTrayIcon->setContextMenu(tray_);
    s_->systemTrayIcon->show();

    connect(tray_, &QMenu::triggered, [this](QAction* a) {
        if (a->text() == "退出") {
            s_->messageServer->closeListen();
            qApp->quit();
        } else if (a->text() == "显示") this->show();
    });


    connectSignal();
    scanAllAddressForDevice();
}

void HChatRoomServerMain::connectSignal() {
    connect(s_->messageServer, &HChatMsgServer::signalListenClientStatus, [&](const QString& name, const QJsonValue& data) {
        QString info_ = messageConcat("Client" ,QString("用户%1: ").arg(name) + data.toString());
        ui->serverConnectMsgEdit->append(info_);
    });
    connect(s_->messageServer, &HChatMsgServer::signalCurrentUserStatus, [&](const QString& name, const quint8& op) {
        if (op == MessageGroup::ClientUserOnLine) {
            ui->serverConnectClient->addItem(name);
        } else if (op == MessageGroup::ClientUserOffLine) {
            for (int i = 0; i < ui->serverConnectClient->count(); i++) {
                if (ui->serverConnectClient->item(i)->text() == name) {
                    QListWidgetItem *item = ui->serverConnectClient->takeItem(i);
                    delete item;
                    break;
                }
            }
        }
    });
}

void HChatRoomServerMain::onStartServerButton(void) {
    bool message_ = s_->messageServer ->startListen(66666);
    bool file_    = s_->sendFileServer->startListen(66667);
    ui->serverConnectMsgEdit->append(messageConcat(QString(message_ ? "消息端口监听成功" : "消息端口监听失败")));
    ui->serverConnectMsgEdit->append(messageConcat(QString(file_    ? "文件端口监听成功" : "文件端口监听失败")));

    if (message_ && file_) {
        ui->serverSendButton ->setEnabled(true);
        ui->serverSendEdit   ->setEnabled(true);
    }

    connect(s_->messageServer, &HChatMsgServer::signalUserStatus, s_->messageServer, [&](const QString& data) {
        ui->serverConnectMsgEdit->append(messageConcat(data));
    });
}

///
/// \brief HChatRoomServerMain::onSendMessageToClient
/// \\\ 广播服务消息给所有客户端
void HChatRoomServerMain::onSendMessageToClient() {
    QString data_ = ui->serverSendEdit->text();
    ui->serverConnectMsgEdit->append(messageConcat(data_.isEmpty() ? "" : data_));
    s_->messageServer->transMessageToAllClient(MessageGroup::ServerSendMsg, data_);
    ui->serverSendEdit->clear();

}

void HChatRoomServerMain::scanAllAddressForDevice() {
    QList<QHostAddress> allAddress_ = QNetworkInterface::allAddresses();

    foreach(const QHostAddress& item, allAddress_) {
        if (item.protocol() == QAbstractSocket::IPv4Protocol) {
            ui->serverAddressList->addItem(item.toString());
        }
    }
}

QString HChatRoomServerMain::messageConcat(QString device, QString context) const {
    return SYSTEMTIME + QString("[ %1 ] :").arg(device) + context;
}

bool HChatRoomServerMain::eventFilter(QObject *watched, QEvent *evt) {
    QKeyEvent   *keys_ = static_cast<QKeyEvent *>(evt);
    quint32      type_ = static_cast<quint32>(evt->type());
    if (type_ == QEvent::KeyPress && keys_->key() == Qt::Key_Return) {
        if (watched == ui->serverSendEdit  &&
            s_->messageServer ->hasListen()&&
            s_->sendFileServer->hasListen()) {
            onSendMessageToClient();
            return true;
        }
    }
    return QWidget::eventFilter(watched, evt);
}

void HChatRoomServerMain::closeEvent(QCloseEvent *e) {
#if 1
    this->hide();
    e->ignore();
#else
    QWidget::closeEvent(e);
#endif
}
