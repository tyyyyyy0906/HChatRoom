/*****************************************************************************
MIT License

Copyright (c) 2020 tyyyyyy0906

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

//图形字体处理类
class IconHelper : public QObject
{
    Q_OBJECT

public:
    static IconHelper *Instance();
    explicit IconHelper(QObject *parent = 0);

    void setIcon(QLabel *lab, QChar c, quint32 size = 12);
    void setIcon(QAbstractButton *btn, QChar c, quint32 size = 12);
    QPixmap getPixmap(const QString &color, QChar c, quint32 size = 12,
                      quint32 pixWidth = 10, quint32 pixHeight = 10);

    //根据按钮获取该按钮对应的图标
    QPixmap getPixmap(QToolButton *btn, bool normal);

    //指定导航面板样式,不带图标
    static void setStyle(QWidget *widget, const QString &type = "left", int borderWidth = 3,
                         const QString &borderColor = "#029FEA",
                         const QString &normalBgColor = "#292F38",
                         const QString &darkBgColor = "#1D2025",
                         const QString &normalTextColor = "#54626F",
                         const QString &darkTextColor = "#FDFDFD");

    //指定导航面板样式,带图标和效果切换
    void setStyle(QWidget *widget, QList<QToolButton *> btns, QList<int> pixChar,
                  quint32 iconSize = 9, quint32 iconWidth = 10, quint32 iconHeight = 10,
                  const QString &type = "left", int borderWidth = 3,
                  const QString &borderColor = "#029FEA",
                  const QString &normalBgColor = "#292F38",
                  const QString &darkBgColor = "#1D2025",
                  const QString &normalTextColor = "#54626F",
                  const QString &darkTextColor = "#FDFDFD");

    //指定导航按钮样式,带图标和效果切换
    void setStyle(QFrame *frame, QList<QToolButton *> btns, QList<int> pixChar,
                  quint32 iconSize = 9, quint32 iconWidth = 10, quint32 iconHeight = 10,
                  const QString &normalBgColor = "#2FC5A2",
                  const QString &darkBgColor = "#3EA7E9",
                  const QString &normalTextColor = "#EEEEEE",
                  const QString &darkTextColor = "#FFFFFF");

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    static IconHelper *self;    //对象自身
    QFont iconFont;             //图形字体
    QList<QToolButton *> btns;  //按钮队列
    QList<QPixmap> pixNormal;   //正常图片队列
    QList<QPixmap> pixDark;     //加深图片队列
};
#endif // ICONHELPER_H
