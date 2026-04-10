#include "RequestInterceptor.h"
#include <QWebEngineUrlRequestInfo>

RequestInterceptor::RequestInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{}

void RequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    const QString host = info.requestUrl().host();
    if (host.isEmpty()) return;

    QWriteLocker locker(&m_lock);

    const bool isNew = !m_accessed.contains(host);
    m_accessed.insert(host);

    if (m_blacklist.contains(host)) {
        m_hits[host]++;
        info.block(true);
    }

    locker.unlock();

    if (isNew)
        emit newDomainSeen(host); // auto-queued to main thread (IO → GUI)
}

void RequestInterceptor::setBlacklist(const QSet<QString> &domains)
{
    QWriteLocker locker(&m_lock);
    m_blacklist = domains;
}

QSet<QString> RequestInterceptor::blacklist() const
{
    QReadLocker locker(&m_lock);
    return m_blacklist;
}

void RequestInterceptor::addDomains(const QStringList &domains)
{
    QWriteLocker locker(&m_lock);
    for (const QString &d : domains)
        m_blacklist.insert(d);
}

void RequestInterceptor::removeDomains(const QStringList &domains)
{
    QWriteLocker locker(&m_lock);
    for (const QString &d : domains)
        m_blacklist.remove(d);
}

QStringList RequestInterceptor::accessedDomains() const
{
    QReadLocker locker(&m_lock);
    QStringList list(m_accessed.cbegin(), m_accessed.cend());
    list.sort(Qt::CaseInsensitive);
    return list;
}

QMap<QString, int> RequestInterceptor::blockedHits() const
{
    QReadLocker locker(&m_lock);
    return m_hits;
}

void RequestInterceptor::clearTracking()
{
    QWriteLocker locker(&m_lock);
    m_accessed.clear();
    m_hits.clear();
}
