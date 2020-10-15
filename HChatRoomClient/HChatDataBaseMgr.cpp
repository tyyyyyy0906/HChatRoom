#include "HChatDataBaseMgr.h"
#include "Global.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDateTime>

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
    p_->userInfo_ = QSqlDatabase::addDatabase("QSQLITE");
    p_->userInfo_.setUserName("admin");
    p_->userInfo_.setPassword("123456");
    p_->userInfo_.setDatabaseName(dataBaseName.isEmpty() ? QStringLiteral("user_info") : dataBaseName);
    if (!p_->userInfo_.open()) {
        return false;
    }
    createTable();
    initAllUser();
    return true;
}

void HChatDataBaseMgr::closeChatDataBase() {
    p_->userInfo_.close();
}

void HChatDataBaseMgr::createTable() {
    QSqlQuery query;
    /// userInfo table
    query.exec("CREATE TABLE USERINFO (id INT PRIMARY KEY, name varchar(20), "
               "passwd varchar(20), head varchar(20), status INT, groupId INT, lasttime DATETIME);");

    query.exec("CREATE TABLE GROUPINFO (id INT PRIMARY KEY, groupId INT, name varchar(20), head varchar(20), "
               "userId INT, identity INT)");

    query.exec("CREATE TABLE USERHEAD (id INT PRIMARY KEY, name varchar(20), data varchar)");

    /// 插入两个用户数据(admin)
    query.exec("INSERT INTO USERINFO VALUES(1, 'admin'   , '123456', '2.bmp', 0, 1, '');");
    query.exec("INSERT INTO USERINFO VALUES(2, 'zhangsan', '123456', '2.bmp', 0, 1, '');");
    query.exec("INSERT INTO USERINFO VALUES(3, 'lisi'    , '123456', '2.bmp', 0, 1, '');");
}

void HChatDataBaseMgr::updateUserStatus(const int &id, const quint8 &state) {
    QString sql = "UPDATE USERINFO SET status=";
    sql += QString::number(state);
    sql += QString(",lasttime='");
    sql += DataCurrTimer;
    sql += QString("' WHERE id=");
    sql += QString::number(id);
    QSqlQuery query(sql);
    query.exec();
}

QJsonArray HChatDataBaseMgr::getAllUserForTable() {
    QSqlQuery query("SELECT * FROM USERINFO ORDER BY id;");
    QJsonArray userArray_;

    while (query.next()) {
        QJsonObject info_;
        info_.insert("id"      , query.value("id"      ).toInt());
        info_.insert("name"    , query.value("name"    ).toString());
        info_.insert("passwd"  , query.value("passwd"  ).toString());
        info_.insert("head"    , query.value("head"    ).toString());
        info_.insert("status"  , query.value("status"  ).toInt());
        info_.insert("groupId" , query.value("groupId" ).toInt());
        info_.insert("lasttime", query.value("lasttime").toString());
        userArray_.append(info_);
    }

    return userArray_;
}

int HChatDataBaseMgr::getUserOnlineStatus(const int& id) const {
    QString sql = "SELECT [status] FROM USERINFO ";
    sql.append("WHERE id=");
    sql.append(QString::number(id));

    QSqlQuery query(sql);
    if (query.next()) return query.value(0).toInt();
    return -1;
}

QJsonObject HChatDataBaseMgr::loginCheck(const QString &name, const QString &passwd) {
    QString sql = "SELECT [id],[head],[status] FROM USERINFO ";
    sql.append("WHERE name='");
    sql.append(name);
    sql.append("' AND passwd='");
    sql.append(passwd);
    sql.append("';");

    QJsonObject info_;
    int id_ = -1, code_ = -1, status_;
    QString head_ = "0.bmp";

    QSqlQuery query(sql);
    if (query.next()) {
        id_     = query.value("id").toInt();
        status_ = query.value("status").toInt();
        if (GlobalMessage::LoginStatus::ClientOnline == status_) {
            id_ = code_ = -2;
        }
        else {
            updateUserStatus(id_, GlobalMessage::LoginStatus::ClientOnline);
            code_ = 0;
        }
        head_ = query.value("head").toString();
    }

    info_.insert("id"  , id_);
    info_.insert("msg" , id_ < 0 ? "error" : "ok");
    info_.insert("head", head_);
    info_.insert("code", code_);

    return info_;
}

void HChatDataBaseMgr::initAllUser() {
    QSqlQuery query("SELECT * FROM USERINFO ORDER BY id;");
    while (query.next()) {
        updateUserStatus(query.value(0).toInt(), GlobalMessage::LoginStatus::ClientOffline);
    }
}
