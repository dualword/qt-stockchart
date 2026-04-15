#pragma once
#include "StockDataProvider.h"
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QTimer>
#include <QMap>
#include <QStringList>
#include <QQueue>

// Loads the Morningstar quote page in a headless QWebEnginePage (Chromium).
// Unlike MorningstarProvider (plain QNetworkAccessManager), this class executes
// Cloudflare's JS challenge natively — the same way a real browser would —
// allowing it to obtain a valid cf_clearance cookie without being blocked.
//
// After loadFinished the full rendered HTML is extracted via
//   runJavaScript("document.documentElement.outerHTML")
// and the same regex-based parsePrice() logic locates the lastPrice field
// embedded in the page's JS object literals.
//
// A QWebEngineScript injected at DocumentCreation hides navigator.webdriver
// so the Chromium instance is not trivially identified as automation.
//
// A persistent QWebEngineProfile ("MorningstarWebProvider") stores cookies on
// disk; Cloudflare session state therefore accumulates across app restarts,
// further reducing bot-detection friction.
//
// Exchange auto-detection (xnys → xnas → xase → arcx) and per-symbol caching
// follow the same strategy as MorningstarProvider.
//
// Page loads are serialised (one at a time) via an internal queue.
class MorningstarWebProvider : public StockDataProvider
{
    Q_OBJECT
public:
    explicit MorningstarWebProvider(QObject *parent = nullptr);
    ~MorningstarWebProvider() override = default;

    QString id() const override { return "morningstarweb"; }
    QList<QPair<QString,QString>> credentialFields() const override { return {}; }

    void fetchData(const QString &symbol, const QString &range = "3mo") override;
    void fetchLatestQuote(const QString &symbol) override;

    // Provide a pointer to the symbol-type map so the provider can restrict URL
    // candidates to the asset class that matches each symbol (Stock/ETF/Fund).
    void setSymbolTypes(const QMap<QString, SymbolType> *types) { m_symbolTypes = types; }

private slots:
    void onLoadFinished(bool ok);
    void onLoadTimeout();

private:
    struct Request {
        QString symbol;
        QString exchange;
    };

    void enqueue(const QString &symbol);
    void loadNext();
    void loadPage(const QString &symbol, const QString &exchange);
    void tryNextExchange(const QString &symbol, const QString &triedExchange);
    void finishCurrent();
    bool parsePrice(const QString &html, double &price, QDateTime &dt) const;

    // Returns the exchange candidates appropriate for the symbol's known type.
    const QStringList &candidatesFor(const QString &symbol) const;

    QWebEngineProfile *m_profile  = nullptr;
    QWebEnginePage    *m_page     = nullptr;
    QTimer            *m_timer    = nullptr;   // guards against hung page loads

    bool            m_busy     = false;
    bool            m_timedOut = false;
    Request         m_current;
    QQueue<Request> m_queue;

    QMap<QString, QString>      m_symbolExchange;  // symbol -> known working exchange
    const QMap<QString, SymbolType> *m_symbolTypes = nullptr;

    static const QStringList kExchanges;     // all exchanges (fallback for Unknown type)
    static const QStringList kStockExchanges;
    static const QStringList kEtfExchanges;
    static const QStringList kFundExchanges;
};
