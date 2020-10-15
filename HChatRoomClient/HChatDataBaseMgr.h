#ifndef HCHATDATABASEMGR_H
#define HCHATDATABASEMGR_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

class HChatDataBaseMgr_;
class HChatDataBaseMgr : public QObject
{
    Q_OBJECT
public:
    static HChatDataBaseMgr &instance(void);
public:
    ///
    /// \brief openChatDataBase
    /// \param dataBaseName
    /// \return
    ///
    bool openChatDataBase(const QString& dataBaseName);
    ///
    /// \brief closeChatDataBase
    ///
    void closeChatDataBase();
    ///
    /// \brief createTable
    ///
    void createTable();
    ///
    /// \brief initAllUser
    ///
    void initAllUser();

private:
    explicit HChatDataBaseMgr(QObject *parent = 0);
    ~HChatDataBaseMgr();
    HChatDataBaseMgr(const HChatDataBaseMgr&) = delete;
    HChatDataBaseMgr& operator=(const HChatDataBaseMgr&) = delete;

private:
    friend class HChatDataBaseMgr_;
    HChatDataBaseMgr_ *p_;
};

#endif // HCHATDATABASEMGR_H
