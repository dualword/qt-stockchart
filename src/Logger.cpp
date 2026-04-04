#include "Logger.h"
#include <QTime>
#include <cstdio>

Logger &Logger::instance()
{
    static Logger s_instance;
    return s_instance;
}

Logger::Logger(QObject *parent) : QObject(parent) {}

static QString buildHtmlLine(const QString &ts, const QString &message)
{
    return QStringLiteral("<span style=\"color:#cc9900;\">[")
           + ts
           + QStringLiteral("]</span> ")
           + message.toHtmlEscaped();
}

void Logger::append(const QString &message)
{
    const QString ts = QTime::currentTime().toString("HH:mm:ss");
    fprintf(stdout, "[%s] %s\n", qPrintable(ts), qPrintable(message));
    fflush(stdout);
    emit messageLogged(buildHtmlLine(ts, message));
}

void Logger::appendSilent(const QString &message)
{
    const QString ts = QTime::currentTime().toString("HH:mm:ss");
    emit messageLogged(buildHtmlLine(ts, message));
}

void Logger::clear()
{
    emit cleared();
}
