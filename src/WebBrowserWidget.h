#pragma once
#include <QWidget>
#include <QTabBar>
#include <QPointer>
#include <QString>
#include <QList>
#include "AdBlockDialog.h"

class QWebEngineView;
class QWebEngineProfile;
class RequestInterceptor;

class WebBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WebBrowserWidget(QWidget *parent = nullptr);

    void setSymbol(const QString &symbol);
    void openAdBlockDialog();

    // Called from MainWindow::saveSettings / loadSettings.
    void saveBlacklist() const;
    void loadBlacklist();

private slots:
    void onTabChanged(int index);

private:
    void loadCurrentTab();
    QString buildUrl(int tabIndex) const;

    QTabBar            *m_tabBar      = nullptr;
    QWebEngineView     *m_webView     = nullptr;
    QWebEngineProfile  *m_profile     = nullptr;
    RequestInterceptor *m_interceptor = nullptr;
    QString             m_symbol;

    QPointer<AdBlockDialog> m_adBlockDialog;

    struct TabInfo { QString name; QString urlPattern; };
    static const QList<TabInfo> kTabs;
};
