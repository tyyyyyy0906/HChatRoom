// (author: sjxnhjp@gmail.com)

#ifndef HCHATROOMSERVERMAIN_H
#define HCHATROOMSERVERMAIN_H

#include <QWidget>
#include <QMessageBox>
#include <QCloseEvent>
#include "HChatServer.h"

class HChatRoomServerMain_;

class HChatMsgServer;
class HChatFileServer;

QT_BEGIN_NAMESPACE
namespace Ui { class HChatRoomServerMain; }
QT_END_NAMESPACE

class HChatRoomServerMain : public QWidget {
    Q_OBJECT

public:
    HChatRoomServerMain(QWidget *parent = nullptr);
    ~HChatRoomServerMain();

public:
    ///
    /// \brief initHChatRoomServerWindowStyle
    /// \brief 初始化Window样式
    ///
    void initHChatRoomServerWindowStyle();

protected:
    ///
    /// \brief eventFilter
    /// \brief 自定义部分Mouse事件
    /// \param watched
    /// \param evt
    /// \return
    ///
    bool eventFilter(QObject *watched, QEvent *evt) override;
    void closeEvent (QCloseEvent *e) override;
private:
    ///
    /// \brief scanAllAddressForDevice
    /// \brief 扫描本地所以ip
    ///
    void scanAllAddressForDevice();

    QString messageConcat(QString device = "Server", QString context = "") const;
    void    connectSignal();

private slots:
    ///
    /// \brief onMenuCloseButton
    /// \brief 开启服务槽函数
    ///
    void onStartServerButton(void);
    void onSendMessageToClient(void);

signals:
    void bordercastAllClient(const quint8& type, const QJsonValue& data);
private:
    Ui::HChatRoomServerMain *ui;
    friend class HChatRoomServerMain_;
    HChatRoomServerMain_ *s_;
};
#endif // HCHATROOMSERVERMAIN_H
