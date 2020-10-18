#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QFile>
#include <QWidget>

namespace App {
class AppConfig {
public:
    static QString conAppInstallPath;   // 应用程序路径
    static QString conAppDatasPath;     // 数据保存路径
    static QString conAppDataBasePath;  // 数据库目录
    static QString conConfigFilePath;   // 配置目录
    static QString conRecordsPath;      // 记录配置
    static QString conIniFilePath;      // 配置ini
    static QString conRecvFilePath;     // 文件路径
    static QString conRecvHeadPath;     // 头像路径
    static QString conServerAddress;    // 服务器地址
    static int     conServerMsgPort;    // 消息端口
    static int     conServerFilePort;   // 文件端口

    static int     conID_;

    static QString conUserName;         // 用户名
    static QString conPassWord;         // 密码

    static int     m_nWinX;
    static int     m_nWinY;

    static void initAppConfig(const QString& installPath);
    static void creatorConfig();
    static void readConfig();
    static void updateConfig(const QString& key, const QVariant& value);
    static void installStyle(QWidget* w);
};
} // namespace App

#endif
