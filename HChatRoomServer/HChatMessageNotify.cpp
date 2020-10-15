// (author: sjxnhjp@gmail.com)

#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPixmap>
#include <QFontMetrics>
#include <QDesktopServices>
#include <QApplication>

#include "HChatMessageNotify.h"

class HChatMessageNotify_ {
public:
    HChatMessageNotify * this_ = Q_NULLPTR;

    int displayTime;
    QString icon, title, body, url;
    QLabel *backgroundLabel, *iconLabel, *titleLabel, *bodyLabel;
    QPushButton *closeBtn;
};

HChatMessageNotify::HChatMessageNotify(int displayTime, QWidget *parent) :
    QWidget(parent),
    p_(new HChatMessageNotify_) {

    p_->this_ = this;
    p_->displayTime = displayTime;
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint| Qt::Tool | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);

    p_->backgroundLabel = new QLabel(this);
    p_->backgroundLabel->move(0, 0);
    p_->backgroundLabel->setObjectName("notify-background");

    QHBoxLayout *mainLayout = new QHBoxLayout(p_->backgroundLabel);
    QVBoxLayout *contentLayout = new QVBoxLayout();

    p_->iconLabel = new QLabel(p_->backgroundLabel);
    p_->iconLabel->setFixedWidth(40);
    p_->iconLabel->setAlignment(Qt::AlignCenter);

    p_->titleLabel = new QLabel(p_->backgroundLabel);
    p_->titleLabel->setObjectName("notify-title");

    p_->bodyLabel = new QLabel(p_->backgroundLabel);
    p_->bodyLabel->setObjectName("notify-body");
    QFont font = p_->bodyLabel->font();
    font.setPixelSize(8);
    p_->bodyLabel->setFont(font);

    contentLayout->addWidget(p_->titleLabel);
    contentLayout->addWidget(p_->bodyLabel);

    mainLayout->addWidget(p_->iconLabel);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(contentLayout);

    p_->closeBtn = new QPushButton("×", p_->backgroundLabel);
    p_->closeBtn->setObjectName("notify-close-btn");
    p_->closeBtn->setFixedSize(16, 16);

    connect(p_->closeBtn, &QPushButton::clicked, this, [this] { Q_EMIT disappeared(); });
}

HChatMessageNotify::~HChatMessageNotify() { delete p_; }


void HChatMessageNotify::showGriant() {
    this->show();

    p_->titleLabel->setText(p_->title);
    p_->titleLabel->setFont(QFont("微软雅黑"));
    QPixmap tempPix = QPixmap(p_->icon);
    tempPix = tempPix.scaled(QSize(30, 30), Qt::KeepAspectRatio);
    p_->iconLabel->setPixmap(tempPix);

    p_->backgroundLabel->setFixedSize(this->size());
    p_->closeBtn->move(p_->backgroundLabel->width() - p_->closeBtn->width() - 5, 5);

    QFontMetrics elidfont(p_->bodyLabel->font());
    QString text = elidfont.elidedText(p_->body, Qt::ElideRight, p_->bodyLabel->width() - 5);
    p_->bodyLabel->setText(text);
    p_->bodyLabel->setFont(QFont("微软雅黑"));

    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this] {
        animation->deleteLater();
        QTimer::singleShot(p_->displayTime, this, [this] {
            this->hideGriant();
        });
    });

    this->setStyleSheet(
        "#notify-background {"
            "border: 1px solid #fff;"
            "background: #444444;"
            "border-radius: 10px;"
        "} "
        "#notify-title {"
            "font-weight: bold;"
            "color: white;"
            "font-size: 14px;"
        "}"
        "#notify-body {"
            "color: white;"
            "font-size: 12px;"
        "}"
        "#notify-close-btn { "
            "border: 0;"
            "color: white;"
        "}"
        "#notify-close-btn:hover { "
            "background: #444444;"
        "}"
    );
}

void HChatMessageNotify::hideGriant() {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(this->windowOpacity());
    animation->setEndValue(0);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this] {
        this->hide();
        animation->deleteLater();
        Q_EMIT disappeared();
    });
}

void HChatMessageNotify::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (!p_->url.isEmpty()) QDesktopServices::openUrl(p_->url);
        hideGriant();
    }
}

void HChatMessageNotify::setUrl(const QString &value) {
    p_->url = value;
}

void HChatMessageNotify::setBody(const QString &value) {
    p_->body = value;
}

void HChatMessageNotify::setTitle(const QString &value) {
    p_->title = value;
}

void HChatMessageNotify::setIcon(const QString &value) {
    p_->icon = value;
}
