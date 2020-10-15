#include "AppConfig.h"

using namespace App;

QString AppConfig::conAppInstallPath  = "./";
QString AppConfig::conAppDatasPath    = "";
QString AppConfig::conAppDataBasePath = "";
QString AppConfig::conConfigFilePath  = "";
QString AppConfig::conRecordsPath     = "";
QString AppConfig::conIniFilePath     = "";
QString AppConfig::conServerAddress   = "127.0.0.1";
QString AppConfig::conUserName        = "zhangsan";
QString AppConfig::conPassWord        = "123456";

int     AppConfig::conServerMsgPort   = 66666;
int     AppConfig::conServerFilePort  = 66667;
int     AppConfig::conID_             = -1;

void AppConfig::initAppConfig(const QString &installPath) {
    conAppInstallPath = installPath + "/";
    conAppDatasPath   = conAppInstallPath + "AppData/";
    conAppDataBasePath= conAppInstallPath + "DataBase/";
    conConfigFilePath = conAppInstallPath + "Config/";
    conRecordsPath    = conAppInstallPath + "Records/";
    conIniFilePath    = conAppInstallPath + "Ini/";
}

void AppConfig::creatorConfig() {
    QSettings setting_(conIniFilePath, QSettings::IniFormat);
    QString strGroups = setting_.childGroups().join("");
    if (!QFile::exists(conIniFilePath) || (strGroups.isEmpty())) {
        setting_.beginGroup("userConfig");
        setting_.setValue("userName", conUserName);
        setting_.setValue("passWord", conPassWord);
        setting_.endGroup();

        setting_.beginGroup("Server");
        setting_.setValue("serverAddress" , conServerAddress);
        setting_.setValue("serverMsgPort" , conServerMsgPort);
        setting_.setValue("serverFilePort", conServerFilePort);
        setting_.endGroup();
        setting_.sync();
    }
}

void AppConfig::readConfig() {

}

void AppConfig::updateConfig(const QString& key, const QVariant& value) {
    Q_UNUSED(key)
    Q_UNUSED(value)
}
