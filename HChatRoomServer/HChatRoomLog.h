#pragma once

#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#define SYSTEMTIME QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"))

void customMessageHandler(const QtMsgType& type, const char* msg) {
    QString txtMessage;
    switch (type) {
        case QtDebugMsg:
            txtMessage = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txtMessage = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txtMessage = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txtMessage = QString("Fatal: %1").arg(msg);
            abort();
    }

    QString file_ = path + QString("%1log.txt").arg(SYSTEMTIME);
    QFile outputFile("log.text");
    outputFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&outputFile);
    textStream << txtMessage << endl;
}
