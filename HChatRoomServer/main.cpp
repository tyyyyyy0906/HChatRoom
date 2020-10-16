#include "HChatRoomServerMain.h"
#include "HChatDataBaseMgr.h"
#include "AppConfig.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
    HChatDataBaseMgr::instance().openChatDataBase(App::AppConfig::conAppDataBasePath + "info.db");

    HChatRoomServerMain w;
    w.show();
    return a.exec();
}
