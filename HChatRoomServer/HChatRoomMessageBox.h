#ifndef HCHATROOMMESSAGEBOX_H
#define HCHATROOMMESSAGEBOX_H

#include <QWidget>
#include <QDialog>
#include <QMutex>
#include <QTimer>
#include <QLineEdit>
#include <QLabel>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QLineEdit;
class QHBoxLayout;
class QVBoxLayout;
QT_END_NAMESPACE

class HChatRoomMessageBox : public QWidget {
    Q_OBJECT
public:
    explicit HChatRoomMessageBox(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *);
};

class HChatRoomMoveWidget : public HChatRoomMessageBox {
    Q_OBJECT
public:
    explicit HChatRoomMoveWidget(QWidget *parent = 0);
    ~HChatRoomMoveWidget();

protected:
    QPoint mousePoint;
    bool m_mousePressed;

    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
};

class HChatRoomDialog : public QDialog {
    Q_OBJECT
public:
    explicit HChatRoomDialog(QWidget *parent = 0);
    ~HChatRoomDialog();

protected:
    QPoint mousePoint;
    bool m_mousePressed;
    QSize m_nNormalSize;

    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
};

class HChatRoomBaseDialog : public HChatRoomDialog {
    Q_OBJECT
public:
    explicit HChatRoomBaseDialog(QWidget *parent = 0);
    ~HChatRoomBaseDialog();

    void setWinIcon(QPixmap pixmap);
    void setWinTitle(const QString &text);

private:
    QWidget    *widgetWinTitle;
    QLabel     *labelWinIcon;
    QLabel     *labelWinTitle;
    QPushButton*btnWinMin;
    QPushButton*btnWinClose;

protected:
    QWidget* widgetBody;

};

class HChatRoomSelfMessageBox : public HChatRoomBaseDialog {
    Q_OBJECT
public:
    typedef enum {
        E_Information =  0x01,
        E_Warning,
        E_Question,
        E_MSGTYPE_Error,
    } E_MSGTYPE;

public:
    explicit HChatRoomSelfMessageBox(QWidget *parent = 0);
    ~HChatRoomSelfMessageBox();

    // 显示消息
    void showMessage(const QString &content, const quint8 &msgType = HChatRoomSelfMessageBox::E_Information, const QString &title = "");

    void startTimer();

    static int infomation(QWidget *parent, const QString &content, const QString &title = "提示");
    static int question  (QWidget *parent, const QString &content, const QString &title = "询问");
    static int warning   (QWidget *parent, const QString &content, const QString &title = "告警");
private:
    QLabel      *labelIcon;
    QLabel      *labelMsgContent;
    QPushButton *btnOk;
    QPushButton *btnCancel;
    QTimer      *m_timer;
    int          m_nTimerCnt;
public slots:
    void sltTimerOut();
};

class HChatRoomInputDialog : public HChatRoomBaseDialog {
    Q_OBJECT
public:
    explicit HChatRoomInputDialog(QWidget *parent = 0);
    ~HChatRoomInputDialog();

    static QString getInputText(QWidget *parent,
                                const QString &text = "",
                                const QString &title = "输入",
                                QLineEdit::EchoMode mode = QLineEdit::Normal);

    QString getText() const;
    void setInputText(const QString &text);

    void setEchoMode(QLineEdit::EchoMode mode);
private:
    static HChatRoomInputDialog *self;

    QLabel      *labelText;
    QLineEdit   *lineEditInput;
    QPushButton *btnOk;
    QPushButton *btnCancel;
};

#endif // HCHATROOMMESSAGEBOX_H
