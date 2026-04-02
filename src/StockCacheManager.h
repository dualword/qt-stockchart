#pragma once
#include <QMap>
#include <QVector>
#include <QDate>
#include <QDateTime>
#include <QString>
#include "StockDataProvider.h"

// Manages the in-memory + persisted cache of historical price data and symbol types.
class StockCacheManager
{
public:
    void loadCache();
    void saveCache();

    void loadSymbolTypeCache();
    void saveSymbolType(const QString &symbol, SymbolType type);

    void loadLoadTimes();
    void saveLoadTimes();

    // Records the current datetime as the last load time for symbol.
    void setLoadTime(const QString &sym) { m_loadTimes[sym] = QDateTime::currentDateTime(); }

    // Returns age of last load in seconds, or -1 if no load time recorded.
    qint64 loadAgeSecs(const QString &sym) const {
        if (!m_loadTimes.contains(sym)) return -1;
        qint64 s = m_loadTimes[sym].secsTo(QDateTime::currentDateTime());
        return s >= 0 ? s : 0;
    }

    // Returns a human-readable age string ("12d", "3h", "10m") or empty if unknown.
    QString ageString(const QString &sym) const;

    // Returns the last closing price on or before `target` in ascending-sorted data.
    // Returns NaN if no data point exists at or before that date.
    static double priceAt(const QVector<StockDataPoint> &data, const QDate &target);

    QMap<QString, QVector<StockDataPoint>> &cache()             { return m_cache; }
    const QMap<QString, QVector<StockDataPoint>> &cache() const { return m_cache; }

    QMap<QString, SymbolType> &symbolTypes()             { return m_symbolTypes; }
    const QMap<QString, SymbolType> &symbolTypes() const { return m_symbolTypes; }

private:
    QMap<QString, QVector<StockDataPoint>> m_cache;
    QMap<QString, SymbolType>              m_symbolTypes;
    QMap<QString, QDateTime>               m_loadTimes;
};
