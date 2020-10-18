#include "HChatRoomLogin.h"
#include "HChatDataBaseMgr.h"
#include "AppConfig.h"
#include "appinit.h"
#include "iconhelper.h"

#include <QTextCodec>
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qInstallMessageHandler(outputMessage);
    a.setQuitOnLastWindowClosed(false);

//// 设置Qt4.x 和5.x platform编码格式，避免Qt4.x和Qt5.x不兼容导致中文下出现文字乱码
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("GTK");
#else
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCString(codec);
    QTextCodec::setCodecForTr(codec);
#else
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    App::AppConfig::initAppConfig(a.applicationDirPath());
    HChatDataBaseMgr::instance().openChatDataBase(App::AppConfig::conAppDataBasePath + "user.db");
    HChatRoomLogin w;
    App::AppConfig::installStyle(&w);

    AppInit::Instance()->start();

    w.show();
    return a.exec();
}

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type) {
    case QtDebugMsg   : text = QString("[Debug]:"   ); break;
    case QtWarningMsg : text = QString("[Warning]:" ); break;
    case QtCriticalMsg: text = QString("[Critical]:"); break;
    case QtFatalMsg   : text = QString("[Fatal]:"   ); abort();
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QFile file("./Log/ClientLog.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    mutex.unlock();
}
