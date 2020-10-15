// (author: sjxnhjp@gmail.com)

#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include <QObject>
#include <QQueue>

#include "HChatMessageNotify.h"

class HChatMessageNotifyManage_;
class HChatMessageNotifyManage : public QObject {
    Q_OBJECT
public:
    explicit HChatMessageNotifyManage( QObject *parent = 0);
    ~HChatMessageNotifyManage();

    void notify(const QString &title, const QString &body,
                const QString &icon = ":/Server/image/Message/message_icon.png", const QString url = "");
    void setMaxCount(int count);
    void setDisplayTime(int ms);

private:
    void rearrange();
    void showNext();

private:
    friend class HChatMessageNotifyManage_;
    HChatMessageNotifyManage_ *p_;
};

#endif // NOTIFYMANAGER_H
