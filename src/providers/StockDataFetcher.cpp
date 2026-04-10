#include "StockDataFetcher.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

StockDataFetcher::StockDataFetcher(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &StockDataFetcher::onReplyFinished);
}

void StockDataFetcher::fetchData(const QString &symbol, const QString &range)
{
    QString urlStr = QString("https://query1.finance.yahoo.com/v8/finance/chart/%1"
                             "?interval=1d&range=%2")
                         .arg(symbol, range);

    QNetworkRequest request{QUrl(urlStr)};
    // Yahoo Finance requires a User-Agent to avoid 429 responses
    request.setRawHeader("User-Agent",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "application/json");

    m_manager->get(request);
}

void StockDataFetcher::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }

    // Extract symbol from the request URL path: /v8/finance/chart/AAPL
    QString symbol = reply->url().path().section('/', -1);

    QByteArray raw = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (doc.isNull()) {
        emit errorOccurred("Failed to parse JSON for " + symbol);
        return;
    }

    QJsonObject chart = doc.object()["chart"].toObject();
    QJsonArray result = chart["result"].toArray();
    if (result.isEmpty()) {
        QString errMsg = chart["error"].toObject()["description"].toString();
        emit errorOccurred(errMsg.isEmpty() ? "No data returned for " + symbol : errMsg);
        return;
    }

    QJsonObject first = result[0].toObject();
    QJsonArray timestamps = first["timestamp"].toArray();
    QJsonArray closes = first["indicators"].toObject()
                            ["quote"].toArray()[0].toObject()
                            ["close"].toArray();

    QVector<StockDataPoint> points;
    points.reserve(timestamps.size());

    for (int i = 0; i < timestamps.size() && i < closes.size(); ++i) {
        if (closes[i].isNull()) continue;
        StockDataPoint pt;
        pt.timestamp = QDateTime::fromSecsSinceEpoch(timestamps[i].toVariant().toLongLong());
        pt.price     = closes[i].toDouble();
        points.append(pt);
    }

    emit dataReady(symbol, points);
}
