#include "MorningstarWebProvider.h"
#include "Logger.h"
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineProfile>
#include <QRegularExpression>
#include <QUrl>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

// Maximum time to wait for a single page load before declaring a timeout.
static constexpr int kLoadTimeoutMs = 20'000;

// Per-asset-type exchange candidates, tried in order until a price is found.
// The working path is cached in m_symbolExchange per symbol so subsequent
// requests skip straight to the one URL that is known to work.
const QStringList MorningstarWebProvider::kStockExchanges = {
    "stocks/xnys",  // Stock — NYSE
    "stocks/xnas",  // Stock — NASDAQ
    "stocks/xase",  // Stock — AMEX
    "stocks/arcx",  // Stock — NYSE Arca
};
const QStringList MorningstarWebProvider::kEtfExchanges = {
    "etfs/arcx",    // ETF  — NYSE Arca (most ETFs)
    "etfs/xnas",    // ETF  — NASDAQ
    "etfs/xnys",    // ETF  — NYSE
};
const QStringList MorningstarWebProvider::kFundExchanges = {
    "funds/xnas",   // Mutual Fund
};
// Fallback used when the symbol type is unknown — tries every path.
const QStringList MorningstarWebProvider::kExchanges =
    kStockExchanges + kEtfExchanges + kFundExchanges;

MorningstarWebProvider::MorningstarWebProvider(QObject *parent)
    : StockDataProvider(parent)
{
    // Persistent profile: cookies (including Cloudflare's cf_clearance) are
    // written to disk and reloaded on next launch.  Session continuity reduces
    // bot-score friction because the site sees a recurring "browser" rather
    // than a brand-new session on every run.
    m_profile = new QWebEngineProfile("MorningstarWebProvider", this);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    // Inject navigator.webdriver = undefined before any page script executes.
    // Cloudflare and other anti-bot systems check this property; removing it
    // makes the Chromium instance indistinguishable from a normal Chrome session.
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
            this,    &MorningstarWebProvider::onLoadFinished);
    connect(m_timer, &QTimer::timeout,
            this,    &MorningstarWebProvider::onLoadTimeout);
}

// ── Public API ────────────────────────────────────────────────────────────────

void MorningstarWebProvider::fetchData(const QString &symbol, const QString &)
{
    // Page scraping returns only the current price bar — no historical range.
    enqueue(symbol);
}

void MorningstarWebProvider::fetchLatestQuote(const QString &symbol)
{
    enqueue(symbol);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

// Returns the exchange list appropriate for the symbol's known asset type.
// Falls back to the full list when the type is unknown so every path is tried.
const QStringList &MorningstarWebProvider::candidatesFor(const QString &symbol) const
{
    if (m_symbolTypes) {
        switch (m_symbolTypes->value(symbol, SymbolType::Unknown)) {
        case SymbolType::Stock: return kStockExchanges;
        case SymbolType::ETF:   return kEtfExchanges;
        case SymbolType::MutualFund: return kFundExchanges;
        default: break;
        }
    }
    return kExchanges;
}

// ── Queue management ──────────────────────────────────────────────────────────

void MorningstarWebProvider::enqueue(const QString &symbol)
{
    // Deduplicate: don't queue the same symbol twice.
    if (m_busy && m_current.symbol == symbol) return;
    for (const Request &r : m_queue)
        if (r.symbol == symbol) return;

    const QStringList &candidates = candidatesFor(symbol);
    // Use the cached exchange only when it belongs to the type's candidate list;
    // a stale cache entry for the wrong asset class would waste a full page load.
    QString exchange = candidates.first();
    if (m_symbolExchange.contains(symbol) && candidates.contains(m_symbolExchange[symbol]))
        exchange = m_symbolExchange[symbol];

    m_queue.enqueue({ symbol, exchange });
    Logger::instance().append(
        QString("MorningstarWeb [%1] queued exchange=%2 (queue size=%3)")
        .arg(symbol, exchange).arg(m_queue.size()));
    loadNext();
}

void MorningstarWebProvider::loadNext()
{
    if (m_busy || m_queue.isEmpty()) return;
    m_busy    = true;
    m_current = m_queue.dequeue();
    loadPage(m_current.symbol, m_current.exchange);
}

void MorningstarWebProvider::finishCurrent()
{
    m_busy = false;
    loadNext();
}

// ── Page loading ──────────────────────────────────────────────────────────────

void MorningstarWebProvider::loadPage(const QString &symbol, const QString &exchange)
{
    m_current = { symbol, exchange };
    const QUrl url(QString("https://www.morningstar.com/%1/%2/quote")
                   .arg(exchange, symbol.toLower()));
    Logger::instance().append(
        QString("MorningstarWeb [%1/%2] loading %3")
        .arg(symbol, exchange, url.toString()));
    m_timedOut = false;
    m_timer->start();
    m_page->load(url);
}

void MorningstarWebProvider::tryNextExchange(const QString &symbol,
                                              const QString &triedExchange)
{
    const QStringList &candidates = candidatesFor(symbol);
    const int idx = candidates.indexOf(triedExchange);
    if (idx < 0 || idx + 1 >= candidates.size()) {
        emit errorOccurred(symbol,
            QString("MorningstarWeb: price not found for %1 on any exchange").arg(symbol));
        finishCurrent();
        return;
    }
    // Still within the current busy slot — just load the next exchange URL.
    loadPage(symbol, candidates[idx + 1]);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MorningstarWebProvider::onLoadTimeout()
{
    m_timedOut = true;
    m_page->triggerAction(QWebEnginePage::Stop);  // triggers loadFinished(false) which handles cleanup
    Logger::instance().append(
        QString("MorningstarWeb [%1/%2] load timed out (%3 s) — stopping")
        .arg(m_current.symbol, m_current.exchange)
        .arg(kLoadTimeoutMs / 1000));
}

void MorningstarWebProvider::onLoadFinished(bool ok)
{
    m_timer->stop();

    const QString symbol   = m_current.symbol;
    const QString exchange = m_current.exchange;

    // Timeout already logged and m_page->stop() was called; just emit the error.
    if (m_timedOut) {
        m_timedOut = false;
        emit errorOccurred(symbol,
            QString("MorningstarWeb: page load timed out for %1/%2").arg(symbol, exchange));
        finishCurrent();
        return;
    }

    if (!ok) {
        // Network / SSL error or 404 on this exchange — try the next one.
        Logger::instance().append(
            QString("MorningstarWeb [%1/%2] loadFinished(false) — trying next exchange")
            .arg(symbol, exchange));
        tryNextExchange(symbol, exchange);
        return;
    }

    // Pull the full rendered HTML from the live DOM.  Morningstar is a Next.js
    // app; the intraday bar data is server-side rendered into <script> tags and
    // is available as soon as loadFinished fires.
    m_page->runJavaScript("document.documentElement.outerHTML",
        [this, symbol, exchange](const QVariant &result) {
            const QString  html      = result.toString();
            const QByteArray htmlBytes = html.toUtf8();
            m_lastHistoryJson = htmlBytes;
            m_lastQuoteJson   = htmlBytes;
            emit historyResponseStored();
            emit quoteResponseStored();

            // Detect an unresolved bot challenge.  Under normal operation
            // Chromium executes the Cloudflare JS challenge automatically.
            // If we still see challenge markers here it means an interactive
            // CAPTCHA (Turnstile) is required — that needs human input.
            if (html.contains("cf-chl-bypass") ||
                html.contains("challenge-platform") ||
                html.contains("jschl_vc") ||
                (html.contains("Cloudflare") && !html.contains("lastPrice"))) {
                const QString msg =
                    QString("MorningstarWeb [%1] bot-check not bypassed "
                            "(interactive CAPTCHA?) — manual intervention required")
                    .arg(symbol);
                Logger::instance().append(msg);
                emit errorOccurred(symbol, "MorningstarWeb: " + msg);
                finishCurrent();
                return;
            }

            double    price = 0.0;
            QDateTime dt;
            if (!parsePrice(html, price, dt)) {
                Logger::instance().append(
                    QString("MorningstarWeb [%1/%2] price not found — trying next exchange")
                    .arg(symbol, exchange));
                tryNextExchange(symbol, exchange);
                return;
            }

            m_symbolExchange[symbol] = exchange;
            Logger::instance().append(
                QString("MorningstarWeb [%1/%2] price=%3  dt=%4")
                .arg(symbol, exchange)
                .arg(price)
                .arg(dt.toString(Qt::ISODate)));

            emit dataReady(symbol, {{dt, price}});
            finishCurrent();
        });
}

// ── Parsing ───────────────────────────────────────────────────────────────────

bool MorningstarWebProvider::parsePrice(const QString &html,
                                         double &price, QDateTime &dt) const
{
    price = 0.0;
    dt    = QDateTime();

    // Morningstar embeds intraday bars as unquoted JS object literals, e.g.:
    //   {datetime:"2026-04-13T14:45:00-04:00",volume:45580,lastPrice:237.8201,...}
    // Match objects containing both datetime and lastPrice; keep the last one
    // (most recent bar).
    QRegularExpression barRx(
        R"rx(\{datetime:"([^"]+)"[^}]*lastPrice:([\d.]+))rx");

    double  latestPrice = 0.0;
    QString latestDtStr;

    QRegularExpressionMatchIterator it = barRx.globalMatch(html);
    while (it.hasNext()) {
        const auto m = it.next();
        const double p = m.captured(2).toDouble();
        if (p > 0.0) {
            latestPrice = p;
            latestDtStr = m.captured(1);
        }
    }

    // Fallback: bare lastPrice value anywhere on the page (no datetime pairing).
    if (latestPrice <= 0.0) {
        QRegularExpression fallbackRx(R"rx(lastPrice["\s:]+(\d+\.?\d*))rx");
        const auto fm = fallbackRx.match(html);
        if (fm.hasMatch())
            latestPrice = fm.captured(1).toDouble();
    }

    if (latestPrice <= 0.0)
        return false;

    price = latestPrice;

    // Parse ISO 8601 with UTC offset, e.g. "2026-04-13T14:45:00-04:00".
    if (!latestDtStr.isEmpty())
        dt = QDateTime::fromString(latestDtStr, Qt::ISODate);

    if (!dt.isValid()) {
        static const QTimeZone kET("America/New_York");
        dt = QDateTime(QDate::currentDate(), QTime(16, 0), kET);
    }

    return true;
}
