#include "HChatRoomLogin.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HChatRoomLogin w;
    w.show();
    return a.exec();
}
