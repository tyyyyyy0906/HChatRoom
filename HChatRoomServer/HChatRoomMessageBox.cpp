#include "HChatRoomMessageBox.h"
#include "AppConfig.h"

#include <QStyleOption>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QShowEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>

using namespace App;

HChatRoomMessageBox::HChatRoomMessageBox(QWidget *parent) {

}

void HChatRoomMessageBox::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

HChatRoomMoveWidget::HChatRoomMoveWidget(QWidget *parent)
    : HChatRoomMessageBox(parent) {
#ifdef Q_OS_WIN32
    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif
}

HChatRoomMoveWidget::~HChatRoomMoveWidget() {

}

void HChatRoomMoveWidget::mouseMoveEvent(QMouseEvent *e) {
    if (m_mousePressed && (e->buttons() && Qt::LeftButton)) {
        this->move(e->globalPos() - mousePoint);
        e->accept();

        if ("MainWindow" == this->objectName()) {
            QPoint pos = e->globalPos() - mousePoint;
            AppConfig::m_nWinX = pos.x();
            AppConfig::m_nWinY = pos.y();
        }
    }
}

void HChatRoomMoveWidget::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        m_mousePressed = true;
        mousePoint = e->globalPos() - this->pos();
        e->accept();
    }
}

void HChatRoomMoveWidget::mouseReleaseEvent(QMouseEvent *) {
    m_mousePressed = false;
}

HChatRoomDialog::HChatRoomDialog(QWidget *parent)
    : QDialog(parent) {
#ifdef Q_OS_WIN32
    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif
}

HChatRoomDialog::~HChatRoomDialog(){ }

void HChatRoomDialog::mouseMoveEvent(QMouseEvent *e) {
    if (m_mousePressed && (e->buttons() && Qt::LeftButton)) {
        this->move(e->globalPos() - mousePoint);
        e->accept();
    }
}

void HChatRoomDialog::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        m_mousePressed = true;
        mousePoint = e->globalPos() - this->pos();
        e->accept();
    }
}

void HChatRoomDialog::mouseReleaseEvent(QMouseEvent *) {
    m_mousePressed = false;
}

HChatRoomBaseDialog::HChatRoomBaseDialog(QWidget *parent)
    : HChatRoomDialog(parent) {
    this->setObjectName("CBaseDialog");
    this->setMinimumSize(300, 150);
    m_nNormalSize = QSize(300, 150);
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 重新构建界面  //
    widgetWinTitle = new QWidget(this);
    widgetWinTitle->setFixedHeight(30);
    widgetWinTitle->setObjectName("widgetWinTitle");

    labelWinIcon = new QLabel(this);
    labelWinIcon->setAlignment(Qt::AlignCenter);
    labelWinIcon->setObjectName("labelWinIcon");
    labelWinIcon->setFixedSize(QSize(30, 30));
    labelWinIcon->setPixmap(QPixmap(":/Server/image/Dialog/robot.png"));

    labelWinTitle = new QLabel(this);
    labelWinTitle->setObjectName("labelWinTitle");
    labelWinTitle->setFixedHeight(30);
    labelWinTitle->setText(tr("自定义系统提示框"));

    btnWinMin   = new QPushButton(this);
    btnWinMin->setObjectName("btnWinMin");
    btnWinMin->setIcon(QIcon(":/Server/image/Common/ic_min_white.png"));

    btnWinClose = new QPushButton(this);
    btnWinClose->setObjectName("btnWinClose");
    btnWinClose->setIcon(QIcon(":/Server/image/Common/ic_close_white.png"));

    QHBoxLayout *horLayoutBtn = new QHBoxLayout();
    horLayoutBtn->setContentsMargins(0, 0, 0, 0);
    horLayoutBtn->setSpacing(2);
    horLayoutBtn->addWidget(btnWinMin);
    horLayoutBtn->addWidget(btnWinClose);

    QHBoxLayout *horLayoutWinTitle = new QHBoxLayout(widgetWinTitle);
    horLayoutWinTitle->setContentsMargins(10, 0, 0, 0);
    horLayoutWinTitle->setSpacing(10);
    horLayoutWinTitle->addWidget(labelWinIcon, 0);
    horLayoutWinTitle->addWidget(labelWinTitle, 1);
    horLayoutWinTitle->addStretch();
    horLayoutWinTitle->addLayout(horLayoutBtn);

    /////////////////////////////////////////////////////////////////////////////
    widgetBody = new QWidget(this);
    widgetBody->setObjectName("widgetBody");

    QVBoxLayout *verLayoutWindow = new QVBoxLayout(this);
    verLayoutWindow->setContentsMargins(2, 2, 2, 2);
    verLayoutWindow->setSpacing(0);
    verLayoutWindow->addWidget(widgetWinTitle);
    verLayoutWindow->addWidget(widgetBody, 1);

    connect(btnWinMin  , SIGNAL(clicked(bool)), this, SLOT(showMinimized()));
    connect(btnWinClose, SIGNAL(clicked(bool)), this, SLOT(close()));
}

HChatRoomBaseDialog::~HChatRoomBaseDialog() {

}

void HChatRoomBaseDialog::setWinIcon(QPixmap pixmap) {
    if (pixmap.isNull()) return;
    if (pixmap.width() > 30 || pixmap.height() > 30) {
        pixmap = pixmap.scaled(30, 30);
    }

    labelWinIcon->setPixmap(pixmap);
}

void HChatRoomBaseDialog::setWinTitle(const QString &text) {
    if (text.isEmpty()) return;
    labelWinTitle->setText(text);
}

HChatRoomSelfMessageBox::HChatRoomSelfMessageBox(QWidget *parent)
    : HChatRoomBaseDialog(parent) {
    labelIcon = new QLabel(widgetBody);
    labelIcon->setFixedSize(QSize(64, 64));
    labelIcon->setAlignment(Qt::AlignCenter);
    labelIcon->setPixmap(QPixmap(":/Server/image/Dialog/ic_info.png"));

    labelMsgContent = new QLabel(widgetBody);
    labelMsgContent->setWordWrap(true);
    labelMsgContent->setMinimumHeight(64);
    labelMsgContent->setObjectName("labelMsgContent");
    labelMsgContent->setStyleSheet(QString("QLabel {font: 12px;}"));
    labelMsgContent->setText(tr("恭喜你，中了500万！"));

    QHBoxLayout *horLayoutContent = new QHBoxLayout();
    horLayoutContent->setContentsMargins(2, 2, 2, 2);
    horLayoutContent->setSpacing(10);
    horLayoutContent->addWidget(labelIcon);
    horLayoutContent->addWidget(labelMsgContent, 1);

    btnCancel = new QPushButton(widgetBody);
    btnCancel->setText(tr("取消"));

    btnOk = new QPushButton(widgetBody);
    btnOk->setText(tr("确定"));

    QHBoxLayout *horLayoutBtn = new QHBoxLayout();
    horLayoutBtn->setContentsMargins(2, 2, 2, 2);
    horLayoutBtn->setSpacing(10);
    horLayoutBtn->addStretch();
    horLayoutBtn->addWidget(btnCancel);
    horLayoutBtn->addWidget(btnOk);

    QVBoxLayout *verLayout = new QVBoxLayout(widgetBody);
    verLayout->setContentsMargins(6, 6, 6, 6);
    verLayout->addLayout(horLayoutContent, 1);
    verLayout->addLayout(horLayoutBtn);

    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnOk    , SIGNAL(clicked(bool)), this, SLOT(accept()));

    // 倒计时定时器
    m_nTimerCnt = 10;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(SltTimerOut()));

    this->setWinTitle(tr("提示"));
    this->setWindowTitle(tr("提示"));
}

HChatRoomSelfMessageBox::~HChatRoomSelfMessageBox() {

}

void HChatRoomSelfMessageBox::showMessage(const QString &content, const quint8 &msgType, const QString &title) {
    if (content.isEmpty()) return;

    if (!title.isEmpty()) this->setWinTitle(title);
    labelMsgContent->setText(content);
    btnCancel->setVisible(HChatRoomSelfMessageBox::E_Question == msgType);
    if (HChatRoomSelfMessageBox::E_Information == msgType)
        labelIcon->setPixmap(QPixmap(":/Server/image/Dialog/ic_info.png"));
    else if (HChatRoomSelfMessageBox::E_Warning == msgType)
        labelIcon->setPixmap(QPixmap(":/Server/image/Dialog/ic_warning.png"));
    else if (HChatRoomSelfMessageBox::E_Question == msgType)
        labelIcon->setPixmap(QPixmap(":/Server/image/Dialog/ic_question.png"));
    else
        labelIcon->setPixmap(QPixmap(":/Server/image/Dialog/ic_info.png"));
}

void HChatRoomSelfMessageBox::startTimer() {
    if (m_timer->isActive())
        m_timer->stop();
    m_timer->start(1000);
}

int HChatRoomSelfMessageBox::infomation(QWidget *parent, const QString &content, const QString &title) {
    HChatRoomSelfMessageBox *messageBox = new HChatRoomSelfMessageBox(parent);
    messageBox->showMessage(content, HChatRoomSelfMessageBox::E_Information, title);
    messageBox->startTimer();
    messageBox->sltTimerOut();
    return messageBox->exec();
}

int HChatRoomSelfMessageBox::question(QWidget *parent, const QString &content, const QString &title) {
    HChatRoomSelfMessageBox *messageBox = new HChatRoomSelfMessageBox(parent);
    messageBox->showMessage(content, HChatRoomSelfMessageBox::E_Question, title);
    return messageBox->exec();
}

int HChatRoomSelfMessageBox::warning(QWidget *parent, const QString &content, const QString &title) {
    HChatRoomSelfMessageBox *messageBox = new HChatRoomSelfMessageBox(parent);
    messageBox->showMessage(content, HChatRoomSelfMessageBox::E_Warning, title);
    return messageBox->exec();
}

void HChatRoomSelfMessageBox::sltTimerOut() {
    btnOk->setText(tr("确定(%1)").arg(m_nTimerCnt--));
    if (m_nTimerCnt < 0) {
        m_timer->stop();
        this->accept();
    }
}

HChatRoomInputDialog::HChatRoomInputDialog(QWidget *parent)
    : HChatRoomBaseDialog(parent) {
    this->setAttribute(Qt::WA_DeleteOnClose, false);

    labelText = new QLabel(widgetBody);
    labelText->setText(tr("输入:"));

    lineEditInput = new QLineEdit(widgetBody);
    lineEditInput->setEchoMode(QLineEdit::Normal);

    btnCancel = new QPushButton(widgetBody);
    btnCancel->setFocusPolicy(Qt::NoFocus);
    btnCancel->setText(tr("取消"));

    btnOk = new QPushButton(widgetBody);
    btnOk->setFocusPolicy(Qt::NoFocus);
    btnOk->setText(tr("确定"));

    QHBoxLayout *horLayoutBtn = new QHBoxLayout();
    horLayoutBtn->setContentsMargins(10, 10, 10, 10);
    horLayoutBtn->setSpacing(10);
    horLayoutBtn->addStretch();
    horLayoutBtn->addWidget(btnCancel);
    horLayoutBtn->addWidget(btnOk);

    QVBoxLayout *verLayout = new QVBoxLayout(widgetBody);
    verLayout->setContentsMargins(10, 10, 10, 10);
    verLayout->addWidget(labelText);
    verLayout->addWidget(lineEditInput);
    verLayout->addLayout(horLayoutBtn);

    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnOk    , SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(lineEditInput, SIGNAL(returnPressed()), this, SLOT(accept()));
}

HChatRoomInputDialog::~HChatRoomInputDialog() {

}

QString HChatRoomInputDialog::getInputText(QWidget *parent, const QString &text, const QString &title, QLineEdit::EchoMode mode) {
    HChatRoomInputDialog dlg(parent);
    dlg.setInputText(text);
    dlg.setWinTitle(title);
    dlg.setEchoMode(mode);

    if (HChatRoomInputDialog::Accepted == dlg.exec()) {
        return dlg.getText();
    }
    return QString();
}

QString HChatRoomInputDialog::getText() const {
    return lineEditInput->text();
}

void HChatRoomInputDialog::setInputText(const QString &text) {
    if (text.isEmpty()) return;
    lineEditInput->setText(text);
    lineEditInput->setFocus();
    lineEditInput->selectAll();
}

void HChatRoomInputDialog::setEchoMode(QLineEdit::EchoMode mode) {
    lineEditInput->setEchoMode(mode);
}
