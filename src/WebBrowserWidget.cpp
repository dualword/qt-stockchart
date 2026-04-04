#include "WebBrowserWidget.h"
#include "RequestInterceptor.h"
#include "Logger.h"
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineLoadingInfo>
#include <QSettings>
#include <QUrl>

// URL patterns:  {upper} = symbol uppercase,  {lower} = symbol lowercase.
// Morningstar requires an exchange prefix in the URL path that we don't have
// per-symbol, so fall back to their search page.
// TradingView resolves the exchange automatically from the ticker alone.
const QList<WebBrowserWidget::TabInfo> WebBrowserWidget::kTabs = {
    { "Morningstar",  "https://www.morningstar.com/search?query={upper}"  },
    { "TradingView",  "https://www.tradingview.com/symbols/{upper}/"      },
    { "StockCharts",  "https://stockcharts.com/sc3/ui/?s={upper}"         },
    { "Zacks",        "https://www.zacks.com/stock/quote/{upper}?q={lower}" },
};

WebBrowserWidget::WebBrowserWidget(QWidget *parent)
    : QWidget(parent)
{
    // ── Profile + interceptor ─────────────────────────────────────────────────
    // Use a persistent named profile so cookies/sessions survive restarts.
    m_profile     = new QWebEngineProfile("StockChartBrowser", this);
    m_interceptor = new RequestInterceptor(this);
    m_profile->setUrlRequestInterceptor(m_interceptor);

    // ── Web view ──────────────────────────────────────────────────────────────
    auto *page = new QWebEnginePage(m_profile, this);
    m_webView = new QWebEngineView(this);
    m_webView->setPage(page);

    // ── Tab bar ───────────────────────────────────────────────────────────────
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabBar = new QTabBar(this);
    m_tabBar->setExpanding(false);
    for (const auto &tab : kTabs)
        m_tabBar->addTab(tab.name);

    layout->addWidget(m_tabBar);
    layout->addWidget(m_webView, 1);

    connect(m_tabBar, &QTabBar::currentChanged, this, &WebBrowserWidget::onTabChanged);

    // ── Loading log ───────────────────────────────────────────────────────────
    connect(page, &QWebEnginePage::loadingChanged,
            this, [this](const QWebEngineLoadingInfo &info) {
        switch (info.status()) {
        case QWebEngineLoadingInfo::LoadStartedStatus:
            Logger::instance().append("Browser: loading " + info.url().toString());
            break;
        case QWebEngineLoadingInfo::LoadSucceededStatus:
            Logger::instance().append("Browser: OK \xe2\x80\x94 " + info.url().toString());
            break;
        case QWebEngineLoadingInfo::LoadFailedStatus:
            Logger::instance().append(
                QString("Browser: failed [%1] \xe2\x80\x94 %2")
                    .arg(info.errorString(), info.url().toString()));
            break;
        default:
            break;
        }
    });
}

// ── Public API ────────────────────────────────────────────────────────────────

void WebBrowserWidget::setSymbol(const QString &symbol)
{
    if (m_symbol == symbol) return;
    m_symbol = symbol;
    loadCurrentTab();
}

void WebBrowserWidget::openAdBlockDialog()
{
    if (!m_adBlockDialog) {
        // Parent to the top-level window so the dialog floats above the whole app.
        m_adBlockDialog = new AdBlockDialog(m_interceptor, window());
        m_adBlockDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_adBlockDialog, &QDialog::finished, this, [this]() {
            saveBlacklist();
        });
    }
    m_adBlockDialog->show();
    m_adBlockDialog->raise();
    m_adBlockDialog->activateWindow();
}

void WebBrowserWidget::saveBlacklist() const
{
    QSettings s("StockChart", "StockChart");
    const QSet<QString> bl = m_interceptor->blacklist();
    s.setValue("adBlockBlacklist", QStringList(bl.cbegin(), bl.cend()));
}

void WebBrowserWidget::loadBlacklist()
{
    QSettings s("StockChart", "StockChart");
    const QStringList list = s.value("adBlockBlacklist").toStringList();
    m_interceptor->setBlacklist(QSet<QString>(list.cbegin(), list.cend()));
}

// ── Private ───────────────────────────────────────────────────────────────────

void WebBrowserWidget::onTabChanged(int)
{
    loadCurrentTab();
}

void WebBrowserWidget::loadCurrentTab()
{
    if (m_symbol.isEmpty()) return;
    m_interceptor->clearTracking(); // fresh domain list for this navigation
    const QString url = buildUrl(m_tabBar->currentIndex());
    if (!url.isEmpty())
        m_webView->load(QUrl(url));
}

QString WebBrowserWidget::buildUrl(int tabIndex) const
{
    if (tabIndex < 0 || tabIndex >= kTabs.size()) return {};
    QString url = kTabs[tabIndex].urlPattern;
    url.replace("{upper}", m_symbol.toUpper());
    url.replace("{lower}", m_symbol.toLower());
    return url;
}
