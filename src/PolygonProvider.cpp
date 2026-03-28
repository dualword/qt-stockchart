#include "PolygonProvider.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QDate>

PolygonProvider::PolygonProvider(QObject *parent)
    : StockDataProvider(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &PolygonProvider::onReplyFinished);
}

QList<QPair<QString,QString>> PolygonProvider::credentialFields() const
{
    return {{"apiKey", "API Key"}};
}

void PolygonProvider::fetchData(const QString &symbol, const QString &range)
{
    QDate toDate   = QDate::currentDate();
    QDate fromDate = toDate.addDays(-rangeToDays(range));

    QString path = QString("/v2/aggs/ticker/%1/range/1/day/%2/%3")
                       .arg(symbol,
                            fromDate.toString("yyyy-MM-dd"),
                            toDate.toString("yyyy-MM-dd"));

    QUrl url("https://api.polygon.io" + path);
    QUrlQuery query;
    query.addQueryItem("adjusted", "true");
    query.addQueryItem("sort",     "asc");
    query.addQueryItem("limit",    "365");
    query.addQueryItem("apiKey",   m_credentials.value("apiKey").trimmed());
    url.setQuery(query);

    QNetworkRequest request{url};
    request.setRawHeader("User-Agent", "StockChart/1.0");

    QNetworkReply *reply = m_manager->get(request);
    m_pending[reply] = {symbol, range};
}

void PolygonProvider::onReplyFinished(QNetworkReply *reply)
{
    if (!m_pending.contains(reply)) return;
    reply->deleteLater();
    auto [symbol, range] = m_pending.take(reply);

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(symbol, reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred(symbol, "Polygon.io: failed to parse JSON");
        return;
    }

    QJsonObject root   = doc.object();
    QString     status = root["status"].toString();

    // Free tier returns "DELAYED"; both are valid
    if (status != "OK" && status != "DELAYED") {
        emit errorOccurred(symbol, "Polygon.io: " + root["error"].toString());
        return;
    }

    QJsonArray results = root["results"].toArray();
    if (results.isEmpty()) {
        emit errorOccurred(symbol, "Polygon.io: no data returned for " + symbol);
        return;
    }

    QVector<StockDataPoint> points;
    points.reserve(results.size());
    for (const QJsonValue &val : results) {
        QJsonObject obj = val.toObject();
        // Polygon timestamps are in milliseconds
        points.append({
            QDateTime::fromMSecsSinceEpoch(obj["t"].toVariant().toLongLong()),
            obj["c"].toDouble()
        });
    }

    emit dataReady(symbol, points);
}

void PolygonProvider::fetchSymbolType(const QString &symbol)
{
    QUrl url("https://api.polygon.io/v3/reference/tickers/" + symbol);
    QUrlQuery query;
    query.addQueryItem("apiKey", m_credentials.value("apiKey").trimmed());
    url.setQuery(query);

    QNetworkRequest request{url};
    request.setRawHeader("User-Agent", "StockChart/1.0");

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, symbol]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QJsonObject results = QJsonDocument::fromJson(reply->readAll()).object()["results"].toObject();
        const QString typeCode = results["type"].toString();
        SymbolType type = SymbolType::Unknown;
        if      (typeCode == "CS")                              type = SymbolType::Stock;
        else if (typeCode == "ETF" || typeCode == "ETV")        type = SymbolType::ETF;
        else if (typeCode == "INDEX")                           type = SymbolType::Index;
        else if (typeCode == "MF"  || typeCode == "FUND")       type = SymbolType::MutualFund;
        else if (typeCode == "CRYPTO")                          type = SymbolType::Crypto;
        if (type != SymbolType::Unknown) emit symbolTypeReady(symbol, type);
    });
}
