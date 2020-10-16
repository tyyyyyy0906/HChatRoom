#ifndef HCHATDATABASEMGR_H
#define HCHATDATABASEMGR_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QJsonObject>

class HChatDataBaseMgr_;
class HChatDataBaseMgr : public QObject {
    Q_OBJECT
public:
    static HChatDataBaseMgr &instance(void);

public:
    bool openChatDataBase(const QString& dataBaseName);
    void updateClientStatus(const int& clientID, const quint8& status);

    QJsonObject checkUserInfoLogin(const QString& userName, const QString& passwd);
    QString     getUserName(const int &id) const;

private:
    explicit HChatDataBaseMgr(QObject *parent = 0);
    ~HChatDataBaseMgr();
    HChatDataBaseMgr(const HChatDataBaseMgr&) = delete;
    HChatDataBaseMgr& operator=(const HChatDataBaseMgr&) = delete;

    void initAllUserStatus();

signals:

private:
    friend class HChatDataBaseMgr_;
    HChatDataBaseMgr_ *p_;
};

#endif // HCHATDATABASEMGR_H
