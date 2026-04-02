#include "StockCacheManager.h"
#include <QSettings>
#include <QDataStream>
#include <limits>
#include <cmath>

double StockCacheManager::priceAt(const QVector<StockDataPoint> &data, const QDate &target)
{
    double result = std::numeric_limits<double>::quiet_NaN();
    for (const StockDataPoint &pt : data) {
        if (pt.timestamp.date() <= target)
            result = pt.price; // ascending order: keep updating
        else
            break;
    }
    return result;
}

void StockCacheManager::saveCache()
{
    QSettings s("StockChart", "StockChart");
    s.beginGroup("historyCache");
    for (auto it = m_cache.cbegin(); it != m_cache.cend(); ++it) {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << (qint32)it.value().size();
        for (const auto &pt : it.value())
            out << pt.timestamp << pt.price;
        s.setValue(it.key(), data);
    }
    s.endGroup();
}

void StockCacheManager::loadCache()
{
    QSettings s("StockChart", "StockChart");
    s.beginGroup("historyCache");
    for (const QString &sym : s.childKeys()) {
        QByteArray data = s.value(sym).toByteArray();
        QDataStream in(&data, QIODevice::ReadOnly);
        qint32 size;
        in >> size;
        QVector<StockDataPoint> points;
        points.reserve(size);
        for (int i = 0; i < size; ++i) {
            QDateTime dt;
            double p;
            in >> dt >> p;
            if (!dt.isNull())
                points.append({dt, p});
        }
        if (!points.isEmpty()) m_cache[sym] = points;
    }
    s.endGroup();
}

void StockCacheManager::loadSymbolTypeCache()
{
    QSettings s("StockChart", "StockChart");
    s.beginGroup("symbolTypes");
    for (const QString &sym : s.childKeys())
        m_symbolTypes[sym] = static_cast<SymbolType>(s.value(sym).toInt());
    s.endGroup();
}

void StockCacheManager::saveSymbolType(const QString &symbol, SymbolType type)
{
    QSettings s("StockChart", "StockChart");
    s.beginGroup("symbolTypes");
    s.setValue(symbol, static_cast<int>(type));
    s.endGroup();
}

void StockCacheManager::saveLoadTimes()
{
    QSettings s("StockChart", "StockChart");
    s.beginGroup("loadTimes");
    s.remove("");
    for (auto it = m_loadTimes.cbegin(); it != m_loadTimes.cend(); ++it)
        s.setValue(it.key(), it.value().toString(Qt::ISODate));
    s.endGroup();
}

void StockCacheManager::loadLoadTimes()
{
    QSettings s("StockChart", "StockChart");
    s.beginGroup("loadTimes");
    for (const QString &sym : s.childKeys()) {
        QDateTime dt = QDateTime::fromString(s.value(sym).toString(), Qt::ISODate);
        if (dt.isValid()) m_loadTimes[sym] = dt;
    }
    s.endGroup();
}

QString StockCacheManager::ageString(const QString &sym) const
{
    if (!m_loadTimes.contains(sym)) return {};
    qint64 secs = m_loadTimes[sym].secsTo(QDateTime::currentDateTime());
    if (secs < 0) secs = 0;
    const qint64 days = secs / 86400;
    const qint64 hrs  = secs / 3600;
    const qint64 mins = secs / 60;
    if (days >= 1) return QString("%1d").arg(days);
    if (hrs  >= 1) return QString("%1h").arg(hrs);
    return QString("%1m").arg(qMax(qint64(1), mins));
}
