#pragma once
#include "StockDataProvider.h"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QMap>
#include <QStringList>

// Scrapes the Morningstar quote page for the current price and timestamp.
// No API key required. Returns one data point (current price).
//
// URL format: https://www.morningstar.com/stocks/{exchange}/{symbol}/quote
//
// The page embeds intraday bars as unquoted JS object literals, e.g.:
//   {datetime:"2026-04-13T14:45:00-04:00",volume:45580,lastPrice:237.8201,...}
// We extract the last (most recent) bar's lastPrice and datetime.
//
// Exchange auto-detection: tries xnys → xnas → xase → arcx in order until a
// price is found. Successful exchange is cached per symbol for subsequent calls.
//
// Session warm-up: on first use the homepage is fetched so that Cloudflare
// session cookies (__cf_bm, etc.) are established before any quote requests.
// Those cookies are then re-sent automatically by the QNetworkCookieJar.
class MorningstarProvider : public StockDataProvider
{
    Q_OBJECT
public:
    explicit MorningstarProvider(QObject *parent = nullptr);

    QString id() const override { return "morningstar"; }
    QList<QPair<QString,QString>> credentialFields() const override { return {}; }

    void fetchData(const QString &symbol, const QString &range = "3mo") override;
    void fetchLatestQuote(const QString &symbol) override;

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    // Special exchange tag stored in m_pending to mark a warm-up reply
    static constexpr const char *kWarmupTag = "__warmup__";

    void doFetch(const QString &symbol);
    void startWarmup(const QString &pendingSymbol);
    void fetchQuotePage(const QString &symbol, const QString &exchange);
    void tryNextExchange(const QString &symbol, const QString &triedExchange);
    bool parsePrice(const QString &html, double &price, QDateTime &dt) const;
    QNetworkRequest buildRequest(const QUrl &url) const;

    QNetworkAccessManager  *m_manager;
    QMap<QString, QString>  m_symbolExchange; // cache: symbol -> known working exchange
    bool                    m_warmedUp = false;
    QStringList             m_warmupQueue;    // symbols waiting for warm-up to finish

    static const QStringList kExchanges;
};
