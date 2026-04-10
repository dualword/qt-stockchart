#pragma once
#include <QDialog>
#include <QRegularExpression>

class RequestInterceptor;
class QListWidget;
class QTreeWidget;
class QPushButton;
class QLineEdit;

class AdBlockDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdBlockDialog(RequestInterceptor *interceptor, QWidget *parent = nullptr);

    // Called by WebBrowserWidget after a page load finishes.
    void refreshActiveList();

signals:
    void reloadRequested();

private slots:
    void onAdd();
    void onRemove();
    void onClearActive();
    void onReload();
    void onRegexChanged(const QString &text);

private:
    void refreshBlacklist();

    RequestInterceptor *m_interceptor  = nullptr;
    QListWidget        *m_activeList   = nullptr;
    QTreeWidget        *m_blackList    = nullptr;
    QPushButton        *m_addBtn       = nullptr;
    QPushButton        *m_removeBtn    = nullptr;
    QLineEdit          *m_adRegexEdit  = nullptr;

    QRegularExpression  m_adRx;
};
