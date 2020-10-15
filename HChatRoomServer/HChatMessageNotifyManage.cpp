// (author: sjxnhjp@gmail.com)

#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QApplication>
#include <QDebug>
#include <QTimer>

#include "HChatMessageNotifyManage.h"

const int RIGHT  = 10;
const int BOTTOM = 10;
const int HEIGHT = 60;
const int WIDTH  = 300;
const int SPACE  = 20;

struct NotifyData {
public:
    NotifyData(const QString &icon, const QString &title,
               const QString &body, const QString url) :
               icon(icon), title(title), body(body), url(url){ }

    QString icon, title, body, url;
};

class HChatMessageNotifyManage_ {
public:
    HChatMessageNotifyManage * this_ = Q_NULLPTR;
    QList<HChatMessageNotify *> notifyList;
    QQueue<NotifyData *> dataQueue;

    int maxCount = 8;
    int displayTime = 3 * 1000;
};

HChatMessageNotifyManage::HChatMessageNotifyManage(QObject *parent)
    : QObject(parent)
    , p_(new HChatMessageNotifyManage_) {

    p_->this_ = this;
}

HChatMessageNotifyManage::~HChatMessageNotifyManage() { delete p_; }

void HChatMessageNotifyManage::notify(const QString &title, const QString &body, const QString &icon, const QString url) {
    p_->dataQueue.enqueue(new NotifyData(icon, title, body, url));
    showNext();
}

void HChatMessageNotifyManage::setMaxCount(int count) { p_->maxCount = count; }
void HChatMessageNotifyManage::setDisplayTime(int ms) { p_->displayTime = ms; }

// 调整所有提醒框的位置
void HChatMessageNotifyManage::rearrange() {
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->availableGeometry();
    QPoint bottomRignt = desktopRect.bottomRight();

    QList<HChatMessageNotify*>::iterator i;
    for (i = p_->notifyList.begin(); i != p_->notifyList.end(); ++i) {
        int index = p_->notifyList.indexOf((*i));

        QPoint pos = bottomRignt - QPoint(WIDTH + RIGHT, (HEIGHT + SPACE) * (index + 1) - SPACE + BOTTOM);
        QPropertyAnimation *animation = new QPropertyAnimation((*i), "pos", this);
        animation->setStartValue((*i)->pos());
        animation->setEndValue(pos);
        animation->setDuration(300);
        animation->start();

        connect(animation, &QPropertyAnimation::finished, this, [&]{ animation->deleteLater(); });
    }
}

void HChatMessageNotifyManage::showNext() {
    if (p_->notifyList.size() >= p_->maxCount || p_->dataQueue.isEmpty()) return;

    NotifyData* data = p_->dataQueue.dequeue();
    HChatMessageNotify *notify = new HChatMessageNotify(p_->displayTime);
    notify->setIcon(data->icon);
    notify->setTitle(data->title);
    notify->setBody(data->body);
    notify->setUrl(data->url);
    notify->setFixedSize(WIDTH, HEIGHT);

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->availableGeometry();

    // 计算提醒框的位置
    QPoint bottomRignt = desktopRect.bottomRight();
    QPoint pos = bottomRignt - QPoint(notify->width() + RIGHT, (HEIGHT + SPACE) * (p_->notifyList.size() + 1) - SPACE + BOTTOM);

    notify->move(pos);
    notify->showGriant();
    p_->notifyList.append(notify);

    connect(notify, &HChatMessageNotify::disappeared, this, [notify, this] {
        p_->notifyList.removeAll(notify);
        this->rearrange();

        // 如果列表是满的，重排完成后显示
        if (p_->notifyList.size() == p_->maxCount - 1) {
            QTimer::singleShot(300, this, [this] { this->showNext(); });
        }
        else this->showNext();
        notify->deleteLater();
    });
}
