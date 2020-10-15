// (author: sjxnhjp@gmail.com)

#ifndef NOTIFY_H
#define NOTIFY_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

class HChatMessageNotify_;
class HChatMessageNotify : public QWidget {
    Q_OBJECT
public:
    explicit HChatMessageNotify(int displayTime, QWidget *parent = 0);
    ~HChatMessageNotify();

    void setIcon (const QString &value);
    void setTitle(const QString &value);
    void setBody (const QString &value);
    void setUrl  (const QString &value);

    void showGriant();

private:
    void hideGriant();
    void mousePressEvent(QMouseEvent *event);

Q_SIGNALS:
    void disappeared();
private:
    friend class HChatMessageNotify__;
    HChatMessageNotify_ *p_;
};

#endif // NOTIFY_H
