#include "HChatDataBaseMgr.h"
#include "Global.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDateTime>
#include <QVariant>
#include <QDebug>
#include <QFile>

#define DataCurrTimer QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")

class HChatDataBaseMgr_ {
public:
    HChatDataBaseMgr *this_ = Q_NULLPTR;

    QSqlDatabase userInfo_;
};

HChatDataBaseMgr::HChatDataBaseMgr(QObject *parent)
    : QObject(parent)
    , p_(new HChatDataBaseMgr_) {

    p_->this_ = this;
}

HChatDataBaseMgr::~HChatDataBaseMgr() {
    delete p_;
}

HChatDataBaseMgr &HChatDataBaseMgr::instance(void) {
    static HChatDataBaseMgr dataBase_;
    return dataBase_;
}

bool HChatDataBaseMgr::openChatDataBase(const QString &dataBaseName) {
    qDebug() << "current database path = " << dataBaseName;
    p_->userInfo_ = QSqlDatabase::addDatabase("QSQLITE");
    p_->userInfo_.setDatabaseName(dataBaseName);
    p_->userInfo_.setUserName("root");
    p_->userInfo_.setPassword("123456");
    if (!p_->userInfo_.open()) {
        qDebug("数据库打开失败");
        return false;
    }
    qDebug("数据库打开成功");
    createTable();
    return true;
}

void HChatDataBaseMgr::closeChatDataBase() {
    p_->userInfo_.close();
}

void HChatDataBaseMgr::createTable() {
    QSqlQuery query;
    /// userInfo table
    query.exec("CREATE TABLE FRIEND (id INT, userId INT, name varchar(50))");

    // 用户数据保存
    query.exec("CREATE TABLE USERINFO (id INT, name varchar(50), passwd varchar(50))");
    /// 插入两个用户数据(admin)
    query.exec("INSERT INTO USERINFO VALUES(1, 'admin'   , '123456');");
    query.exec("INSERT INTO USERINFO VALUES(2, 'zhangsan', '123456');");
    query.exec("INSERT INTO USERINFO VALUES(3, 'lisi'    , '123456');");
}

void HChatDataBaseMgr::initAllUser() {
    QSqlQuery query("SELECT * FROM USERINFO ORDER BY id;");
    while (query.next()) {
        updateUserStatus(query.value(0).toInt(), GlobalMessage::LoginStatus::ClientOffline);
    }
}
