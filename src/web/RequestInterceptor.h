#pragma once
#include <QWebEngineUrlRequestInterceptor>
#include <QReadWriteLock>
#include <QSet>
#include <QMap>
#include <QStringList>

// Runs on the browser network IO thread — all shared state is protected by m_lock.
// clearTracking() resets per-page-load data (accessed domains + hit counts) without
// touching the blacklist.
class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    explicit RequestInterceptor(QObject *parent = nullptr);

    // Called on IO thread by Qt WebEngine — do not call directly.
    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

    // Blacklist management — call from main thread only.
    void setBlacklist(const QSet<QString> &domains);
    QSet<QString> blacklist() const;
    void addDomains(const QStringList &domains);
    void removeDomains(const QStringList &domains);

    // Snapshots safe to read from the main thread.
    QStringList accessedDomains() const;   // all domains seen this page load, sorted
    QMap<QString, int> blockedHits() const; // blocked domain -> hit count this page load

    // Reset per-page tracking; call before each new page navigation.
    void clearTracking();

signals:
    // Emitted once per newly-seen domain (queued across thread boundary automatically).
    void newDomainSeen(const QString &domain);

private:
    mutable QReadWriteLock m_lock;
    QSet<QString>      m_blacklist;
    QSet<QString>      m_accessed;
    QMap<QString, int> m_hits;
};
