#pragma once
#include <QDialog>

class RequestInterceptor;
class QListWidget;
class QTreeWidget;
class QPushButton;
class QTimer;

class AdBlockDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdBlockDialog(RequestInterceptor *interceptor, QWidget *parent = nullptr);

private slots:
    void onAdd();
    void onRemove();
    void refreshLists();

private:
    void refreshActiveList();
    void refreshBlacklist();

    RequestInterceptor *m_interceptor  = nullptr;
    QListWidget        *m_activeList   = nullptr;
    QTreeWidget        *m_blackList    = nullptr;
    QPushButton        *m_addBtn       = nullptr;
    QPushButton        *m_removeBtn    = nullptr;
    QTimer             *m_refreshTimer = nullptr;
};
