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

#include "appinit.h"
#include "qmutex.h"
#include "qapplication.h"
#include "qevent.h"
#include "qwidget.h"

AppInit *AppInit::self = nullptr;
AppInit *AppInit::Instance()
{
    if (!self)
    {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self)
        {
            self = new AppInit;
        }
    }

    return self;
}

AppInit::AppInit(QObject *parent) : QObject(parent)
{
}

bool AppInit::eventFilter(QObject *obj, QEvent *evt)
{
    QWidget *w = (QWidget *)obj;
    if (!w->property("canMove").toBool())
    {
        return QObject::eventFilter(obj, evt);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;

    QMouseEvent *event = static_cast<QMouseEvent *>(evt);
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (event->button() == Qt::LeftButton)
        {
            mousePressed = true;
            mousePoint = event->globalPos() - w->pos();
            return true;
        }
    }

    else if (event->type() == QEvent::MouseButtonRelease)
    {
        mousePressed = false;
        return true;
    }

    else if (event->type() == QEvent::MouseMove)
    {
        if (mousePressed && (event->buttons() && Qt::LeftButton))
        {
            w->move(event->globalPos() - mousePoint);
            return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

void AppInit::start()
{
    qApp->installEventFilter(this);
}
