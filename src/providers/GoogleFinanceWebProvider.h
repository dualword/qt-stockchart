#pragma once
#include "StockDataProvider.h"
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QTimer>
#include <QMap>
#include <QStringList>
#include <QQueue>

// Loads the Google Finance quote page in a headless QWebEnginePage (Chromium).
// URL pattern: https://www.google.com/finance/quote/{SYMBOL}:{EXCHANGE}
//
// The current price is extracted from the data-last-price attribute that Google
// renders into the DOM, with a regex fallback on the full page HTML.
//
// Exchange auto-detection is restricted to the asset class known for the symbol
// (Stock / ETF / MutualFund) when setSymbolTypes() has been called.  The working
// exchange is cached per symbol so subsequent requests use a single URL directly.
//
// A persistent QWebEngineProfile stores cookies across app restarts.
// navigator.webdriver is hidden at DocumentCreation to reduce bot-score friction.
// Page loads are serialised one at a time via an internal queue.
class GoogleFinanceWebProvider : public StockDataProvider
{
    Q_OBJECT
public:
    explicit GoogleFinanceWebProvider(QObject *parent = nullptr);
    ~GoogleFinanceWebProvider() override = default;

    QString id() const override { return "googlefinanceweb"; }
    QList<QPair<QString,QString>> credentialFields() const override { return {}; }

    void fetchData(const QString &symbol, const QString &range = "3mo") override;
    void fetchLatestQuote(const QString &symbol) override;

    // Provide a pointer to the symbol-type map so URL candidates can be narrowed
    // to the correct asset class (Stock / ETF / Fund) for each symbol.
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
    void runExtraction();   // called ~2 s after loadFinished to allow async rendering
    const QStringList &candidatesFor(const QString &symbol) const;

    QWebEngineProfile *m_profile  = nullptr;
    QWebEnginePage    *m_page     = nullptr;
    QTimer            *m_timer    = nullptr;

    bool            m_busy     = false;
    bool            m_timedOut = false;
    Request         m_current;
    QQueue<Request> m_queue;

    QMap<QString, QString>           m_symbolExchange;
    const QMap<QString, SymbolType> *m_symbolTypes = nullptr;

    static const QStringList kExchanges;       // all (fallback for Unknown type)
    static const QStringList kStockExchanges;
    static const QStringList kEtfExchanges;
    static const QStringList kFundExchanges;
};
