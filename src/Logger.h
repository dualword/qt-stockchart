#pragma once
#include <QObject>
#include <QString>

// Singleton logger: captures messages and emits them as formatted HTML lines.
// Consumers connect to messageLogged() and cleared() to update a display widget.
// Thread-safe via Qt::QueuedConnection in appendSilent() calls from the message handler.
class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger &instance();

    // Prepend [HH:MM:SS] timestamp, write to stdout, emit messageLogged().
    Q_INVOKABLE void append(const QString &message);

    // Same as append() but does NOT write to stdout.
    // Used by the Qt message handler which already writes to stdout itself.
    Q_INVOKABLE void appendSilent(const QString &message);

    void clear();

signals:
    void messageLogged(const QString &htmlLine);
    void cleared();

private:
    explicit Logger(QObject *parent = nullptr);
};
