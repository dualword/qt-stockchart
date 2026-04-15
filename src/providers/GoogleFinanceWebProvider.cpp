#include "GoogleFinanceWebProvider.h"
#include "Logger.h"
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineProfile>
#include <QUrl>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

// Maximum time to wait for a single page load before declaring a timeout.
static constexpr int kLoadTimeoutMs  = 20'000;
// Delay after loadFinished before extracting: Google Finance fetches price data
// asynchronously via its own JavaScript, so we must wait for that to settle.
static constexpr int kExtractDelayMs = 2'500;

// Per-asset-type Google Finance exchange codes, tried in order until a price
// is found.  The working code is cached in m_symbolExchange so subsequent
// requests use a single URL directly.
const QStringList GoogleFinanceWebProvider::kStockExchanges = {
    "NYSE",           // NYSE-listed stocks (JPM, GS, …)
    "NASDAQ",         // NASDAQ-listed stocks (AAPL, MSFT, …)
    "NYSEAMERICAN",   // NYSE American (formerly AMEX) — smaller caps
};
const QStringList GoogleFinanceWebProvider::kEtfExchanges = {
    "NYSEARCA",       // NYSE Arca — most ETFs (SPY, GLD, …)
    "NASDAQ",         // NASDAQ-listed ETFs (QQQ, …)
    "NYSE",           // NYSE-listed ETFs
};
const QStringList GoogleFinanceWebProvider::kFundExchanges = {
    "MUTF",           // Mutual funds
};
// Fallback used when the symbol type is unknown — tries every exchange code.
const QStringList GoogleFinanceWebProvider::kExchanges =
    kStockExchanges + kEtfExchanges + kFundExchanges;

GoogleFinanceWebProvider::GoogleFinanceWebProvider(QObject *parent)
    : StockDataProvider(parent)
{
    // Persistent profile: Google session cookies accumulate across launches,
    // making the Chromium instance look like a returning user.
    m_profile = new QWebEngineProfile("GoogleFinanceWebProvider", this);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    // Hide navigator.webdriver so Google's bot-detection sees a normal browser.
    QWebEngineScript hideDriver;
    hideDriver.setName("HideWebDriver");
    hideDriver.setSourceCode(
        "Object.defineProperty(navigator, 'webdriver', { get: () => undefined });"
    );
    hideDriver.setInjectionPoint(QWebEngineScript::DocumentCreation);
    hideDriver.setWorldId(QWebEngineScript::MainWorld);
    m_profile->scripts()->insert(hideDriver);

    m_page  = new QWebEnginePage(m_profile, this);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(kLoadTimeoutMs);

    connect(m_page,  &QWebEnginePage::loadFinished,
            this,    &GoogleFinanceWebProvider::onLoadFinished);
    connect(m_timer, &QTimer::timeout,
            this,    &GoogleFinanceWebProvider::onLoadTimeout);
}

// ── Public API ────────────────────────────────────────────────────────────────

void GoogleFinanceWebProvider::fetchData(const QString &symbol, const QString &)
{
    // Page scraping returns only the current price bar — no historical range.
    enqueue(symbol);
}

void GoogleFinanceWebProvider::fetchLatestQuote(const QString &symbol)
{
    enqueue(symbol);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

const QStringList &GoogleFinanceWebProvider::candidatesFor(const QString &symbol) const
{
    if (m_symbolTypes) {
        switch (m_symbolTypes->value(symbol, SymbolType::Unknown)) {
        case SymbolType::Stock:      return kStockExchanges;
        case SymbolType::ETF:        return kEtfExchanges;
        case SymbolType::MutualFund: return kFundExchanges;
        default: break;
        }
    }
    return kExchanges;
}

// ── Queue management ──────────────────────────────────────────────────────────

void GoogleFinanceWebProvider::enqueue(const QString &symbol)
{
    // Deduplicate: don't queue the same symbol twice.
    if (m_busy && m_current.symbol == symbol) return;
    for (const Request &r : m_queue)
        if (r.symbol == symbol) return;

    const QStringList &candidates = candidatesFor(symbol);
    // Use the cached exchange only if it still belongs to the type's candidate
    // list; a stale entry from the wrong asset class would waste a page load.
    QString exchange = candidates.first();
    if (m_symbolExchange.contains(symbol) && candidates.contains(m_symbolExchange[symbol]))
        exchange = m_symbolExchange[symbol];

    m_queue.enqueue({ symbol, exchange });
    Logger::instance().append(
        QString("GoogleFinanceWeb [%1] queued exchange=%2 (queue size=%3)")
        .arg(symbol, exchange).arg(m_queue.size()));
    loadNext();
}

void GoogleFinanceWebProvider::loadNext()
{
    if (m_busy || m_queue.isEmpty()) return;
    m_busy    = true;
    m_current = m_queue.dequeue();
    loadPage(m_current.symbol, m_current.exchange);
}

void GoogleFinanceWebProvider::finishCurrent()
{
    m_busy = false;
    loadNext();
}

// ── Page loading ──────────────────────────────────────────────────────────────

void GoogleFinanceWebProvider::loadPage(const QString &symbol, const QString &exchange)
{
    m_current = { symbol, exchange };
    // Google Finance URL: /finance/quote/AAPL:NASDAQ
    const QUrl url(QString("https://www.google.com/finance/quote/%1:%2")
                   .arg(symbol.toUpper(), exchange));
    Logger::instance().append(
        QString("GoogleFinanceWeb [%1/%2] loading %3")
        .arg(symbol, exchange, url.toString()));
    m_timedOut = false;
    m_timer->start();
    m_page->load(url);
}

void GoogleFinanceWebProvider::tryNextExchange(const QString &symbol,
                                                const QString &triedExchange)
{
    const QStringList &candidates = candidatesFor(symbol);
    const int idx = candidates.indexOf(triedExchange);
    if (idx < 0 || idx + 1 >= candidates.size()) {
        emit errorOccurred(symbol,
            QString("GoogleFinanceWeb: price not found for %1 on any exchange").arg(symbol));
        finishCurrent();
        return;
    }
    loadPage(symbol, candidates[idx + 1]);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void GoogleFinanceWebProvider::onLoadTimeout()
{
    m_timedOut = true;
    m_page->triggerAction(QWebEnginePage::Stop);
    Logger::instance().append(
        QString("GoogleFinanceWeb [%1/%2] load timed out (%3 s) — stopping")
        .arg(m_current.symbol, m_current.exchange)
        .arg(kLoadTimeoutMs / 1000));
}

void GoogleFinanceWebProvider::onLoadFinished(bool ok)
{
    m_timer->stop();

    if (m_timedOut) {
        m_timedOut = false;
        emit errorOccurred(m_current.symbol,
            QString("GoogleFinanceWeb: page load timed out for %1/%2")
            .arg(m_current.symbol, m_current.exchange));
        finishCurrent();
        return;
    }

    if (!ok) {
        Logger::instance().append(
            QString("GoogleFinanceWeb [%1/%2] loadFinished(false) — trying next exchange")
            .arg(m_current.symbol, m_current.exchange));
        tryNextExchange(m_current.symbol, m_current.exchange);
        return;
    }

    // Google Finance loads price data asynchronously via its own JavaScript
    // after the initial DOM is ready.  Wait before extracting so that XHR
    // responses have time to populate the price element.
    QTimer::singleShot(kExtractDelayMs, this, &GoogleFinanceWebProvider::runExtraction);
}

// ── Extraction (runs after the async delay) ───────────────────────────────────

void GoogleFinanceWebProvider::runExtraction()
{
    const QString symbol   = m_current.symbol;
    const QString exchange = m_current.exchange;

    // Capture the full rendered HTML for the API call tracker first.
    m_page->runJavaScript("document.documentElement.outerHTML",
        [this, symbol, exchange](const QVariant &htmlResult) {

            const QString    html      = htmlResult.toString();
            const QByteArray htmlBytes = html.toUtf8();
            m_lastHistoryJson = htmlBytes;
            m_lastQuoteJson   = htmlBytes;
            emit historyResponseStored();
            emit quoteResponseStored();

            // ── Targeted JS extraction ────────────────────────────────────────
            // Run a multi-strategy snippet directly in the page context.
            // Returns "PRICE|UNIXTIMESTAMP" on success, or an empty string.
            //
            // Strategy 1 — data-last-price attribute (present on some page versions).
            // Strategy 2 — JSON-LD <script type="application/ld+json"> block.
            // Strategy 3 — inline <script> tags: patterns used by Google Finance's
            //              internal data serialisation (obfuscated but consistent).
            //
            // The price and timestamp are separated by "|" to keep the return
            // value a plain string (QWebEngine delivers Promises as QVariant::Invalid).
            static const QString kJs =
                "(function() {"
                "  var el = document.querySelector('[data-last-price]');"
                "  if (el) {"
                "    var p = parseFloat(el.getAttribute('data-last-price') || '0');"
                "    if (p > 0) return p + '|' + (el.getAttribute('data-last-price-time') || '');"
                "  }"
                "  var ld = document.querySelector('script[type=\"application/ld+json\"]');"
                "  if (ld) {"
                "    try {"
                "      var d = JSON.parse(ld.textContent);"
                "      var p2 = parseFloat(d.price || d.currentPrice || '0');"
                "      if (p2 > 0) return p2 + '|';"
                "    } catch(e) {}"
                "  }"
                "  var sc = document.querySelectorAll('script:not([src])');"
                "  var pats = ["
                "    /\"price\"\\s*:\\s*\"?([\\d.]+)\"?/,"
                "    /\"lastPrice\"\\s*:\\s*([\\d.]+)/,"
                "    /\"currentPrice\"\\s*:\\s*([\\d.]+)/,"
                "    /\"regularMarketPrice\"\\s*:\\s*([\\d.]+)/,"
                "    /\"nav\"\\s*:\\s*([\\d.]+)/"
                "  ];"
                "  for (var i = 0; i < sc.length; i++) {"
                "    var t = sc[i].textContent;"
                "    for (var j = 0; j < pats.length; j++) {"
                "      var m = t.match(pats[j]);"
                "      if (m && parseFloat(m[1]) > 0) return parseFloat(m[1]) + '|';"
                "    }"
                "  }"
                "  return '';"
                "})()";

            m_page->runJavaScript(kJs,
                [this, symbol, exchange, html](const QVariant &res) {

                    const QString extracted = res.toString().trimmed();
                    Logger::instance().append(
                        QString("GoogleFinanceWeb [%1/%2] JS extraction: '%3'")
                        .arg(symbol, exchange, extracted));

                    if (extracted.isEmpty()) {
                        // Log a brief diagnostic snippet so the HTML structure can
                        // be inspected to improve extraction patterns.
                        const int bodyPos = html.indexOf("<body");
                        const QString snippet =
                            bodyPos >= 0 ? html.mid(bodyPos, 400) : html.left(400);
                        Logger::instance().append(
                            QString("GoogleFinanceWeb [%1/%2] page snippet: %3")
                            .arg(symbol, exchange,
                                 snippet.simplified().left(300)));
                        tryNextExchange(symbol, exchange);
                        return;
                    }

                    const QStringList parts = extracted.split('|');
                    const double price = parts.value(0).toDouble();
                    if (price <= 0.0) {
                        tryNextExchange(symbol, exchange);
                        return;
                    }

                    QDateTime dt;
                    const qint64 unixSec = parts.value(1).toLongLong();
                    if (unixSec > 0)
                        dt = QDateTime::fromSecsSinceEpoch(unixSec, QTimeZone::utc());
                    if (!dt.isValid()) {
                        static const QTimeZone kET("America/New_York");
                        dt = QDateTime(QDate::currentDate(), QTime(16, 0), kET);
                    }

                    m_symbolExchange[symbol] = exchange;
                    Logger::instance().append(
                        QString("GoogleFinanceWeb [%1/%2] price=%3  dt=%4")
                        .arg(symbol, exchange)
                        .arg(price)
                        .arg(dt.toString(Qt::ISODate)));

                    emit dataReady(symbol, {{dt, price}});
                    finishCurrent();
                });
        });
}
