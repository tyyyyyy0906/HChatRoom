#include "HChatDataBaseMgr.h"
#include "Global.h"

#include <QDebug>
#include <QDateTime>

using namespace GlobalMessage;

class HChatDataBaseMgr_ {
public:
    QSqlDatabase serverDataBase;
};

HChatDataBaseMgr::HChatDataBaseMgr(QObject *parent)
    : QObject(parent)
    , p_(new HChatDataBaseMgr_) {

}

HChatDataBaseMgr::~HChatDataBaseMgr() {
    delete p_;
}

HChatDataBaseMgr &HChatDataBaseMgr::instance(void) {
    static HChatDataBaseMgr dataBase_;
    return dataBase_;
}

bool HChatDataBaseMgr::openChatDataBase(const QString &dataBaseName) {
    p_->serverDataBase = QSqlDatabase::addDatabase("QSQLITE");
    p_->serverDataBase.setDatabaseName(dataBaseName);
    p_->serverDataBase.setUserName("root");
    p_->serverDataBase.setPassword("123456");
    if (!p_->serverDataBase.open()) {
        qDebug() << "数据库打开失败";
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE USERINFO (id INT PRIMARY KEY, name varchar(20), "
               "passwd varchar(20), head varchar(20), status INT, groupId INT, lasttime DATETIME);");

    query.exec("CREATE TABLE GROUPINFO (id INT PRIMARY KEY, groupId INT, name varchar(20), head varchar(20), "
               "userId INT, identity INT)");

    query.exec("CREATE TABLE USERHEAD (id INT PRIMARY KEY, name varchar(20), data varchar)");

    query.exec("INSERT INTO USERINFO VALUES(1, 'admin', '123456', '2.bmp', 0, 1, '');");
    query.exec("INSERT INTO USERINFO VALUES(2, 'userA', '123456', '1.bmp', 0, 1, '');");
    query.exec("INSERT INTO USERINFO VALUES(3, 'userB', '123456', '1.bmp', 0, 1, '');");

    initAllUserStatus();

    return true;
}

void HChatDataBaseMgr::updateClientStatus(const int &clientID, const quint8 &status) {
    QString sql_ = "UPDATE USERINFO SET status=";
    sql_ += QString::number(status);
    sql_ += QString(",lasttime='");
    sql_ += QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    sql_ += QString("' WHERE id=");
    sql_ += QString::number(clientID);

    QSqlQuery query(sql_);
    query.exec();
}

QJsonObject HChatDataBaseMgr::checkUserInfoLogin(const QString &userName, const QString &passwd) {
    QString sql_ = "SELECT [id],[head],[status] FROM USERINFO ";
    sql_.append("WHERE name='");
    sql_.append(userName);
    sql_.append("' AND passwd='");
    sql_.append(passwd);
    sql_.append("';");

    qDebug() << "查询用户是否存在sql语句 = " << sql_;

    QJsonObject info_;
    int id_ = -1, code_ = -1;

    QSqlQuery query(sql_);
    if (query.next()) {
        id_         = query.value("id"    ).toInt();
        int nStatus = query.value("status").toInt();

        if (MessageGroup::ClientUserOnLine == nStatus) id_ = code_ = -2;
        else {
            updateClientStatus(id_, MessageGroup::ClientUserOnLine);
            code_ = 0;
        }
    }

    info_.insert("id"  , id_);
    info_.insert("msg" , id_ < 0 ? "error" : "ok");
    info_.insert("head", "null");
    info_.insert("code", code_);

    return info_;
}

QString HChatDataBaseMgr::getUserName(const int &id) const {
    QString sql_ = "SELECT [name] FROM USERINFO ";
    sql_.append("WHERE id=");
    sql_.append(QString::number(id));
    QSqlQuery query(sql_);
    if (query.next()) {
        return query.value(0).toString();
    }
    return QString("");
}

void HChatDataBaseMgr::initAllUserStatus() {
    QSqlQuery query("SELECT * FROM USERINFO ORDER BY id;");
    while (query.next()) {
        updateClientStatus(query.value(0).toInt(), MessageGroup::ClientUserOffLine);
    }
}
