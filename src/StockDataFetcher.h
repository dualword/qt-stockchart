#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVector>
#include <QDateTime>

struct StockDataPoint {
    QDateTime timestamp;
    double price;
};

class StockDataFetcher : public QObject
{
    Q_OBJECT
public:
    explicit StockDataFetcher(QObject *parent = nullptr);
    void fetchData(const QString &symbol, const QString &range = "3mo");

signals:
    void dataReady(const QString &symbol, const QVector<StockDataPoint> &data);
    void errorOccurred(const QString &message);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
};
