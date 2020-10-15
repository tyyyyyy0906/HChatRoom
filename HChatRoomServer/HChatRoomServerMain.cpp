// (author: sjxnhjp@gmail.com)

#include "HChatRoomServerMain.h"
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

class HChatRoomServerMain_ {
public:
    HChatRoomServerMain *this_  = Q_NULLPTR;
    QTcpServer          *server_= Q_NULLPTR;
    QTcpSocket          *socket_= Q_NULLPTR;

    QList<QTcpSocket *> mClientList;

    int listernPort = 66666;
};

HChatRoomServerMain::HChatRoomServerMain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HChatRoomServerMain)
    , s_(new HChatRoomServerMain_) {

    ui->setupUi(this);

    s_->this_   = this;
    s_->server_ = new QTcpServer(this);
    s_->socket_ = new QTcpSocket(this);

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

        menu->addAction(copy_ );
        menu->addAction(clear_);
        menu->addAction(select_All);
        menu->move(QCursor::pos());
        menu->show();
    });

    /// socket关联slote
    connect(s_->server_, SIGNAL(newConnection()),this, SLOT(onNewConnection()));
}

HChatRoomServerMain::~HChatRoomServerMain() {
    delete ui;
    delete s_;
}

void HChatRoomServerMain::initHChatRoomServerWindowStyle() {
    this->setWindowTitle(QStringLiteral("服务器"));
    this->setFixedSize(410, 420);
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    ui->serverSendButton->setText(QStringLiteral("发送"));
    ui->serverSendButton->setEnabled(false);
    ui->serverSendEdit  ->installEventFilter(this);
    ui->serverConnectMsgEdit->setReadOnly(true);
    ui->serverConnectMsgEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    scanAllAddressForDevice();
}

void HChatRoomServerMain::onStartServerButton(void) {
    QHostAddress serverAddress = QHostAddress(ui->serverAddressList->currentText());

    if (s_->server_->isListening()) {
        s_->server_->close();
        ui->serverStartButton->setText(QStringLiteral("开启服务"));
        QString msg_ = QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd")) + ":[Server]:\n" + QString("服务关闭");
        ui->serverConnectMsgEdit->append(msg_);
        ui->serverPortEdit->setEnabled(true);
        ui->serverAddressList->setEnabled(true);
        ui->serverSendButton->setEnabled(false);
        return;
    }
    else {
        if (s_->server_->listen(QHostAddress::AnyIPv4, s_->listernPort)) {
            ui->serverStartButton->setText(QStringLiteral("关闭服务"));
            ui->serverPortEdit->setEnabled(false);
            ui->serverAddressList->setEnabled(false);
            ui->serverSendButton->setEnabled(true);
            QString msg_ = QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd")) + "[Server]:\n" + QString("服务开启");
            ui->serverConnectMsgEdit->append(msg_);
            qDebug() << "listen success!!!";
        } else {
            QMessageBox::warning(this, "Server Listen Error", s_->server_->errorString());
        }
    }
}

void HChatRoomServerMain::onNewConnection() {
    QString clientInfo;
    s_->socket_ = s_->server_->nextPendingConnection();
    s_->mClientList.append(s_->socket_);

    clientInfo = s_->socket_->peerAddress().toString() + ":" +  QString::number(s_->socket_->peerPort());
    ui->serverConnectClient->addItem(clientInfo);

    connect(s_->socket_, SIGNAL(readyRead())   , this, SLOT(onReadyReadData()));
    connect(s_->socket_, SIGNAL(disconnected()), this, SLOT(onClientDisconnect()));
}

void HChatRoomServerMain::onClientDisconnect() {
    QMessageBox::information(this, "client close Signal", "client over");
}

void HChatRoomServerMain::onReadyReadData() {
    QByteArray recvArray;
    QTcpSocket* current = nullptr;
    if (!s_->mClientList.isEmpty()) {
        for(int index = 0; index < s_->mClientList.count(); index ++) {
            current = s_->mClientList.at(index);
            if (current->isReadable()) {
                recvArray = current->readAll();
                if (recvArray.isEmpty()) continue;
                QString str = QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd")) +
                                      "[Client]:\n" + str.fromLocal8Bit(recvArray.data());
                ui->serverConnectMsgEdit->append(str);
                break;
            }
        }
        for (int index = 0; index < s_->mClientList.count(); index++) {
            QTcpSocket* temp = s_->mClientList.at(index);
            if (current == temp) continue;
            if (temp->isWritable()) temp->write(recvArray);
        }
    }
}

void HChatRoomServerMain::onSendMessageToClient() {
    QString sendString = ui->serverSendEdit->text();
    QByteArray sendArr = sendString.toLocal8Bit();

    if (!s_->mClientList.isEmpty()) {
        for(int index = 0;index < s_->mClientList.count();index ++) {
            QTcpSocket* temp = s_->mClientList.at(index);
            if (temp->isWritable()) temp->write(sendArr);
        }
    }
    QString str = QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd")) + "[Server]:\n" + sendString;
    ui->serverConnectMsgEdit->append(str);
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

bool HChatRoomServerMain::eventFilter(QObject *watched, QEvent *evt) {
    QKeyEvent   *keys_ = static_cast<QKeyEvent *>(evt);
    quint32      type_ = static_cast<quint32>(evt->type());
    if (type_ == QEvent::KeyPress && keys_->key() == Qt::Key_Return) {
        if (watched == ui->serverSendEdit && s_->server_->isListening()) {
            onSendMessageToClient();
            return true;
        }
    }
    return QWidget::eventFilter(watched, evt);
}
